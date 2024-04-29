#include "graphics.h"
#include "vm.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

extern xdisplay xd;

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: echip file\n");
    return 1;
  }

  size_t buff_size = WIDTH * HEIGHT * PIXEL_BYTES;
  char *raw_buffer = malloc(buff_size);
  if (raw_buffer == NULL) {
    fprintf(stderr, "Failed to create window drawing buffer.\n");
  }
  if (init_vm(argv[1], raw_buffer) < 0) {
    fprintf(stderr, "Failed to start vm.\n");
    exit(1);
  }
  if (create_app_window(raw_buffer) < 0) {
    exit(1);
  }

  int close_window = 0;
  while (!close_window) {
    XEvent event = {0};
    XNextEvent(xd.display, &event);
    printf("Waiting for an event...\n");
    switch (event.type) {
    case ClientMessage: {
      if ((Atom)event.xclient.data.l[0] == xd.wm_delete_window) {
        printf("Shutting down the application...\n");
        close_app_window();
        close_window = 1;
      }
      break;
    }
    case DestroyNotify: {
      if (event.xdestroywindow.window == xd.app_window) {
        close_window = 1;
        printf("Window was closed by window manager, shutting down...\n");
      }
      break;
    }
    case KeyPress: {
      if (event.xkey.keycode == XKeysymToKeycode(xd.display, XK_Q)) {
        close_app_window();
        close_window = 1;
        break;
      }
      if (event.xkey.keycode == XKeysymToKeycode(xd.display, XK_S)) {
        step();
      }
      break;
    }
    }
  }
  free_vm();
  printf("Success\n");
  return 0;
}
