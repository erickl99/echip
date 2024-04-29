#ifndef graphics_h
#define graphics_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define HEIGHT 216
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
void draw_sprite(char *buffer, unsigned char x, unsigned char y, unsigned char *sprite, unsigned char height);

#endif // !graphics_h
