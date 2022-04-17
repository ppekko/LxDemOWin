#pragma once
//  MIT License
//
//  Copyright (c) 2022 PipeWarp
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
//              Insert the line
//                  #define LXDEMOWIN_IMPLEMENTATION
//              before including this header in *one* C++ file to create the implementation.
//

#ifdef LXDEMOWIN_IMPLEMENTATION
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <X11/Xlib.h>

#include <X11/Xutil.h>

#include <GL/gl.h>

#include <GL/glx.h>

#define false 0
#define true 1
#define bool uint_fast8_t

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
  pthread_create(&timer->tithread, NULL, (void* (*)(void*))LXDOW_TimerUpdate, (void *)thread);
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

  where = strchr(extension, ' ');
  if (where || *extension == '\0')
    return false;


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

  uint_fast8_t glx_major, glx_minor;





  uint_fast8_t fbcount;
  display.fbc = glXChooseFBConfig(display.display, DefaultScreen(display.display), visual_attribs, &fbcount);

  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

  uint_fast8_t i;
  for (i = 0; i < fbcount; ++i)
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig(display.display, display.fbc[i]);
    if (vi)
    {
      uint_fast16_t samp_buf, samples;
      glXGetFBConfigAttrib(display.display, display.fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(display.display, display.fbc[i], GLX_SAMPLES, &samples);

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

  display.swa.colormap = display.cmap = XCreateColormap(display.display,
                                                        RootWindow(display.display, display.vi->screen),
                                                        display.vi->visual, AllocNone);
  display.swa.background_pixmap = None;
  display.swa.border_pixel = 0;
  display.swa.event_mask = StructureNotifyMask;
  return display;
}

static LXDOW_Window LXDOW_createWindow(LXDOW_Display display, char *name, int width, int height)
{
  LXDOW_Window window;
  window.win = XCreateWindow(display.display, RootWindow(display.display, display.vi->screen),
                             0, 0, width, height, 0, display.vi->depth, InputOutput,
                             display.vi->visual,
                             CWBorderPixel | CWColormap | CWEventMask, &display.swa);


  // XFree( vi );

  XStoreName(display.display, window.win, name);


  XMapWindow(display.display, window.win);
  XSync(display.display, False);

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

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
      glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");

  window->ctx = 0;

  ctxErrorOccurred = 0;
  int (*oldHandler)(Display *, XErrorEvent *) =
      XSetErrorHandler(&ctxErrorHandler);


  int context_attribs[] =
      {
          GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
          GLX_CONTEXT_MINOR_VERSION_ARB, 0,
          // GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
          None};

  window->ctx = glXCreateContextAttribsARB(display->display, display->bestFbc, 0,
                                           True, context_attribs);


  XSync(display->display, False);
  if (!ctxErrorOccurred && window->ctx)
  {
    // Created GL 3->0 context
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


    window->ctx = glXCreateContextAttribsARB(display->display, display->bestFbc, 0,
                                             True, context_attribs);
  }
  glXMakeCurrent(display->display, window->win, window->ctx);


  XSync(display->display, False);

  XSetErrorHandler(oldHandler);





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
#endif
