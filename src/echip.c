#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define HEIGHT 320
#define WIDTH 320

#define BIT_DEPTH 24

#define PIXEL_BITS 32
#define PIXEL_BYTES 4

#define WHITE 0x00FFFFFF
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
      if (x % 16 && y % 16) {
        *px = color;
      } else {
        *px = 0;
      }
    }
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

void decode(unsigned char *bytes, size_t size) {
  printf("We received %lu\n", size);
  for (int i = 0; i < size; i = i + 2) {
    unsigned char left_nibble = bytes[i] >> 4;
    unsigned char right_nibble = bytes[i] & 31;
    printf("Left: %02x, Right: %02x, Full: %02x, Arg: %02x\n", left_nibble,
           right_nibble, bytes[i], bytes[i + 1]);
    switch (left_nibble) {
    case 0:
      printf("This is a clear screen or return instruction\n");
      break;
    case 1:
      printf("This is a jump instruction.\n");
      break;
    case 6:
      printf("This is a set register instruction\n");
      break;
    case 7:
      printf("This is an add value to register instruction\n");
      break;
    case 10:
      printf("This is set index register instruction\n");
      break;
    case 13:
      printf("This is a draw instruction\n");
      break;
    default:
      printf("Unrecognized opcode\n");
      break;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: echip file\n");
    return 1;
  }

  char *filename = argv[1];
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "File %s not found.\n", filename);
  }
  struct stat file_info;
  fstat(fileno(fp), &file_info);
  size_t file_size = file_info.st_size;
  unsigned char *bytes = malloc(file_size);
  size_t bytes_read = fread(bytes, 1, file_size, fp);
  if (bytes_read != file_size) {
    fprintf(stderr, "An error occurred while trying to read the file.\n");
    free(bytes);
    fclose(fp);
    return 1;
  }
  decode(bytes, file_size);
  free(bytes);
  fclose(fp);
  return 0;
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
      break;
    }
    case KeyRelease: {
      printf("You released a key\n");
      break;
    }
    }
  }

  XCloseDisplay(display);
  XDestroyImage(image_buffer);
  printf("Success\n");
  return 0;
}
