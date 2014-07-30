#include <signal.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <tuple>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "BBTdefines.hpp"
#include "GameController.hpp"
#include "GameState.hpp"

using std::string;

const GLdouble SCREEN_WIDTH  = 272.0;
const GLdouble SCREEN_HEIGHT = 480.0;
const GLdouble BLOCK_SIZE    = 20.0;
const GLdouble AREA_WIDTH    = SCREEN_WIDTH / BLOCK_SIZE;
const GLdouble AREA_HEIGHT   = SCREEN_HEIGHT / BLOCK_SIZE;

static std::vector<GLuint> block_textures;
static std::vector<GLuint> digit_textures;
static GLuint top_bar_texture;

////////////////////////////////////////////////////////////////////////////////
// Load a texture from an image file and return GL texture ID
GLuint LoadTexture(string file) {
  SDL_Surface *tex = IMG_Load(file.c_str());

  if(!tex) {
    printf("Error loading texture %s: %s", file.c_str(), IMG_GetError());
    return 0;
  }

  // Convert to RGBA format for GL
  SDL_PixelFormat fmt;
  fmt.palette       = 0;
  fmt.colorkey      = 0;
  fmt.alpha         = 0;
  fmt.BitsPerPixel  = 32;
  fmt.BytesPerPixel = 4;
  fmt.Rmask = 0x000000FF; fmt.Rshift = 24; fmt.Rloss = 0;
  fmt.Gmask = 0x0000FF00; fmt.Gshift = 16; fmt.Gloss = 0;
  fmt.Bmask = 0x00FF0000; fmt.Bshift = 8;  fmt.Bloss = 0;
  fmt.Amask = 0xFF000000; fmt.Ashift = 0;  fmt.Aloss = 0;

  SDL_Surface *tex_rgba = SDL_ConvertSurface(tex, &fmt, SDL_SWSURFACE);
  SDL_FreeSurface(tex);

  // Load GL texture
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_rgba->w, tex_rgba->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_rgba->pixels);
  SDL_FreeSurface(tex_rgba);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return tex_id;
}

////////////////////////////////////////////////////////////////////////////////
void DrawBox(GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
  glBegin(GL_QUADS);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(x + w, y);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(x + w, y + h);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(x, y + h);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(x, y);
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////
void DrawBox(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLuint tex) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex);
  glColor3ub(255, 255, 255);
  DrawBox(x, y, w, h);
  glDisable(GL_TEXTURE_2D);
}

////////////////////////////////////////////////////////////////////////////////
void DrawBlock(const BlockData & block, int x, int y) {
  static BoardState last_state;

  // Dont draw if outside the game board
  if(x < 0 || x >= last_state.size()) return;
  if(y < 0 || y >= last_state[x].size()) return;

  // Don't draw if the same as last time
  if(last_state[x][y] == block) return;
  last_state[x][y] = block;

  // Draw a black square if no color, otherwise textured square
  glPushMatrix();
  glTranslatef(x, y, 0.0);

  if(block.color < 1 || block.color > (signed)block_textures.size()) {
    glColor3ub(0, 0, 0);
    DrawBox(0.0f, 0.0f, 1.0f, 1.0f);
  } else {
    DrawBox(0.0f, 0.0f, 1.0f, 1.0f, block_textures[block.color - 1]);
  }
  glPopMatrix();
}

////////////////////////////////////////////////////////////////////////////////
void DrawDigits(GLfloat x, GLfloat y, int n_digits, unsigned int score) {
  char digits[n_digits + 1];
  snprintf(digits, sizeof(digits), "%.*u", n_digits, score);
  
  for(int i = 0; i < n_digits; i++) {
    DrawBox(x + i, y, 1, 2, digit_textures[digits[i] - '0']);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main loop for display thread
void DisplayHandler(GameController & controller) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

  // Attempt to create portrait orientation surface
  SDL_Surface *surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_OPENGL | SDL_FULLSCREEN);

  // If we fail to create the first surface, assume that we're running
  // in landscape on the LCD CAPE and rotate here
  bool rotate_display = false;
  if(surface == NULL) {
    rotate_display = true;
    surface = SDL_SetVideoMode(SCREEN_HEIGHT, SCREEN_WIDTH, 0, SDL_OPENGL | SDL_FULLSCREEN);
  }

  // Still failed. WTF?
  if(surface == NULL) {
    printf("Failed to create video surface: %s\n", SDL_GetError());
    exit(1);
  }

  // Don't let SDL stomp on these
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  // Set up projection -- reversed if display rotated
  glViewport(0, 0, surface->w, surface->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if(rotate_display) {
    gluOrtho2D(0.0, AREA_HEIGHT, 0.0, AREA_WIDTH);
  } else {
    gluOrtho2D(0.0, AREA_WIDTH, 0.0, AREA_HEIGHT);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // If rotated, rotate around center and return to bottom-left corner
  if(rotate_display) {
    glTranslatef(AREA_HEIGHT / 2.0, AREA_WIDTH / 2.0, 0.0);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-AREA_WIDTH / 2.0, -AREA_HEIGHT / 2.0, 0.0);
  }

  // Load textures
  top_bar_texture = LoadTexture(string("top_bar.png"));
  for(int i = 1; i <= BlockData::num_colors; i++) {
    block_textures.push_back(LoadTexture(string("block") + std::to_string(i) + ".png"));
  }

  for(int i = 0; i <= 9; i++) {
    digit_textures.push_back(LoadTexture(string("digit") + std::to_string(i) + ".png"));
  }

  // Initialize display
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw static top bar and initial score/level values
  DrawBox(0.0f, AREA_HEIGHT - 4, 14.0f, 4.0f, top_bar_texture);
  DrawDigits(1.0f, AREA_HEIGHT - 3, 6, 0);
  DrawDigits(8.0f, AREA_HEIGHT - 3, 1, 0);

  GameState game;
  unsigned int last_score = 0;
  unsigned int last_level = 0;

  // Redraw display as fast as we can (not at all fast)
  while(true) {
    controller.getGameState(game);

    // Draw score
    if(last_score != game.score) {
      DrawDigits(1.0f, AREA_HEIGHT - 3, 6, game.score);
      last_score = game.score;
    }

    // Draw level
    if(last_level != game.level) {
      DrawDigits(8.0f, AREA_HEIGHT - 3, 1, game.level);
      last_level = game.level;
    }

    glPushMatrix();
    glTranslatef((AREA_WIDTH - BOARD_WIDTH) / 2.0f, 0.0f, 0.0f);

    // Draw game board
    for(unsigned int x = 0; x < game.board.size(); x++)  {
      for(unsigned int y = 0; y < game.board[x].size(); y++) {
        DrawBlock(game.board[x][y], x, y);
      }
    }

    // Draw active tetromino over game board
    for(int x = 0; x < Tetromino::width; x++)  {
      for(int y = 0; y < Tetromino::height; y++) {
        BlockData b = game.active.getBlock(x, y);
        if(b.color != 0) DrawBlock(b, game.active.pos_x + x, game.active.pos_y + y);
      }
    }

    glPopMatrix();

    // Draw outlines
    /*
    static std::vector<std::tuple<GLfloat, GLfloat, GLfloat, GLfloat>> segments;

    segments.clear();
    for(unsigned int x = 0; x < game.board.size(); x++)  {
      for(unsigned int y = 0; y < game.board[x].size(); y++) {
        BlockData b = game.board[x][y];

        if(b.color == 0) continue;

        GLfloat offset = 1.0f / BLOCK_SIZE / 2.0f;
        GLfloat top = y + 1.0 - offset;
        GLfloat bottom = y + offset;
        GLfloat left = x + offset;
        GLfloat right = x + 1.0f - offset;

        if(x == 0 || game.board[x - 1][y].id != b.id) {
          segments.push_back(std::make_tuple(left, bottom - offset, left, top + offset));
        }

        if(x == game.board[x].size() - 1 || game.board[x + 1][y].id != b.id) {
          segments.push_back(std::make_tuple(right, bottom - offset, right, top + offset));
        }

        if(y == 0 || game.board[x][y - 1].id != b.id) {
          segments.push_back(std::make_tuple(left - offset, bottom, right + offset, bottom));
        }

        if(y == game.board.size() - 1 || game.board[x][y + 1].id != b.id) {
          segments.push_back(std::make_tuple(left - offset, top, right + offset, top));
        }
      }
    }

    glDisable(GL_TEXTURE_2D);
    glColor3ub(255, 255, 255);
    glColor3ub(0, 0, 0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for(auto it = segments.begin(); it != segments.end(); ++it) {
      glVertex2f(std::get<0>(*it), std::get<1>(*it));
      glVertex2f(std::get<2>(*it), std::get<3>(*it));
    }
    glEnd();
    */

    SDL_GL_SwapBuffers();

    // Nothing handled yet
    //SDL_Event event;
    //while(SDL_PollEvent(&event));
  }
}
