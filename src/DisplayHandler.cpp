#include <signal.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <rtdk.h>
#include <native/timer.h>

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
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);

    float rotation = 0.0;

    while(true) {
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();

       rotation += 1.0;
       glTranslatef(0.5, 0.5, 0.0);
       glRotatef(rotation, 0.0, 0.0, 1.0);
       glTranslatef(-0.5, -0.5, 0.0);

       glClear(GL_COLOR_BUFFER_BIT);
       glBegin(GL_QUADS);
       glColor3f(1.0, 1.0, 1.0);
       glVertex2f(0.0, 0.0);
       glColor3f(1.0, 0.0, 0.0);
       glVertex2f(1.0, 0.0);
       glColor3f(0.0, 1.0, 0.0);
       glVertex2f(1.0, 1.0);
       glColor3f(0.0, 0.0, 1.0);
       glVertex2f(0.0, 1.0);
       glEnd();

       SDL_GL_SwapBuffers();

       // Nothing handled yet
       //SDL_Event event;
       //while(SDL_PollEvent(&event));
    }
}
