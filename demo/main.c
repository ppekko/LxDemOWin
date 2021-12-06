#include "LxDemOWin.h"
#define WinWidth 640
#define WinHeight 480


void _start()
{
  asm volatile("sub $8, %rsp\n");
  LXDOW_Display display = LXDOW_createDisplay();
  LXDOW_Window window = LXDOW_createWindow(display, NULL, WinWidth, WinHeight);

  LXDOW_OPENGL_initContext(&display, &window);

  while (1)
  {
    LXDOW_updateEvent(&display, &window);

    glClearColor(1, 1, 1, 1);

    glClear(GL_COLOR_BUFFER_BIT);

    LXDOW_exitCallback(&window);
    LXDOW_swapBuffers(&display, &window);

  }
  LXDOW_deInitWindow(&display, &window);
  LXDOW_deInitDisplay(&display);
  asm ( \
  "movl $1,%eax\n" \
  "xor %ebx,%ebx\n" \
  "int $128\n" \
  );
}
