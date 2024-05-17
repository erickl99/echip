#include "graphics.h"
#include "vm.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

extern xdisplay xd;

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: echip file\n");
    return 1;
  }

  size_t buff_size = WIDTH * HEIGHT * PIXEL_BYTES * 64;
  char *raw_buffer = malloc(buff_size);
  printf("Buffer starts at %p, ends at %p\n", raw_buffer,
         raw_buffer + buff_size);
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

  struct timespec frame_start;
  struct timespec frame_finish;
  clock_gettime(CLOCK_REALTIME, &frame_start);
  unsigned long target_frame_time = 1000000000 / 60;
  int close_window = 0;
  while (!close_window) {
    XEvent event = {0};
    while (!close_window && XPending(xd.display) > 0) {
      printf("Received an event\n");
      XNextEvent(xd.display, &event);
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
          printf("Received exit button.\n");
          close_app_window();
          printf("Finsihed closing window.\n");
          close_window = 1;
        }
        break;
      }
      }
    }
    if (close_window) {
      break;
    }
    step();
    clock_gettime(CLOCK_REALTIME, &frame_finish);
    unsigned long frame_time =
        (frame_finish.tv_sec - frame_start.tv_sec) * 1000000000 +
        (frame_finish.tv_nsec - frame_start.tv_nsec);
    struct timespec sleep_time = {.tv_nsec = target_frame_time - frame_time};
    nanosleep(&sleep_time, 0);
    clock_gettime(CLOCK_REALTIME, &frame_finish);
    frame_start = frame_finish;
  }
  free_vm();
  printf("Success\n");
  return 0;
}
