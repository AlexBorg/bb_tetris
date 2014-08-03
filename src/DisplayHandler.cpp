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

// Textures to load
static std::vector<GLuint> digit_textures;
static GLuint tex_bg;
static GLuint tex_block_bg, tex_block_outer, tex_block_inner;
static GLuint tex_block_top, tex_block_bottom, tex_block_left, tex_block_right;
static GLuint tex_paused;
static GLuint tex_game_over;

// Colors to use for blocks
static std::vector<unsigned int> colors = {0x000000, 0xFF6666, 0x66FF66,
  0x6666FF, 0xFFFF66, 0x66FFFF, 0xFF66FF, 0x0099FF };

// 3x3 table of which textures to use to draw this block -- this consists of
// four corners, four edges, and the center, which is always the same.
struct BlockTextureMap {
  GLuint tex[3][3];
  unsigned int color;

  BlockTextureMap() {
    color = 0;
    memset(tex, 0, sizeof(tex));
  }

  // Generate texture map from board state
  BlockTextureMap(unsigned int block_x, unsigned int block_y, const BoardState & board) {
    bool context[3][3];

    for(auto x = 0; x < 3; x++) {
      for(auto y = 0; y < 3; y++) {
        auto pos_x = block_x + x - 1;
        auto pos_y = block_y + y - 1;
        context[x][y] = pos_x >= 0 && pos_x < board.size() &&
                        pos_y >= 0 && pos_y < board[pos_x].size() &&
                        board[block_x][block_y] == board[pos_x][pos_y];
      }
    }

    initialize(context, board[block_x][block_y].color);
  }

  // Generate texture map based on individual tetromino
  BlockTextureMap(unsigned int block_x, unsigned int block_y, const Tetromino & piece) {
    bool context[3][3];

    for(auto x = 0; x < 3; x++) {
      for(auto y = 0; y < 3; y++) {
        int pos_x = block_x + x - 1;
        int pos_y = block_y + y - 1;
        context[x][y] = pos_x >= 0 && pos_x < piece.width &&
                        pos_y >= 0 && pos_y < piece.height &&
                        piece.getBlock(pos_x, pos_y).color != 0;
      }
    }

    initialize(context, piece.getBlock(block_x, block_y).color);
  }

  // From context of surrounding blocks, determine which textures to use to
  // draw this block -- this consists of four corners, four edges, and the
  // center, which is always the same.
  void initialize(bool ctx[3][3], unsigned int block_color) {
    color = colors.at(block_color);

    tex[0][0] = ctx[1][0] && ctx[0][1] && ctx[0][0] ? tex_block_bg :
                ctx[1][0] && ctx[0][1]              ? tex_block_inner :
                ctx[1][0]                           ? tex_block_left :
                ctx[0][1]                           ? tex_block_bottom :
                                                      tex_block_outer;
    tex[0][1] = ctx[0][1] ? tex_block_bg : tex_block_left;
    tex[0][2] = ctx[1][2] && ctx[0][1] && ctx[0][2] ? tex_block_bg :
                ctx[1][2] && ctx[0][1]              ? tex_block_inner :
                ctx[1][2]                           ? tex_block_left :
                ctx[0][1]                           ? tex_block_top :
                                                      tex_block_outer;
    tex[1][0] = ctx[1][0] ? tex_block_bg : tex_block_bottom;
    tex[1][1] = tex_block_bg;
    tex[1][2] = ctx[1][2] ? tex_block_bg : tex_block_top;
    tex[2][0] = ctx[1][0] && ctx[2][1] && ctx[2][0] ? tex_block_bg :
                ctx[1][0] && ctx[2][1]              ? tex_block_inner :
                ctx[1][0]                           ? tex_block_right :
                ctx[2][1]                           ? tex_block_bottom :
                                                      tex_block_outer;
    tex[2][1] = ctx[2][1] ? tex_block_bg : tex_block_right;
    tex[2][2] = ctx[1][2] && ctx[2][1] && ctx[2][2] ? tex_block_bg :
                ctx[1][2] && ctx[2][1]              ? tex_block_inner :
                ctx[1][2]                           ? tex_block_right :
                ctx[2][1]                           ? tex_block_top :
                                                      tex_block_outer;
  }

  bool operator==(const BlockTextureMap & other) {
    return memcmp(tex, other.tex, sizeof(tex)) == 0 && color == other.color;
  }

  bool operator!=(const BlockTextureMap & other) {
    return !operator==(other);
  }
};

typedef std::array<std::array<BlockTextureMap, BOARD_HEIGHT>, BOARD_WIDTH> BoardTextureMap;

////////////////////////////////////////////////////////////////////////////////
void DrawBox(GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(x, y);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(x, y + h);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(x + w, y + h);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(x + w, y);
  glEnd();
}

////////////////////////////////////////////////////////////////////////////////
void DrawBlockPart(float x, float y, float w, float h, GLuint tex) {
  glBindTexture(GL_TEXTURE_2D, tex);

  glPushMatrix();
  glTranslatef(x, y, 0.0f);

  glBegin(GL_QUADS);
  glTexCoord2f(x, 1.0f - y);
  glVertex2f(0.0f, 0.0f);
  glTexCoord2f(x, 1.0f - y - h);
  glVertex2f(0.0f, h);
  glTexCoord2f(x + w, 1.0f - y - h);
  glVertex2f(w, h);
  glTexCoord2f(x + w, 1.0f - y);
  glVertex2f(w, 0.0f);
  glEnd();

  glPopMatrix();
}

////////////////////////////////////////////////////////////////////////////////
// Draw a block at x,y using a texture map
void DrawBlock(float x, float y, const BlockTextureMap & map) {
  glPushMatrix();
  glTranslatef(x, y, 0.0f);

  glColor3ub((map.color >> 16) & 0xFF, (map.color >> 8) & 0xFF, map.color & 0xFF);

  if(map.color == 0) {
    // Just draw an untexture box if color is black
    DrawBox(0.0f, 0.0f, 1.0f, 1.0f);
  } else {
    glEnable(GL_TEXTURE_2D);
    DrawBlockPart(0.00f, 0.75f, 0.25f, 0.25f, map.tex[0][2]);
    DrawBlockPart(0.25f, 0.75f, 0.50f, 0.25f, map.tex[1][2]);
    DrawBlockPart(0.75f, 0.75f, 0.25f, 0.25f, map.tex[2][2]);
    DrawBlockPart(0.00f, 0.25f, 0.25f, 0.50f, map.tex[0][1]);
    DrawBlockPart(0.25f, 0.25f, 0.50f, 0.50f, map.tex[1][1]);
    DrawBlockPart(0.75f, 0.25f, 0.25f, 0.50f, map.tex[2][1]);
    DrawBlockPart(0.00f, 0.00f, 0.25f, 0.25f, map.tex[0][0]);
    DrawBlockPart(0.25f, 0.00f, 0.50f, 0.25f, map.tex[1][0]);
    DrawBlockPart(0.75f, 0.00f, 0.25f, 0.25f, map.tex[2][0]);
    glDisable(GL_TEXTURE_2D);
  }

  glPopMatrix();
}

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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return tex_id;
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
  tex_bg           = LoadTexture(string("background.png"));
  tex_block_bg     = LoadTexture(string("block_bg.png"));
  tex_block_outer  = LoadTexture(string("block_outer.png"));
  tex_block_inner  = LoadTexture(string("block_inner.png"));
  tex_block_top    = LoadTexture(string("block_top.png"));
  tex_block_bottom = LoadTexture(string("block_bottom.png"));
  tex_block_left   = LoadTexture(string("block_left.png"));
  tex_block_right  = LoadTexture(string("block_right.png"));
  tex_paused       = LoadTexture(string("paused.png"));
  tex_game_over    = LoadTexture(string("game_over.png"));

  for(int i = 0; i <= 9; i++) {
    digit_textures.push_back(LoadTexture(string("digit") + std::to_string(i) + ".png"));
  }

  // Initialize display
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw static top bar and initial score/level values
  DrawBox(0.0f, 0.0f, 14.0f, AREA_HEIGHT, tex_bg);
  DrawDigits(1.0f, AREA_HEIGHT - 3, 6, 0);
  DrawDigits(8.0f, AREA_HEIGHT - 3, 1, 0);

  GameState game, last_game;
  BoardTextureMap curr_board, last_board;

  // Redraw display as fast as we can (not at all fast)
  while(true) {
    last_game = game;
    controller.getGameState(game);

    // Draw score
    if(game.score != last_game.score) {
      DrawDigits(1.0f, AREA_HEIGHT - 3, 6, game.score);
    }

    // Draw level
    if(game.level != last_game.level) {
      DrawDigits(8.0f, AREA_HEIGHT - 3, 1, game.level);
    }

    // Draw next tetromino
    if(game.next != last_game.next) {
      glPushMatrix();
      glTranslatef(11.5f, AREA_HEIGHT - 2.0f, 0.0f);
      
      // Just blank out the whole area
      glColor3ub(0, 0, 0);
      DrawBox(-1.5f, -1.5f, 3.0f, 3.0f);

      // Scale down a bit so it fits
      glScalef(0.75f, 0.75f, 0.0f);
      
      for(auto x = 0; x < game.next.width; x++) {
          for(auto y = 0; y < game.next.height; y++) {
              auto center = game.next.getCenter();

              BlockTextureMap block_tex = BlockTextureMap(x, y, game.next);
              if(block_tex.color != 0) {
                DrawBlock(x - center.first, y - center.second, block_tex);
              }
          }
      }
      glPopMatrix();
    }

    // Draw game area
    glPushMatrix();
    glTranslatef((AREA_WIDTH - BOARD_WIDTH) / 2.0f, 0.0f, 0.0f);

    if(!game.game_over && !game.paused) {
      bool refresh = last_game.game_over || last_game.paused;

      // Draw game board
      game.active.place(game.board); // Draw with active tetromino

      for(unsigned int x = 0; x < game.board.size(); x++)  {
        for(unsigned int y = 0; y < game.board[x].size(); y++) {
          curr_board[x][y] = BlockTextureMap(x, y, game.board);

          // Redraw block only if it has changed since the last frame
          if(curr_board[x][y] != last_board[x][y] || refresh) {
            DrawBlock(x, y, curr_board[x][y]);
          }
        }
      }

      last_board = curr_board;

    } else if(game.game_over && !last_game.game_over) {
      // Draw "GAME OVER" message
      DrawBox(0.0f, 0.0f, 10.0f, 20.0f, tex_game_over);

    } else if(game.paused && !last_game.paused) {
      // Draw "PAUSED" message
      DrawBox(0.0f, 0.0f, 10.0f, 20.0f, tex_paused);

    }

    glPopMatrix();

    SDL_GL_SwapBuffers();
  }
}
