#include "graphics.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

xdisplay xd;

int create_app_window(char *buffer) {
  Display *display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "No display available.\n");
    return -1;
  }
  Window root = XDefaultRootWindow(display);
  int screen = XDefaultScreen(display);
  XVisualInfo vis_info = {0};
  if (!XMatchVisualInfo(display, screen, BIT_DEPTH, TrueColor, &vis_info)) {
    fprintf(stderr, "No matching visual info!\n");
    XCloseDisplay(display);
    return -1;
  }
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
  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, app_window, &wm_delete_window, 1);

  XStoreName(display, app_window, "Echip");
  XMapWindow(display, app_window);
  XSync(display, False);
  xd.image = XCreateImage(display, vis_info.visual, vis_info.depth, ZPixmap, 0,
                          buffer, WIDTH, HEIGHT, PIXEL_BITS, 0);
  xd.gc = DefaultGC(display, screen);
  xd.display = display;
  xd.app_window = app_window;
  xd.wm_delete_window = wm_delete_window;
  return 0;
}

void close_app_window() {
  XDestroyWindow(xd.display, xd.app_window);
  XCloseDisplay(xd.display);
  XDestroyImage(xd.image);
}

static void switch_pixel(char *buffer, unsigned char x, unsigned char y) {
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

void draw_sprite(char *buffer, unsigned char x, unsigned char y,
                 unsigned char *sprite, unsigned char height) {
  unsigned char mod_x = x % 64;
  unsigned char mod_y = y % 32;
  for (unsigned char i = 0; i < height && mod_y + i < 32; i++) {
    unsigned char mask = 128;
    for (unsigned char j = 0; j < 8 && mod_x + j < 64; j++) {
      if ((sprite[i] & mask) > 0) {
        switch_pixel(buffer, mod_x + j, mod_y + i);
      }
      mask >>= 1;
    }
  }
  XPutImage(xd.display, xd.app_window, xd.gc, xd.image, 0, 0, 0, 0, WIDTH, HEIGHT);
}

void clear_screen(char *buffer) {
  int pitch = WIDTH * PIXEL_BYTES;
  for (int y = 0; y < HEIGHT; y++) {
    char *row = buffer + (y * pitch);
    for (int x = 0; x < WIDTH; x++) {
      unsigned int *px = (unsigned int *)(row + (x * PIXEL_BYTES));
      *px = BLACK;
    }
  }
  XPutImage(xd.display, xd.app_window, xd.gc, xd.image, 0, 0, 0, 0, WIDTH, HEIGHT);
}
