/*
  @name: zPen
  @description: A Xlib drawing tool
  @author: Marco Aurelio Zoqui <marco_at_zoqui_dot_com>
  @version: 0.1
*/
// Dependencies:
// For clipboard management it requires the xclip program
// sudo apt install xclip
//
// Hot to compile:
// gcc zpen.c -o zpen -lX11 -lm
//

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#define PI 3.14159265358979323846 /* pi */
#define MAX_POINTS 10000
#define MAX_COLORS 6
#define SMOOTHING_LEVEL 7
#define SMOOTHED_LINE_WIDTH 4
#define THICKNESS 3
#define UNDO_MAX 20
#define ARROW_SIZE 20
// xlsfonts | grep courier
// #define FONT "-*-*-*-*-*-*-60-*-*-*-*-*-iso8859-*"
#define FONT "*-helvetica-*-18-*"

typedef struct
{
  int x, y;
} Point;

typedef struct
{
  Point points[MAX_POINTS];
  int count;
} Path;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int convert_ppm_to_png(const char *ppm_path, const char *png_path)
{
  FILE *file = fopen(ppm_path, "rb");
  if (!file)
  {
    printf("Error opening PPM file: %s\n", ppm_path);
    return 0;
  }

  // Read PPM Header
  char format[3];
  int width, height, maxval;
  if (fscanf(file, "%2s\n%d %d\n%d\n", format, &width, &height, &maxval) != 4)
  {
    printf("Error reading PPM file header.\n");
    fclose(file);
    return 0;
  }
  if (format[0] != 'P' || format[1] != '6')
  {
    printf("Invalid PPM format. Only P6 is supported.\n");
    fclose(file);
    return 0;
  }
  if (maxval != 255)
  {
    printf("Unsupported max value: %d. Only images with maxval=255 are supported.\n", maxval);
    fclose(file);
    return 0;
  }

  // Allocate memory for image data
  int channels = 3; // RGB
  size_t image_size = width * height * channels;
  unsigned char *data = (unsigned char *)malloc(image_size);
  if (!data)
  {
    printf("Error allocating memory for image data.\n");
    fclose(file);
    return 0;
  }

  // Read image data
  if (fread(data, 1, image_size, file) != image_size)
  {
    printf("Error reading image data.\n");
    free(data);
    fclose(file);
    return 0;
  }
  fclose(file);

  // Write the image in PNG format
  if (!stbi_write_png(png_path, width, height, channels, data, width * channels))
  {
    printf("Error saving PNG file: %s\n", png_path);
    free(data);
    return 0;
  }

  free(data);
  return 1;
}

int convert_ppm_to_jpeg(const char *ppm_path, const char *jpeg_path, int quality)
{
  FILE *file = fopen(ppm_path, "rb");
  if (!file)
  {
    printf("Error opening PPM file: %s\n", ppm_path);
    return 0;
  }
  // Read PPM Header
  char format[3];
  int width, height, maxval;
  if (fscanf(file, "%2s\n%d %d\n%d\n", format, &width, &height, &maxval) != 4)
  {
    printf("Error reading PPM file header.\n");
    fclose(file);
    return 0;
  }
  if (format[0] != 'P' || format[1] != '6')
  {
    printf("Invalid PPM format. Only P6 is supported.\n");
    fclose(file);
    return 0;
  }
  if (maxval != 255)
  {
    printf("Unsupported max value: %d. Only images with maxval=255 are supported..\n", maxval);
    fclose(file);
    return 0;
  }
  // Allocate memory for image data
  int channels = 3; // RGB
  size_t image_size = width * height * channels;
  unsigned char *data = (unsigned char *)malloc(image_size);
  if (!data)
  {
    printf("Erro ao alocar memória para a imagem.\n");
    fclose(file);
    return 0;
  }
  // Read image data
  if (fread(data, 1, image_size, file) != image_size)
  {
    printf("Erro ao ler dados da imagem.\n");
    free(data);
    fclose(file);
    return 0;
  }
  fclose(file);
  // Write the image in JPEG format
  if (!stbi_write_jpg(jpeg_path, width, height, channels, data, quality))
  {
    printf("Error saving JPEG file: %s\n", jpeg_path);
    free(data);
    return 0;
  }
  free(data);
  return 1;
}

/**
 * Get the full path of the ~/.zpen directory
 * Returns a dynamically allocated string that must be freed by the caller
 */
char *get_zpen_directory()
{
  const char *home_dir;
  char *zpen_dir;
  // Try to get HOME from the environment
  home_dir = getenv("HOME");
  if (!home_dir)
  {
    // If HOME is not set, try to get it from passwd
    struct passwd *pwd = getpwuid(getuid());
    if (pwd)
      home_dir = pwd->pw_dir;
    else
      return NULL;
  }
  // Allocates space for the full path
  size_t len = strlen(home_dir) + strlen("/.zpen") + 1;
  zpen_dir = malloc(len);
  if (!zpen_dir)
    return NULL;
  snprintf(zpen_dir, len, "%s/.zpen", home_dir);
  return zpen_dir;
}

/**
 * Creates the ~/.zpen directory if it does not exist
 * Returns 0 on success, -1 on error
 */
int ensure_zpen_directory()
{
  char *zpen_dir = get_zpen_directory();
  if (!zpen_dir)
    return -1;
  struct stat st;
  if (stat(zpen_dir, &st) == -1)
  {
    if (errno == ENOENT)
    {
      // Directory does not exist, let&#39;s create it with permissions 700 (rwx------)
      if (mkdir(zpen_dir, S_IRWXU) == -1)
      {
        free(zpen_dir);
        return -1;
      }
    }
    else
    {
      free(zpen_dir);
      return -1;
    }
  }
  free(zpen_dir);
  return 0;
}

/*
convert to png
*/
void saveScreenshotFile(XImage *image, int toClipboard)
{
  // Ensures the ~/.zpen directory exists
  if (ensure_zpen_directory() == -1)
  {
    fprintf(stderr, "Failed to create or access .zpen directory\n");
    return;
  }
  // Save first to temporary PPM file
  FILE *f = fopen("/tmp/screenshot.ppm", "wb");
  if (f == NULL)
  {
    fprintf(stderr, "Failed to open temporary file for writing.\n");
    return;
  }
  fprintf(f, "P6\n%d %d\n255\n", image->width, image->height);
  for (int y = 0; y < image->height; y++)
  {
    for (int x = 0; x < image->width; x++)
    {
      int pixel = XGetPixel(image, x, y);
      unsigned char red = (pixel & image->red_mask) >> 16;
      unsigned char green = (pixel & image->green_mask) >> 8;
      unsigned char blue = pixel & image->blue_mask;

      fputc(red, f);
      fputc(green, f);
      fputc(blue, f);
    }
  }
  fclose(f);
  // Save as png (jpeg will be used in future. right now xclip does not support it)
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  char filename[1024];
  const char *home = getenv("HOME");
  if (!home)
  {
    printf("Error: Could not get HOME directory.\n");
    return;
  }
  snprintf(filename, sizeof(filename), "%s/.zpen/img%04d%02d%02d%02d%02d%02d.png",
           home,
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
           tm.tm_hour, tm.tm_min, tm.tm_sec);

  if (!convert_ppm_to_png("/tmp/screenshot.ppm", filename))
  {
    printf("PPM file to JPEG conversion failed.\n");
  }
  remove("/tmp/screenshot.ppm"); // Clean the temporary file
  if (toClipboard)
  {
    char command[200];
    snprintf(command, sizeof(command), "xclip -selection clipboard -t image/png -i %s", filename);
    int result = system(command);
    if (result == -1)
    {
      perror("Something went wrong with xclip call");
    }
  }
}

void saveScreenshot(Display *d, Window w, int screen, int x0, int y0, int x1, int y1, int toClipboard)
{
  XImage *image;
  image = XGetImage(d, w, x0, y0, x1 - x0, y1 - y0, AllPlanes, ZPixmap);
  if (image == NULL)
  {
    fprintf(stderr, "Failed to capture screenshot.\n");
    return;
  }
  saveScreenshotFile(image, toClipboard);
  XDestroyImage(image);
}

void addPoint(Path *p, int x, int y)
{
  if (p->count < MAX_POINTS)
  {
    p->points[p->count].x = x;
    p->points[p->count].y = y;
    p->count++;
  }
}

void smoothPath(Path *path, int smoothing_level)
{
  if (smoothing_level <= 1)
    return;

  Point smoothed_points[MAX_POINTS];
  int smoothed_count = 0;

  for (int i = 0; i < path->count; i++)
  {
    int sum_x = 0, sum_y = 0, count = 0;
    for (int j = -smoothing_level; j <= smoothing_level; j++)
    {
      int index = i + j;
      if (index >= 0 && index < path->count)
      {
        sum_x += path->points[index].x;
        sum_y += path->points[index].y;
        count++;
      }
    }
    smoothed_points[smoothed_count].x = sum_x / count;
    smoothed_points[smoothed_count].y = sum_y / count;
    smoothed_count++;
  }

  // Copy back the smoothed points
  for (int i = 0; i < smoothed_count; i++)
  {
    path->points[i] = smoothed_points[i];
  }
  path->count = smoothed_count;
}

/**
 * Draw an path
 * p: array of points (x,y)
 */
void drawPath(Display *d, Window w, GC gc, Path *p)
{
  for (int i = 1; i < p->count; i++)
  {
    XDrawLine(d, w, gc,
              p->points[i - 1].x, p->points[i - 1].y,
              p->points[i].x, p->points[i].y);
  }
}

/**
 * Draw an line on the screen
 * x0, y0: point that marks the tip of the line
 * x1, y1: point that marks the end of the line
 */
void drawLine(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  // XDrawLine(d, w, gc, x0, y0, x1, y1);
  XDrawLine(d, w, gc, x0, y0, x1, y1);
}

/**
 * Draw an arrow on the screen
 * x0, y0: point that marks the tip of the line
 * x1, y1: point that marks the end of the line
 */
void drawArrow(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1, int arrowSize)
{
  drawLine(d, w, gc, x0, y0, x1, y1);
  float angle = atan2(y0 - y1, x0 - x1) + PI;
  float arrowAngle = PI / 4;
  int f_x1 = x1 - arrowSize * cos(angle + arrowAngle);
  int f_y1 = y1 - arrowSize * sin(angle + arrowAngle);
  int f_x2 = x1 - arrowSize * cos(angle - arrowAngle);
  int f_y2 = y1 - arrowSize * sin(angle - arrowAngle);
  drawLine(d, w, gc, x1, y1, f_x1, f_y1);
  drawLine(d, w, gc, x1, y1, f_x2, f_y2);
}

/**
 * draws a rectangle on the screen
 * x0, y0 : point that marks a corner of the rectangle
 * x1, y1 : point that marks the opposite corner of the rectangle
 * */
void drawRetangle(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);

  if (x0 <= x1)
  {
    if (y0 <= y1)
    {
      XDrawRectangle(d, w, gc, x0, y0, width, height);
    }
    else
    {
      XDrawRectangle(d, w, gc, x0, y1, width, height);
    }
  }
  else
  {
    if (y0 <= y1)
    {
      XDrawRectangle(d, w, gc, x1, y0, width, height);
    }
    else
    {
      XDrawRectangle(d, w, gc, x1, y1, width, height);
    }
  }
}

/**
 * draws a circle on the screen
 * x0, y0 : point that marks a corner of the rectangle
 * x1, y1 : point that marks the opposite corner of the rectangle
 * */
void drawCircle(Display *d, Window w, GC gc, int x0, int y0, int width)
{
  // int width = abs(x1 - x0);
  XDrawArc(d, w, gc, x0 - (int)(width / 2), y0 - (int)(width / 2), width, width, 0, 360 * 64);
}

/**
 * Initializes undo levels that will contain "screenshots" of the state of the undo
 */
void initUndo(Pixmap *p, Display *d, Window w, int screen, unsigned int width, unsigned int height, int undoMax)
{
  for (int i = 0; i < undoMax; i++)
  {
    p[i] = XCreatePixmap(d, w, width, height, XDefaultDepth(d, screen));
  }
}

void bye(Display *d, Window w)
{
  XUndefineCursor(d, w);
  XCloseDisplay(d);
  exit(0);
}

void setCursor(Display *d, Window w, Cursor *cursor, int cursorId)
{
  *cursor = XCreateFontCursor(d, cursorId);
  XDefineCursor(d, w, *cursor);
  XSync(d, False);
}

void setShapeCursor(Display *d, Window w, Cursor *cursor, char shape)
{
  switch (shape)
  {
  case 'c':
    setCursor(d, w, cursor, XC_dot);
    break;
  case 'p':
    setCursor(d, w, cursor, XC_pencil);
    break;
  case 'r':
    setCursor(d, w, cursor, XC_sizing);
    break;
  case 'a':
    // setCursor(d, w, cursor, XC_right_ptr);
    setCursor(d, w, cursor, XC_ur_angle);
    break;
  case 'l':
    setCursor(d, w, cursor, XC_tcross);
    break;
  }
}

////////////////////////
// MAIN
////////////////////////
int main()
{
  int color_index = 0;
  int color_list[MAX_COLORS] = {
      0xFF3333 /* red */,
      0x00FF00 /* green */,
      0x3333FF /* blue */,
      0xFFFF33 /* yellow */,
      0xFFA500 /* orange */,
      0xFFFFFF /* white */
  };
  int f_screenshot = 0;
  Display *d;
  Window w;
  int screen;
  XEvent e;
  GC gc;
  Cursor cursor;
  GC gcPreDraw;        // To pre-draw the shape before painting it
  XPoint rect[2];      // 2 points to draw a rectangle
  XPoint pointPreDraw; // initial incorrect values, so as not to paint it without having the first point chosen

  // Freehand Pen
  char shape = 'a'; // Initial TOOL
  char prv_shape = shape;
  long color = color_list[0];
  int drawing = 0;
  Path path;
  path.count = 0;
  pointPreDraw.x = -1;
  pointPreDraw.y = -1;
  int p = 0; // points counter
  int stepCnt = 1;
  d = XOpenDisplay(NULL);
  if (d == NULL)
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  screen = XDefaultScreen(d);

  unsigned int height = DisplayHeight(d, screen); // height and width of the screen
  unsigned int width = DisplayWidth(d, screen);

  w = XCreateWindow(d, DefaultRootWindow(d), -10, -35, width + 10, height + 35, CopyFromParent,
                    CopyFromParent, CopyFromParent, CopyFromParent, CopyFromParent, CopyFromParent);

  // We change to full screen
  Atom atoms[2] = {XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", False), None};
  XChangeProperty(d, w, XInternAtom(d, "_NET_WM_STATE", False), XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)atoms, 1);

  // We create the GC (Graphic Context) that we will use along with its attributes
  gc = XCreateGC(d, w, 0, NULL);

  XSetForeground(d, gc, color);
  XSetLineAttributes(d, gc, THICKNESS, LineSolid, CapRound, JoinMiter);

  XGCValues gcValuesPreDraw;
  gcValuesPreDraw.function = GXxor;
  gcValuesPreDraw.foreground = color; // 0xAACCFF;
  gcPreDraw = XCreateGC(d, w, GCForeground + GCFunction, &gcValuesPreDraw);
  XSetLineAttributes(d, gcPreDraw, THICKNESS - 2, LineDoubleDash, CapRound, JoinMiter);

  // The events that we are going to listen to
  XSelectInput(d, w,
               ExposureMask | ButtonPressMask | KeyPressMask |
                   ButtonReleaseMask | ButtonMotionMask);

  // We show the window
  XMapWindow(d, w);

  // Prepare the undo levels
  int maxUndo = 0;
  int undoLevel = 0;
  Pixmap undoStack[UNDO_MAX];
  initUndo(undoStack, d, w, screen, width, height, UNDO_MAX);

  // https://tronche.com/gui/x/xlib/appendix/b/
  setShapeCursor(d, w, &cursor, shape);

  // BEGIN Text input
  char text[256] = {0}; /* a char buffer for KeyPress Events */
  int l_text = 0;       // text length
  int t_text = 0;       // text input flag (1 typing | 0 not typing)
  int x_text = 0;       // initial x cursor position
  int y_text = 0;
  Pixmap textPixMap = XCreatePixmap(d, w, width, height, XDefaultDepth(d, screen));
  // END Text input
  drawCircle(d, w, gc, width - 10, height - 10, 2);
  while (True)
  {
    XNextEvent(d, &e);
    switch (e.type)
    {
    case ButtonPress: /* Initial position of the tool */
      rect[p].x = e.xbutton.x;
      rect[p].y = e.xbutton.y;
      p++;
      if (shape == 'p')
      {
        drawing = 1;
        path.count = 0; // Clear the current path
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
      }
    case Expose:
      break;
    case ButtonRelease: /* End position of the tool */
      rect[p].x = e.xbutton.x;
      rect[p].y = e.xbutton.y;
      switch (shape)
      {
      case 'p':
      {
        if (drawing)
        {
          drawing = 0;
          XCopyArea(d, undoStack[undoLevel], w, gc, 0, 0, width, height, 0, 0);
          smoothPath(&path, SMOOTHING_LEVEL);
          drawPath(d, w, gc, &path);
          undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
          maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        }
      }
      break;
      case 'c':
      {
        // predraw is deleted before saving the undo level
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawCircle(d, w, gcPreDraw, rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
        }
        // save the background at the current position and increment it
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0); // save the current background
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawCircle(d, w, gc, rect[0].x, rect[0].y, abs(rect[1].x - rect[0].x));
      }
      break;
      case 'r':
      { // predraw is deleted before saving the undo level
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        if (f_screenshot)
        {
          saveScreenshot(d, w, screen, rect[0].x, rect[0].y, rect[1].x, rect[1].y, f_screenshot == 2 ? 1 : 0);
          XSetForeground(d, gcPreDraw, color);
          shape = prv_shape;
          if (f_screenshot == 2)
          {
            bye(d, w);
          }
        }
        else
        {
          // save the background at the current position and increment it
          XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0); // save the current background
          undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
          maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
          drawRetangle(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
        }
        f_screenshot = 0;
        setShapeCursor(d, w, &cursor, shape);
      }
      break;
      case 'a':
      { // predraw is deleted before saving the undo level
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawArrow(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
        }
        // save the background at the current position and increment it
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0); // save the current background
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawArrow(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y, ARROW_SIZE);
      }
      break;
      case 'l':
      { // predraw is deleted before saving the undo level
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawLine(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        // save the background at the current position and increment it
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0); // save the current background
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawLine(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
      }
      break;
      }
      // restart the points and the predraw tool
      p = 0;
      pointPreDraw.x = -1;
      pointPreDraw.y = -1;
      break;
    case MotionNotify: /* tool predraw */
      if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
      {
        // predraw is deleted before saving the undo level
        switch (shape)
        {
        case 'c':
          drawCircle(d, w, gcPreDraw,
                     rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
          break;
        case 'r':
          drawRetangle(d, w, gcPreDraw,
                       rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'a':
          drawArrow(d, w, gcPreDraw,
                    rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
          break;
        case 'l':
          drawLine(d, w, gcPreDraw,
                   rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        }
      }
      pointPreDraw.x = e.xmotion.x;
      pointPreDraw.y = e.xmotion.y;
      switch (shape)
      {
      case 'p':
        if (drawing)
        {
          addPoint(&path, e.xbutton.x, e.xbutton.y);
          if (path.count > 1)
          {
            drawPath(d, w, gcPreDraw, &path);
          }
        }
        break;
      case 'c':
        drawCircle(d, w, gcPreDraw,
                   rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
        break;
      case 'r':
        drawRetangle(d, w, gcPreDraw,
                     rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'a':
        drawArrow(d, w, gcPreDraw,
                  rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
        break;
      case 'l':
        drawLine(d, w, gcPreDraw,
                 rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      }
      XFlush(d);
      break;
    case KeyPress:
      // xmodmap -pke # all keys
      // https://stackoverflow.com/questions/12343987/convert-ascii-character-to-x11-keycode/25771958#25771958
      // https://gist.github.com/javiercantero/7753445
      if (t_text) // text typing
      {
        KeySym key;
        char ltext[25];
        int n = XLookupString(&e.xkey, ltext, sizeof(ltext) - 1, &key, NULL);
        ltext[n] = 0x00;
        if (key == XK_Return || e.xkey.keycode == 104)
        {
          l_text = 0;
          t_text = 0;
          *text = 0x00;
          setCursor(d, w, &cursor, XC_pencil);
        }
        else if (key == XK_BackSpace && l_text > 0)
        {
          text[--l_text] = 0x00;
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          XDrawString(d, w, gc, x_text, y_text, text, strlen(text));
          XFlush(d);
        }
        else if (n > 0 && isprint(ltext[0]) && l_text < sizeof(text) - 1)
        {
          strcat(text, ltext);
          l_text += n;
          XFontStruct *ft = XLoadQueryFont(d, FONT);
          XSetFont(d, gc, ft->fid);
          XDrawString(d, w, gc, x_text, y_text, text, strlen(text));
        }
        if (e.xkey.keycode == 0x09 /* ESC */)
        {
          t_text = 0;
          l_text = 0;
          *text = 0x00;
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          setCursor(d, w, &cursor, XC_pencil);
        }
      }
      else // not typing
      {
        if (e.xkey.keycode == 0x09 /* ESC */)
        {
          bye(d, w);
        }
        else if (e.xkey.keycode == 54 /* c cicle*/)
        {
          shape = 'c';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 27 /* r retangle*/)
        {
          shape = 'r';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 33 /* p pen */)
        {
          shape = 'p';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 38 /* a arrow*/)
        {
          shape = 'a';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 46 /* l line */)
        {
          shape = 'l';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 41 /* f save screenshot to file save*/)
        {
          prv_shape = shape;
          shape = 'r';
          p = 0;
          f_screenshot = 1;
          setCursor(d, w, &cursor, XC_icon);
          XSetForeground(d, gcPreDraw, 0xFFFFFF);
        }
        else if (e.xkey.keycode == 39 /* s save screenshot, copy it to the clipboard and exit*/)
        {
          prv_shape = shape;
          shape = 'r';
          p = 0;
          f_screenshot = 2;
          setCursor(d, w, &cursor, XC_icon);
          XSetForeground(d, gcPreDraw, 0xFFFFFF);
        }
        else if (e.xkey.keycode == 28 /* t inject text*/)
        {
          // userText(d, w, gc, e.xbutton.x, e.xbutton.y);
          XCopyArea(d, w, textPixMap, gc, 0, 0, width, height, 0, 0); // save the current
          setCursor(d, w, &cursor, XC_xterm);
          x_text = e.xbutton.x;
          y_text = e.xbutton.y;
          t_text = 1;
        }
        else if (e.xkey.keycode == 65 /* space next color*/)
        {
          color_index = (color_index + 1) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 114 /* RIGHT arrow next color*/)
        {
          color_index = (color_index + 1) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 113 /* LEFT arrow previous color*/)
        {
          color_index = (color_index - 1 + MAX_COLORS) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 19 /* 0 (reset to red) */)
        {
          color = 0xFF3333;
          XSetForeground(d, gc, color);
          XSetForeground(d, gcPreDraw, color);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 29 /* y yellow */)
        {
          color = 0xFFFF33;
          XSetForeground(d, gc, color);
          XSetForeground(d, gcPreDraw, color);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 56 /* b blue */)
        {
          color = 0x3333FF;
          XSetForeground(d, gc, color);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 25 /* w white */)
        {
          color = 0xFFFFFF;
          XSetForeground(d, gc, color);
          XSetForeground(d, gcPreDraw, color);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 42 /* g green */)
        {
          color = 0x00FF00;
          XSetForeground(d, gc, color);
          XSetForeground(d, gcPreDraw, color);
          drawCircle(d, w, gc, width - 10, height - 10, 2);
        }
        else if (e.xkey.keycode == 57 /* n number */)
        {
          XFontStruct *ft = XLoadQueryFont(d, FONT);
          XSetFont(d, gc, ft->fid);
          char s[4] = {'(', stepCnt + '0', ')', '\0'};
          stepCnt++;
          if (stepCnt >= 9)
            stepCnt = 0;
          XDrawString(d, w, gc, e.xbutton.x, e.xbutton.y, s, strlen(s));
        }
        else if (e.xkey.keycode == 30 /* u undo */)
        {
          if (maxUndo > 0)
          { // if there are levels that can be undone
            undoLevel = (undoLevel == 0) ? UNDO_MAX - 1 : --undoLevel;
            maxUndo = (maxUndo < 0) ? 0 : --maxUndo;
            // restore the saved background
            XCopyArea(d, undoStack[undoLevel], w, gc, 0, 0, width, height, 0, 0);
          }
        }
      }
    }
  }
  return 0;
}
