#include <signal.h>
#include <cstdlib>
#include <vector>
#include <string>
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

static std::vector<GLuint> block_textures;

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
void DrawBlock(const BlockData & block, int x, int y) {
  static BoardState last_state;

  // Dont draw if outside the game board
  if(x < 0 || x >= last_state.size()) return;
  if(y < 0 || y >= last_state[x].size()) return;

  // Don't draw if the same as last time
  if(last_state[x][y] == block) return;
  last_state[x][y] = block;

  // Draw a black square if no color, otherwise textured square
  if(block.color < 1 || block.color > (signed)block_textures.size()) {
    glColor3ub(0, 0, 0);
    glDisable(GL_TEXTURE_2D);
  } else {
    glColor3ub(255, 255, 255);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, block_textures[block.color - 1]);
  }

  // Draw block
  glPushMatrix();
  glTranslatef(x, y, 0.0);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 1.0f);
  glVertex2f(0.0, 0.0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex2f(0.0, 1.0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex2f(1.0, 1.0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex2f(1.0, 0.0);
  glEnd();

  glPopMatrix();
}

////////////////////////////////////////////////////////////////////////////////
// Main loop for display thread
void DisplayHandler(GameController & controller) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
  SDL_Surface *surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_OPENGL | SDL_FULLSCREEN);

  // Don't let SDL stomp on these
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  // Set up projection
  glViewport(0, 0, surface->w, surface->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, SCREEN_WIDTH / BLOCK_SIZE, 0.0, SCREEN_HEIGHT / BLOCK_SIZE);

  // Load textures
  for(int i = 1; i <= BlockData::num_colors; i++) {
    block_textures.push_back(LoadTexture(string("block") + std::to_string(i) + ".png"));
  }

  // Initialize display
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  GameState game;

  while(true) {
    controller.getGameState(game);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw game board
    for(unsigned int x = 0; x < game.board.size(); x++)  {
      for(unsigned int y = 0; y < game.board[x].size(); y++) {
        DrawBlock(game.board[x][y], x, y);
      }
    }

    // Draw active tetromino
    for(int x = 0; x < Tetromino::width; x++)  {
      for(int y = 0; y < Tetromino::height; y++) {
        BlockData b = game.active.getBlock(x, y);
        if(b.color != 0) DrawBlock(b, game.active.pos_x + x, game.active.pos_y + y);
      }
    }

    SDL_GL_SwapBuffers();

    // Nothing handled yet
    //SDL_Event event;
    //while(SDL_PollEvent(&event));
  }
}
