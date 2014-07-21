#include <signal.h>
#include <cstdlib>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <rtdk.h>
#include <native/timer.h>
#include "BBTdefines.hpp"

static GameState game;

void DisplayHandler() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_ShowCursor(0);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_Surface *surface = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL | SDL_FULLSCREEN);

  // Don't let SDL stomp on these
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  glViewport(0, 0, surface->w, surface->h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble)BOARD_WIDTH, 0.0, (GLdouble)BOARD_HEIGHT);

  float rotation = 0.0;

  GLubyte colors[][3] = {
    {0, 0, 0},
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 0, 255},
    {0, 255, 255},
    {255, 255, 0},
    {255, 255, 255}
  };

  for(int x = 0; x < BOARD_WIDTH; x++) {
    for(int y = 0; y < BOARD_HEIGHT; y++) {
      game.board[x][y].color = random() % 8;
    }
  }

  while(true) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    rotation += 1.0;
    glTranslatef(BOARD_WIDTH / 2, BOARD_HEIGHT / 2, 0.0);
    glRotatef(rotation, 0.0, 0.0, 1.0);
    glTranslatef(-BOARD_WIDTH / 2, -BOARD_HEIGHT / 2, 0.0);

    glClear(GL_COLOR_BUFFER_BIT);
    for(int x = 0; x < BOARD_WIDTH; x++)  {
      for(int y = 0; y < BOARD_HEIGHT; y++) {
        glBegin(GL_QUADS);
        glColor3ubv(colors[game.board[x][y].color]);
        glVertex2f(x + 0.0, y + 0.0);
        glVertex2f(x + 1.0, y + 0.0);
        glVertex2f(x + 1.0, y + 1.0);
        glVertex2f(x + 0.0, y + 1.0);
        glEnd();
      }
    }

    SDL_GL_SwapBuffers();

    // Nothing handled yet
    //SDL_Event event;
    //while(SDL_PollEvent(&event));
  }
}
