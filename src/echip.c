#include "vm.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define HEIGHT 512
#define WIDTH 256

#define BIT_DEPTH 24

#define PIXEL_BITS 32
#define PIXEL_BYTES 4

#define WHITE 0x00FFFFFF
#define BLACK 0x00000000
#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define PURPLE 0x00FF00FF

void color_buffer(char *buffer, unsigned int color, int width, int height) {
  int pitch = width * PIXEL_BYTES;
  for (int y = 0; y < height; y++) {
    char *row = buffer + (y * pitch);
    for (int x = 0; x < width; x++) {
      unsigned int *px = (unsigned int *)(row + (x * PIXEL_BYTES));
      if (x % 8 && y % 8) {
        *px = color;
      } else {
        *px = BLACK;
      }
    }
  }
}

void switch_pixel(char *buffer, unsigned char x, unsigned char y) {
  int pitch = WIDTH * PIXEL_BYTES;
  for (int i = 0; i < 8; i++) {
    char *start = buffer + (8 * y + i) * pitch + 8 * PIXEL_BYTES * x;
    for (int j = 0; j < 8; j++) {
      unsigned int *px = (unsigned int *)(start + j * PIXEL_BYTES);
      if (*px == WHITE) {
        *px = BLACK;
      } else {
        *px = WHITE;
      }
    }
  }
}

void color_pixels(char *buffer, unsigned char x, unsigned char y, unsigned char height) {
  unsigned char mod_x = x % 32;
  unsigned char mod_y = y % 64;
  for (unsigned char i = 0; i < height && mod_y + i < 64; i++) {
    switch_pixel(buffer, mod_x, mod_y + i);
  }
}

Window create_app_window(Display *display, Window root, XVisualInfo vis_info) {
  XSetWindowAttributes win_attr;
  win_attr.background_pixel = WHITE;
  win_attr.colormap =
      XCreateColormap(display, root, vis_info.visual, AllocNone);
  win_attr.event_mask = StructureNotifyMask | KeyPressMask;
  unsigned long win_attr_mask = CWBackPixel | CWColormap | CWEventMask;
  Window app_window =
      XCreateWindow(display, root, 0, 0, WIDTH, HEIGHT, 0, vis_info.depth,
                    InputOutput, vis_info.visual, win_attr_mask, &win_attr);

  XSizeHints size_hints = {0};
  size_hints.flags = PMinSize | PMaxSize;
  size_hints.min_width = WIDTH;
  size_hints.max_width = WIDTH;
  size_hints.min_height = HEIGHT;
  size_hints.min_height = HEIGHT;
  XSetWMNormalHints(display, app_window, &size_hints);
  return app_window;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: echip file\n");
    return 1;
  }
  if (init_vm(argv[1]) < 0) {
    fprintf(stderr, "Failed to start vm.\n");
    exit(1);
  }
  for (int i = 0; i < 5; i++) {
    step();
  }
  free_vm();
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "No display available.\n");
    exit(1);
  }
  Window root = XDefaultRootWindow(display);
  int screen = XDefaultScreen(display);

  XVisualInfo vis_info = {0};
  if (!XMatchVisualInfo(display, screen, BIT_DEPTH, TrueColor, &vis_info)) {
    fprintf(stderr, "No matching visual info!\n");
  }

  Window app_window = create_app_window(display, root, vis_info);

  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, app_window, &wm_delete_window, 1);

  XStoreName(display, app_window, "Echip");
  XMapWindow(display, app_window);
  XSync(display, False);

  size_t buff_size = WIDTH * HEIGHT * PIXEL_BYTES;
  char *raw_buffer = malloc(buff_size);
  if (raw_buffer == NULL) {
    fprintf(stderr, "Failed to create window drawing buffer.\n");
  }
  XImage *image_buffer =
      XCreateImage(display, vis_info.visual, vis_info.depth, ZPixmap, 0,
                   raw_buffer, WIDTH, HEIGHT, PIXEL_BITS, 0);
  GC gc = DefaultGC(display, screen);

  int close_window = 0;
  while (!close_window) {
    XEvent event = {0};
    XNextEvent(display, &event);
    switch (event.type) {
    case ClientMessage: {
      if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
        XDestroyWindow(display, app_window);
        close_window = 1;
        printf("Shutting down the application...\n");
      }
      break;
    }
    case DestroyNotify: {
      if (event.xdestroywindow.window == app_window) {
        close_window = 1;
        printf("Window was closed by window manager, shutting down...\n");
      }
      break;
    }
    case KeyPress: {
      if (event.xkey.keycode == XKeysymToKeycode(display, XK_R)) {
        printf("Painting the canvas red\n");
        color_buffer(raw_buffer, RED, WIDTH, HEIGHT);
        XPutImage(display, app_window, gc, image_buffer, 0, 0, 0, 0, WIDTH,
                  HEIGHT);
      }
      if (event.xkey.keycode == XKeysymToKeycode(display, XK_B)) {
        printf("Painting the canvas blue\n");
        color_buffer(raw_buffer, BLUE, WIDTH, HEIGHT);
        XPutImage(display, app_window, gc, image_buffer, 0, 0, 0, 0, WIDTH,
                  HEIGHT);
      }
      if (event.xkey.keycode == XKeysymToKeycode(display, XK_C)) {
        printf("Painting a sprite\n");
        color_pixels(raw_buffer, 2, 68, 15);
        XPutImage(display, app_window, gc, image_buffer, 0, 0, 0, 0, WIDTH,
                  HEIGHT);
      }
      break;
    }
    }
  }

  XCloseDisplay(display);
  XDestroyImage(image_buffer);
  printf("Success\n");
  return 0;
}
