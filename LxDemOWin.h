#pragma once
//  MIT License
//
//  Copyright (c) 2021 PipeWarp
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//
//--------------------------------------------------------------------------------
//
//
//
//               LxDemOWin                                      github.com/PipeWarp/LxDemOWin
//               Linux Demo OpenGL and Window Manager           By PipeWarp (copy paste engineer)
//
//
//               A simple and tiny window and OpenGl handler intended for Demoscene development
//
//
//               A lot of this code is botched together from various bits of example code and stuff online, so don't expect quailty
//               
//               Basically, if you see something you don't like in the code, boo-hoo. I'm lazy.
//
//

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <X11/Xlib.h>

#include <X11/Xutil.h>

#include <GL/gl.h>

#include <GL/glx.h>

#include <stdbool.h>

#include <pthread.h>

#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool,
                                                     const int *);

typedef struct LXDOW_Window
{
  Window win;
  char *name;
  int width;
  int height;
  XSetWindowAttributes swa;
  Colormap cmap;
  GLXContext ctx;
  XEvent event;
  Atom wmDeleteMessage;
} LXDOW_Window;

typedef struct LXDOW_Display
{
  Display *display;
  XVisualInfo *vi;
  Colormap cmap;
  GLXFBConfig *fbc, bestFbc;
  XSetWindowAttributes swa;
} LXDOW_Display;

typedef struct LXDOW_Timer
{
  float time;
  pthread_t tithread;
} LXDOW_Timer;

typedef struct LXDOW_Timer_THREADFORMAT
{
  LXDOW_Timer *timer;
  float sleepTime;
} LXDOW_Timer_THREADFORMAT;

typedef struct LXDOW_Shader
{
    unsigned int ID;
} LXDOW_Shader;

static LXDOW_Shader LXDOW_OPENGL_loadShader(const char* vertexD, const char* fragmentD)
{

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexD, NULL);
    
    glCompileShader(vertex);
    

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentD, NULL);
    glCompileShader(fragment);

    unsigned int ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    LXDOW_Shader temp;
    temp.ID = ID;
    return temp;
}


static void LXDOW_TimerUpdate(void *a)
{
  LXDOW_Timer_THREADFORMAT *input = (LXDOW_Timer_THREADFORMAT *)a;
  while (1)
  {
    usleep(input->sleepTime);
    input->timer->time += 0.001;
  }
}

static void LXDOW_StartTimer(LXDOW_Timer *timer, float sleepTime)
{
  LXDOW_Timer_THREADFORMAT *thread = (LXDOW_Timer_THREADFORMAT *)malloc(sizeof(LXDOW_Timer_THREADFORMAT));
  thread->sleepTime = sleepTime;
  thread->timer = timer;
  pthread_create(&timer->tithread, NULL, &LXDOW_TimerUpdate, (void *)thread);
}
static void LXDOW_KillTimer(LXDOW_Timer *timer)
{
  pthread_cancel(timer->tithread);
}
static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev)
{
  ctxErrorOccurred = true;
  return 0;
}

static void LXDOW_updateEvent(LXDOW_Display *display, LXDOW_Window *window)
{
  while (XPending(display->display))
    XNextEvent(display->display, &window->event);
}

static bool LXDOW_OPENGL_isExtensionSupported(const char *extList, const char *extension)
{
  const char *start;
  const char *where, *terminator;

  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if (where || *extension == '\0')
    return false;

  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for (start = extList;;)
  {
    where = strstr(start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return true;

    start = terminator;
  }

  return false;
}

static LXDOW_Display LXDOW_createDisplay()
{
  LXDOW_Display display;
  display.display = XOpenDisplay(NULL);

  if (!display.display)
  {
    // printf("Failed to open X display\n");
  }

  static int visual_attribs[] =
      {
          GLX_X_RENDERABLE, True,
          GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
          GLX_RENDER_TYPE, GLX_RGBA_BIT,
          GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
          GLX_RED_SIZE, 8,
          GLX_GREEN_SIZE, 8,
          GLX_BLUE_SIZE, 8,
          GLX_ALPHA_SIZE, 8,
          GLX_DEPTH_SIZE, 24,
          GLX_STENCIL_SIZE, 8,
          GLX_DOUBLEBUFFER, True,
          // GLX_SAMPLE_BUFFERS  , 1,
          // GLX_SAMPLES         , 4,
          None};

  int glx_major, glx_minor;


  if (!glXQueryVersion(display.display, &glx_major, &glx_minor) ||
      ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
  {
    // printf("Invalid GLX version");
    //
  }


  int fbcount;
  display.fbc = glXChooseFBConfig(display.display, DefaultScreen(display.display), visual_attribs, &fbcount);
  if (!display.fbc)
  {
    // printf( "Failed to retrieve a framebuffer config\n" );
    //
  }
  // printf( "Found %d matching FB configs.\n", fbcount );

  // printf( "Getting XVisualInfos\n" );
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

  int i;
  for (i = 0; i < fbcount; ++i)
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig(display.display, display.fbc[i]);
    if (vi)
    {
      int samp_buf, samples;
      glXGetFBConfigAttrib(display.display, display.fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(display.display, display.fbc[i], GLX_SAMPLES, &samples);

      // printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
      //         " SAMPLES = %d\n",
      //         i, vi -> visualid, samp_buf, samples );

      if (best_fbc < 0 || samp_buf && samples > best_num_samp)
        best_fbc = i, best_num_samp = samples;
      if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
        worst_fbc = i, worst_num_samp = samples;
    }
    XFree(vi);
  }

  display.bestFbc = display.fbc[best_fbc];


  XFree(display.fbc);


  display.vi = glXGetVisualFromFBConfig(display.display, display.bestFbc);
  // printf( "Chosen visual ID = 0x%x\n", display.vi->visualid );

  // printf( "Creating colormap\n" );
  display.swa.colormap = display.cmap = XCreateColormap(display.display,
                                                        RootWindow(display.display, display.vi->screen),
                                                        display.vi->visual, AllocNone);
  display.swa.background_pixmap = None;
  display.swa.border_pixel = 0;
  display.swa.event_mask = StructureNotifyMask;
  // printf("Display Created!");
  return display;
}

static LXDOW_Window LXDOW_createWindow(LXDOW_Display display, char *name, int width, int height)
{
  LXDOW_Window window;
  // printf( "Creating window\n" );
  window.win = XCreateWindow(display.display, RootWindow(display.display, display.vi->screen),
                             0, 0, width, height, 0, display.vi->depth, InputOutput,
                             display.vi->visual,
                             CWBorderPixel | CWColormap | CWEventMask, &display.swa);
  if (!window.win)
  {
    // printf( "Failed to create window.\n" );
    //
  }


  // XFree( vi );

  XStoreName(display.display, window.win, name);

  // printf( "Mapping window\n" );
  XMapWindow(display.display, window.win);
  XSync(display.display, False);
  // printf("Window Created!");
  window.wmDeleteMessage = XInternAtom(display.display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display.display, window.win, &window.wmDeleteMessage, 1);
  return window;
}
static void LXDOW_swapBuffers(LXDOW_Display *display, LXDOW_Window *window)
{
  glXSwapBuffers(display->display, window->win);
}
static void LXDOW_deInitWindow(LXDOW_Display *display, LXDOW_Window *window)
{
  glXMakeCurrent(display->display, 0, 0);
  glXDestroyContext(display->display, window->ctx);
  XDestroyWindow(display->display, window->win);
}
static void LXDOW_deInitDisplay(LXDOW_Display *display)
{
  XFreeColormap(display->display, display->cmap);
  XCloseDisplay(display->display);
}
static void LXDOW_OPENGL_initContext(LXDOW_Display *display, LXDOW_Window *window)
{
  const char *glxExts = glXQueryExtensionsString(display->display,
                                                 DefaultScreen(display->display));

  // NOTE: It is not necessary to create or make current to a context before
  // calling glXGetProcAddressARB
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
      glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

  window->ctx = 0;

  ctxErrorOccurred = 0;
  int (*oldHandler)(Display *, XErrorEvent *) =
      XSetErrorHandler(&ctxErrorHandler);

  // Check for the GLX_ARB_create_context extension string and the function->
  // If either is not present, use GLX 1->3 context creation method->

  int context_attribs[] =
      {
          GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
          GLX_CONTEXT_MINOR_VERSION_ARB, 0,
          // GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
          None};

  // printf( "Creating context\n" );
  window->ctx = glXCreateContextAttribsARB(display->display, display->bestFbc, 0,
                                           True, context_attribs);

  // Sync to ensure any errors generated are processed->
  XSync(display->display, False);
  if (!ctxErrorOccurred && window->ctx)
  {
    // printf( "Created GL 3->0 context\n" );
  }
  else
  {
    // Couldn't create GL 3->0 context->  Fall back to old-style 2->x context->
    // When a context version below 3->0 is requested, implementations will
    // return the newest context version compatible with OpenGL versions less
    // than version 3->0->
    // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
    context_attribs[1] = 1;
    // GLX_CONTEXT_MINOR_VERSION_ARB = 0
    context_attribs[3] = 0;

    ctxErrorOccurred = 0;

    // printf( "Failed to create GL 3->0 context"
    //         " ->->-> using old-style GLX context\n" );
    window->ctx = glXCreateContextAttribsARB(display->display, display->bestFbc, 0,
                                             True, context_attribs);
  }
  glXMakeCurrent(display->display, window->win, window->ctx);

  // Sync to ensure any errors generated are processed->
  XSync(display->display, False);

  // Restore the original error handler
  XSetErrorHandler(oldHandler);

  if (ctxErrorOccurred || !window->ctx)
  {
    // printf( "Failed to create an OpenGL context\n" );
    //
  }

  /*
  // Verifying that context is a direct context
  if ( ! glXIsDirect ( display->display, window->ctx ) )
  {
    //printf( "Indirect GLX rendering context obtained\n" );
  }
  else
  {
    //printf( "Direct GLX rendering context obtained\n" );
  }*/
  glXMakeCurrent(display->display, window->win, window->ctx);
}

static int LXDOW_exitCallback(LXDOW_Window *window){
  switch(window->event.type){
      case ClientMessage:
           if (window->event.xclient.data.l[0] == window->wmDeleteMessage)
               return 1;
            break;
  }
  return 0;
}