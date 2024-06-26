#ifndef graphics_h
#define graphics_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "types.h"

#define HEIGHT 256
#define WIDTH 512
#define WHITE 0x00FFFFFF
#define BLACK 0x00000000
#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define PURPLE 0x00FF00FF
#define PIXEL_BITS 32
#define PIXEL_BYTES 4
#define BIT_DEPTH 24

typedef struct {
  Window app_window;
  Display *display;
  XImage *image;
  Atom wm_delete_window;
  GC gc;
} xdisplay;

int create_app_window(char *buffer);
void close_app_window();
void clear_screen(char *buffer);
void draw_sprite(char *buffer, uint8 x, uint8 y, uint8 *sprite, uint8 height);

#endif // !graphics_h
