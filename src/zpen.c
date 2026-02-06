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
#include <X11/Xlocale.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <locale.h>

#define PI 3.14159265358979323846 /* pi */
#define MAX_COLORS 9
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
  Point *items;
  size_t count, capacity;
} Path;

// https://gist.github.com/rexim/b5b0c38f53157037923e7cdd77ce685d
#define da_append(xs, x)                                                         \
  do                                                                             \
  {                                                                              \
    if ((xs)->count >= (xs)->capacity)                                           \
    {                                                                            \
      if ((xs)->capacity == 0)                                                   \
        (xs)->capacity = 256;                                                    \
      else                                                                       \
        (xs)->capacity *= 2;                                                     \
      (xs)->items = realloc((xs)->items, (xs)->capacity * sizeof(*(xs)->items)); \
    }                                                                            \
    (xs)->items[(xs)->count++] = (x);                                            \
  } while (0)

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void signal_handler(int sig)
{
  printf("Caught signal %d, exiting...\n", sig);
  exit(0);
}

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
  }
  if (format[0] != 'P' || format[1] != '6')
  {
    printf("Invalid PPM format. Only P6 is supported.\n");
  }
  if (maxval != 255)
  {
    printf("Unsupported max value: %d. Only images with maxval=255 are supported..\n", maxval);
  }
  // Allocate memory for image data
  int channels = 3; // RGB
  size_t image_size = width * height * channels;
  unsigned char *data = (unsigned char *)malloc(image_size);
  if (!data)
  {
    printf("Erro ao alocar memória para a imagem.\n");
  }
  // Read image data
  if (fread(data, 1, image_size, file) != image_size)
  {
    printf("Erro ao ler dados da imagem.\n");
    free(data);
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
    // Increase buffer size to accommodate long filenames
    char command[1100]; // Increased from 200 to 1100 bytes
    snprintf(command, sizeof(command), "xclip -selection clipboard -t image/png -i \"%s\"", filename);
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

  // Capture from root window instead of application window for Cinnamon compatibility
  Window root = DefaultRootWindow(d);

  // Translate window-relative coordinates to root window coordinates
  Window child;
  int root_x0, root_y0, root_x1, root_y1;
  XTranslateCoordinates(d, w, root, x0, y0, &root_x0, &root_y0, &child);
  XTranslateCoordinates(d, w, root, x1, y1, &root_x1, &root_y1, &child);

  // Ensure coordinates are properly ordered
  int x = (root_x0 < root_x1) ? root_x0 : root_x1;
  int y = (root_y0 < root_y1) ? root_y0 : root_y1;
  int width = abs(root_x1 - root_x0) + 1;
  int height = abs(root_y1 - root_y0) + 1;

  // Make sure we have valid dimensions
  if (width <= 0 || height <= 0)
  {
    fprintf(stderr, "Invalid screenshot dimensions: %dx%d\n", width, height);
    return;
  }

  image = XGetImage(d, root, x, y, width, height, AllPlanes, ZPixmap);
  if (image == NULL)
  {
    // Fallback to application window if root capture fails
    image = XGetImage(d, w, x, y, width, height, AllPlanes, ZPixmap);
    if (image == NULL)
    {
      fprintf(stderr, "Failed to capture screenshot.\n");
      return;
    }
  }

  saveScreenshotFile(image, toClipboard);
  XDestroyImage(image);
}

static inline void addPoint(Path *p, int x, int y)
{
  da_append(p, ((Point){x, y}));
}

void smoothPath(Path *path, int smoothing_level)
{
  if (smoothing_level <= 1 || path->count < 3)
    return;

  Path smoothed_path = {0};

  for (int i = 0; i < path->count; i++)
  {
    if (i == 0 || i == path->count - 1)
    {
      // Preserve first and last points exactly to avoid gaps
      addPoint(&smoothed_path, path->items[i].x, path->items[i].y);
    }
    else
    {
      // Apply smoothing only to interior points
      int sum_x = 0, sum_y = 0, count = 0;
      for (int j = -smoothing_level; j <= smoothing_level; j++)
      {
        int index = i + j;
        if (index >= 0 && index < path->count)
        {
          sum_x += path->items[index].x;
          sum_y += path->items[index].y;
          count++;
        }
      }
      addPoint(&smoothed_path, sum_x / count, sum_y / count);
    }
  }

  // Copy back the smoothed points
  path->count = 0;
  for (int i = 0; i < smoothed_path.count; i++)
  {
    addPoint(path, smoothed_path.items[i].x, smoothed_path.items[i].y);
  }
  free(smoothed_path.items);
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
              p->items[i - 1].x, p->items[i - 1].y,
              p->items[i].x, p->items[i].y);
  }
}

/**
 * Draw an line on the screen
 * x0, y0: point that marks the tip of the line
 * x1, y1: point that marks the end of the line
 */
void drawLine(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
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
 * draws a rounded rectangle on the screen
 * x0, y0 : point that marks a corner of the rectangle
 * x1, y1 : point that marks the opposite corner of the rectangle
 * */
void drawRoundedRetangle(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);
  int x = (x0 < x1) ? x0 : x1;
  int y = (y0 < y1) ? y0 : y1;

  // Corner radius is proportional to the smaller dimension
  int radius = (width < height ? width : height) / 10;
  if (radius < 3)
    radius = 3;
  if (radius > 15)
    radius = 15;

  int diameter = radius * 2;

  // Draw four corners (arcs)
  // Top-left corner
  XDrawArc(d, w, gc, x, y, diameter, diameter, 90 * 64, 90 * 64);
  // Top-right corner
  XDrawArc(d, w, gc, x + width - diameter, y, diameter, diameter, 0, 90 * 64);
  // Bottom-right corner
  XDrawArc(d, w, gc, x + width - diameter, y + height - diameter, diameter, diameter, 270 * 64, 90 * 64);
  // Bottom-left corner
  XDrawArc(d, w, gc, x, y + height - diameter, diameter, diameter, 180 * 64, 90 * 64);

  // Draw four sides (lines)
  // Top side
  XDrawLine(d, w, gc, x + radius, y, x + width - radius, y);
  // Right side
  XDrawLine(d, w, gc, x + width, y + radius, x + width, y + height - radius);
  // Bottom side
  XDrawLine(d, w, gc, x + radius, y + height, x + width - radius, y + height);
  // Left side
  XDrawLine(d, w, gc, x, y + radius, x, y + height - radius);
}

/**
 * draws a circle on the screen
 * x0, y0 : point that marks a corner of the rectangle
 * x1, y1 : point that marks the opposite corner of the rectangle
 * */
void drawCircle(Display *d, Window w, GC gc, int x0, int y0, int width)
{
  XDrawArc(d, w, gc, x0 - (int)(width / 2), y0 - (int)(width / 2), width, width, 0, 360 * 64);
}

/**
 * draws the horizontal color palette at the bottom-right of the screen
 * */
void drawColorPalette(Display *d, Window w, GC gc, int screen_width, int screen_height, unsigned long color_list[], int selected_color_index)
{
  int circle_size = 8; // Small circle size
  int gap = 4;         // Small gap between circles
  int palette_width = MAX_COLORS * circle_size + (MAX_COLORS - 1) * gap;
  int start_x = screen_width - palette_width - 4; // 4px margin from right edge
  int y = screen_height - 28;                     // Position from bottom edge (accounting for -35 window offset)

  // Save current GC color
  XGCValues values;
  XGetGCValues(d, gc, GCForeground, &values);
  unsigned long original_color = values.foreground;

  for (int i = 0; i < MAX_COLORS; i++)
  {
    int x = start_x + i * (circle_size + gap);

    // Clear the border area first with fully transparent black to erase any previous selection indicator
    XSetForeground(d, gc, 0x00000000);
    XFillArc(d, w, gc, x - circle_size / 2 - 3, y - circle_size / 2 - 3, circle_size + 6, circle_size + 6, 0, 360 * 64);

    // Set color for this circle
    XSetForeground(d, gc, color_list[i]);

    // Draw filled circle
    XFillArc(d, w, gc, x - circle_size / 2, y - circle_size / 2, circle_size, circle_size, 0, 360 * 64);

    if (i == selected_color_index)
    {
      // Add white border for selected color
      XSetForeground(d, gc, 0xFFFFFFFF);
      XDrawArc(d, w, gc, x - circle_size / 2, y - circle_size / 2, circle_size, circle_size, 0, 360 * 64);
    }
  }

  // Restore original GC color
  XSetForeground(d, gc, original_color);
}

/**
 * draws an opening curly brace on the screen
 * x0, y0 : top point of the brace
 * x1, y1 : bottom point of the brace
 * */
void drawOpeningBrace(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);
  int midY = (y0 + y1) / 2;
  int leftX = (x0 < x1) ? x0 : x1;
  int topY = (y0 < y1) ? y0 : y1;
  int rightX = leftX + width;
  int bottomY = topY + height;
  int quarterH = height / 4;
  int sixthW = width / 6;
  int indentAmount = width / 4; // Shallow indent, not all the way to left

  // Opening brace { with shallow middle indent
  // Top horizontal section
  XDrawLine(d, w, gc, leftX + width / 3, topY, rightX - sixthW / 2, topY);
  // Top right curve downward
  XDrawLine(d, w, gc, rightX - sixthW / 2, topY, rightX - sixthW / 3, topY + sixthW / 2);
  XDrawLine(d, w, gc, rightX - sixthW / 3, topY + sixthW / 2, rightX - sixthW / 4, topY + sixthW);
  // Right side to upper middle
  XDrawLine(d, w, gc, rightX - sixthW / 4, topY + sixthW, rightX - sixthW / 4, midY - quarterH / 2);
  // Gentle curve outward for upper middle (angle RIGHT - inverted)
  XDrawLine(d, w, gc, rightX - sixthW / 4, midY - quarterH / 2, rightX + indentAmount, midY - sixthW / 3);
  XDrawLine(d, w, gc, rightX + indentAmount, midY - sixthW / 3, rightX + indentAmount + sixthW / 4, midY);
  // Gentle curve inward for lower middle (back from right bulge)
  XDrawLine(d, w, gc, rightX + indentAmount + sixthW / 4, midY, rightX + indentAmount, midY + sixthW / 3);
  XDrawLine(d, w, gc, rightX + indentAmount, midY + sixthW / 3, rightX - sixthW / 4, midY + quarterH / 2);
  // Right side from lower middle to bottom
  XDrawLine(d, w, gc, rightX - sixthW / 4, midY + quarterH / 2, rightX - sixthW / 4, bottomY - sixthW);
  // Bottom right curve upward
  XDrawLine(d, w, gc, rightX - sixthW / 4, bottomY - sixthW, rightX - sixthW / 3, bottomY - sixthW / 2);
  XDrawLine(d, w, gc, rightX - sixthW / 3, bottomY - sixthW / 2, rightX - sixthW / 2, bottomY);
  // Bottom horizontal section
  XDrawLine(d, w, gc, rightX - sixthW / 2, bottomY, leftX + width / 3, bottomY);
}

/**
 * draws a closing curly brace on the screen
 * x0, y0 : top point of the brace
 * x1, y1 : bottom point of the brace
 * */
void drawClosingBrace(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);
  int midY = (y0 + y1) / 2;
  int leftX = (x0 < x1) ? x0 : x1;
  int topY = (y0 < y1) ? y0 : y1;
  int rightX = leftX + width;
  int bottomY = topY + height;
  int quarterH = height / 4;
  int sixthW = width / 6;
  int bulgeAmount = width / 4; // Shallow bulge, not all the way to right

  // Closing brace } with shallow middle bulge
  // Top horizontal section
  XDrawLine(d, w, gc, rightX - width / 3, topY, leftX + sixthW / 2, topY);
  // Top left curve downward
  XDrawLine(d, w, gc, leftX + sixthW / 2, topY, leftX + sixthW / 3, topY + sixthW / 2);
  XDrawLine(d, w, gc, leftX + sixthW / 3, topY + sixthW / 2, leftX + sixthW / 4, topY + sixthW);
  // Left side to upper middle
  XDrawLine(d, w, gc, leftX + sixthW / 4, topY + sixthW, leftX + sixthW / 4, midY - quarterH / 2);
  // Gentle curve outward for upper middle (angle LEFT - inverted)
  XDrawLine(d, w, gc, leftX + sixthW / 4, midY - quarterH / 2, leftX - bulgeAmount, midY - sixthW / 3);
  XDrawLine(d, w, gc, leftX - bulgeAmount, midY - sixthW / 3, leftX - bulgeAmount - sixthW / 4, midY);
  // Gentle curve inward for lower middle (back from left indent)
  XDrawLine(d, w, gc, leftX - bulgeAmount - sixthW / 4, midY, leftX - bulgeAmount, midY + sixthW / 3);
  XDrawLine(d, w, gc, leftX - bulgeAmount, midY + sixthW / 3, leftX + sixthW / 4, midY + quarterH / 2);
  // Left side from lower middle to bottom
  XDrawLine(d, w, gc, leftX + sixthW / 4, midY + quarterH / 2, leftX + sixthW / 4, bottomY - sixthW);
  // Bottom left curve upward
  XDrawLine(d, w, gc, leftX + sixthW / 4, bottomY - sixthW, leftX + sixthW / 3, bottomY - sixthW / 2);
  XDrawLine(d, w, gc, leftX + sixthW / 3, bottomY - sixthW / 2, leftX + sixthW / 2, bottomY);
  // Bottom horizontal section
  XDrawLine(d, w, gc, leftX + sixthW / 2, bottomY, rightX - width / 3, bottomY);
}

/**
 * draws an opening square bracket on the screen
 * x0, y0 : top point of the bracket
 * x1, y1 : bottom point of the bracket
 * */
void drawOpeningBracket(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);
  int leftX = (x0 < x1) ? x0 : x1;
  int topY = (y0 < y1) ? y0 : y1;
  int rightX = leftX + width;
  int bottomY = topY + height;
  int sixthW = width / 6;

  // Opening bracket [ - simple rectangular shape
  // Top horizontal line
  XDrawLine(d, w, gc, leftX + width / 3, topY, rightX - sixthW / 2, topY);
  // Top right curve downward
  XDrawLine(d, w, gc, rightX - sixthW / 2, topY, rightX - sixthW / 3, topY + sixthW / 2);
  XDrawLine(d, w, gc, rightX - sixthW / 3, topY + sixthW / 2, rightX - sixthW / 4, topY + sixthW);
  // Right side vertical line (no middle angle)
  XDrawLine(d, w, gc, rightX - sixthW / 4, topY + sixthW, rightX - sixthW / 4, bottomY - sixthW);
  // Bottom right curve upward
  XDrawLine(d, w, gc, rightX - sixthW / 4, bottomY - sixthW, rightX - sixthW / 3, bottomY - sixthW / 2);
  XDrawLine(d, w, gc, rightX - sixthW / 3, bottomY - sixthW / 2, rightX - sixthW / 2, bottomY);
  // Bottom horizontal line
  XDrawLine(d, w, gc, rightX - sixthW / 2, bottomY, leftX + width / 3, bottomY);
}

/**
 * draws a closing square bracket on the screen
 * x0, y0 : top point of the bracket
 * x1, y1 : bottom point of the bracket
 * */
void drawClosingBracket(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  int height = abs(y1 - y0);
  int width = abs(x1 - x0);
  int leftX = (x0 < x1) ? x0 : x1;
  int topY = (y0 < y1) ? y0 : y1;
  int rightX = leftX + width;
  int bottomY = topY + height;
  int sixthW = width / 6;

  // Closing bracket ] - simple rectangular shape
  // Top horizontal line
  XDrawLine(d, w, gc, rightX - width / 3, topY, leftX + sixthW / 2, topY);
  // Top left curve downward
  XDrawLine(d, w, gc, leftX + sixthW / 2, topY, leftX + sixthW / 3, topY + sixthW / 2);
  XDrawLine(d, w, gc, leftX + sixthW / 3, topY + sixthW / 2, leftX + sixthW / 4, topY + sixthW);
  // Left side vertical line (no middle angle)
  XDrawLine(d, w, gc, leftX + sixthW / 4, topY + sixthW, leftX + sixthW / 4, bottomY - sixthW);
  // Bottom left curve upward
  XDrawLine(d, w, gc, leftX + sixthW / 4, bottomY - sixthW, leftX + sixthW / 3, bottomY - sixthW / 2);
  XDrawLine(d, w, gc, leftX + sixthW / 3, bottomY - sixthW / 2, leftX + sixthW / 2, bottomY);
  // Bottom horizontal line
  XDrawLine(d, w, gc, leftX + sixthW / 2, bottomY, rightX - width / 3, bottomY);
}

/**
 * draws a square bracket on the screen (auto-detects opening/closing based on direction)
 * x0, y0 : first point of the bracket
 * x1, y1 : second point of the bracket
 * */
void drawBracket(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  // If dragging left to right, draw opening bracket
  // If dragging right to left, draw closing bracket
  if (x1 >= x0)
  {
    drawOpeningBracket(d, w, gc, x0, y0, x1, y1);
  }
  else
  {
    drawClosingBracket(d, w, gc, x0, y0, x1, y1);
  }
}

/**
 * draws a curly brace on the screen (auto-detects opening/closing based on direction)
 * x0, y0 : first point of the brace
 * x1, y1 : second point of the brace
 * */
void drawBrace(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1)
{
  // If dragging left to right, draw opening brace
  // If dragging right to left, draw closing brace
  if (x1 >= x0)
  {
    drawOpeningBrace(d, w, gc, x0, y0, x1, y1);
  }
  else
  {
    drawClosingBrace(d, w, gc, x0, y0, x1, y1);
  }
}

/**
 * Initializes undo levels that will contain "screenshots" of the state of the undo
 */
void initUndo(Pixmap *p, Display *d, Window w, unsigned int width, unsigned int height, int depth, int undoMax)
{
  for (int i = 0; i < undoMax; i++)
  {
    p[i] = XCreatePixmap(d, w, width, height, depth);
  }
}

void bye(Display *d, Window w)
{
  XUndefineCursor(d, w);
  XCloseDisplay(d);
  exit(0);
}

/**
 * Draw text with a cursor at the end
 */
void drawTextWithCursor(Display *d, Window w, GC gc, XFontSet fontset, int x, int y, const char *text, int cursor_height)
{
  int text_width = 0;
  if (fontset && text && strlen(text) > 0)
  {
    XRectangle ink, logical;
    XmbTextExtents(fontset, text, strlen(text), &ink, &logical);
    text_width = logical.width;
    XmbDrawString(d, w, fontset, gc, x, y, text, strlen(text));
  }
  // Draw cursor (vertical line)
  XDrawLine(d, w, gc, x + text_width + 2, y - cursor_height + 4, x + text_width + 2, y + 4);
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
    setCursor(d, w, cursor, XC_ur_angle);
    break;
  case 'l':
    setCursor(d, w, cursor, XC_tcross);
    break;
  case 'k':
    setCursor(d, w, cursor, XC_left_side);
    break;
  case 'b':
    setCursor(d, w, cursor, XC_left_side);
    break;
  }
}

////////////////////////
// MAIN
////////////////////////
int main()
{
  // Set up locale for international text input
  setlocale(LC_ALL, "");

  // Set up signal handlers
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  int color_index = 0;
  unsigned long color_list[MAX_COLORS] = {
      0xFFFF3333 /* red */,
      0xFF00FF00 /* green */,
      0xFF3333FF /* blue */,
      0xFFFFFF33 /* yellow */,
      0xFFFFA500 /* orange */,
      0xFFFFFFFF /* white */,
      0xFFFF00FF /* magenta */,
      0xFFFFC0CB /* pink */,
      0xFF808080 /* gray */
  };
  int f_screenshot = 0;
  Display *d;
  Window w;
  int screen;
  XEvent e;
  GC gc;
  Cursor cursor;
  GC gcPreDraw;
  XPoint rect[2];
  XPoint pointPreDraw;

  char shape = 'a';
  char prv_shape = shape;
  int skipNextEsc = 0;
  int roundedRect = 1;
  unsigned long color = color_list[0];
  int thickness = THICKNESS;
  int drawing = 0;
  Path path = {0};
  path.count = 0;
  pointPreDraw.x = -1;
  pointPreDraw.y = -1;
  int p = 0;
  int stepCnt = 1;

  d = XOpenDisplay(NULL);
  if (d == NULL)
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  screen = XDefaultScreen(d);
  unsigned int height = DisplayHeight(d, screen);
  unsigned int width = DisplayWidth(d, screen);
  Window root = DefaultRootWindow(d);

  // Capture initial screenshot before creating window to freeze the desktop
  XImage *bgImage = XGetImage(d, root, 0, 0, width, height, AllPlanes, ZPixmap);
  if (!bgImage)
  {
    fprintf(stderr, "Failed to capture background screenshot\n");
    XCloseDisplay(d);
    exit(1);
  }

  XVisualInfo vinfo;
  XMatchVisualInfo(d, screen, 32, TrueColor, &vinfo);

  // Create window with override_redirect to bypass WM and cover entire screen (including panels)
  XSetWindowAttributes attrs;
  attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
  attrs.event_mask = ExposureMask | ButtonPressMask | KeyPressMask |
                     ButtonReleaseMask | ButtonMotionMask | KeyReleaseMask |
                     FocusChangeMask | StructureNotifyMask;
  attrs.border_pixel = 0;
  attrs.background_pixel = 0;
  attrs.override_redirect = True;

  w = XCreateWindow(d, root, 0, 0, width, height, 0,
                    vinfo.depth, InputOutput, vinfo.visual,
                    CWEventMask | CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect, &attrs);

  // Remove window decorations (title bar) using Motif hints
  struct
  {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
  } hints = {2, 0, 0, 0, 0}; // flags=2 means decorations field is valid, decorations=0 means none
  Atom motif_hints = XInternAtom(d, "_MOTIF_WM_HINTS", False);
  XChangeProperty(d, w, motif_hints, motif_hints, 32, PropModeReplace,
                  (unsigned char *)&hints, 5);

  // Create GC
  gc = XCreateGC(d, w, 0, NULL);
  XSetForeground(d, gc, color);
  XSetLineAttributes(d, gc, thickness, LineSolid, CapRound, JoinMiter);

  XGCValues gcValuesPreDraw;
  gcValuesPreDraw.function = GXxor;
  gcValuesPreDraw.foreground = color;
  gcPreDraw = XCreateGC(d, w, GCForeground + GCFunction, &gcValuesPreDraw);
  XSetLineAttributes(d, gcPreDraw, thickness > 2 ? thickness - 2 : 1, LineDoubleDash, CapRound, JoinMiter);

  // Map window
  XMapWindow(d, w);
  XFlush(d);

  // Wait for window to be mapped and get focus
  usleep(100000);

  // Fill entire window with opaque black so no transparent gaps remain
  XSetForeground(d, gc, 0xFF000000);
  XFillRectangle(d, w, gc, 0, 0, width, height);
  XSetForeground(d, gc, color);

  // Copy frozen background to window - window is at (0,0) matching the captured screenshot
  {
    // Convert 24-bit root window capture to 32-bit (add alpha=0xFF) to match our window depth
    XImage *bg32 = XCreateImage(d, vinfo.visual, 32, ZPixmap, 0, NULL,
                                width, height, 32, 0);
    bg32->data = malloc(bg32->bytes_per_line * height);
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        unsigned long pixel = XGetPixel(bgImage, x, y);
        XPutPixel(bg32, x, y, pixel | 0xFF000000);
      }
    }
    XPutImage(d, w, gc, bg32, 0, 0, 0, 0, width, height);
    XDestroyImage(bg32);
  }
  XDestroyImage(bgImage);

  // Set input focus to our window
  XSetInputFocus(d, w, RevertToParent, CurrentTime);
  XRaiseWindow(d, w);

  // Set up XIM for international text input (composed characters like ç, á, ã)
  XIM xim = NULL;
  XIC xic = NULL;
  if (XSupportsLocale())
  {
    XSetLocaleModifiers("");
    xim = XOpenIM(d, NULL, NULL, NULL);
    if (xim)
    {
      xic = XCreateIC(xim,
                      XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                      XNClientWindow, w,
                      XNFocusWindow, w,
                      NULL);
    }
  }

  // Create fontset for UTF-8 text rendering
  char **missing_list;
  int missing_count;
  char *default_string;
  XFontSet fontset = XCreateFontSet(d, "-*-helvetica-medium-r-*-*-18-*-*-*-*-*-*-*,-*-*-medium-r-*-*-18-*-*-*-*-*-*-*",
                                    &missing_list, &missing_count, &default_string);
  if (missing_count > 0)
    XFreeStringList(missing_list);

  // Prepare undo/redo levels
  int maxUndo = 0;
  int undoLevel = 0;
  int maxRedo = 0;
  int redoLevel = 0;
  Pixmap undoStack[UNDO_MAX];
  Pixmap redoStack[UNDO_MAX];
  initUndo(undoStack, d, w, width, height, vinfo.depth, UNDO_MAX);
  initUndo(redoStack, d, w, width, height, vinfo.depth, UNDO_MAX);

  // Draw color palette before initializing undo stack so it's included in saved states
  drawColorPalette(d, w, gc, width, height, color_list, color_index);
  XSync(d, False); // Ensure palette is drawn before copying to undo stack

  // Initialize undo stack with background (including color palette)
  for (int i = 0; i < UNDO_MAX; i++)
  {
    XCopyArea(d, w, undoStack[i], gc, 0, 0, width, height, 0, 0);
  }
  XFlush(d);

  setShapeCursor(d, w, &cursor, shape);

  // Text input variables
  char text[256] = {0};
  int l_text = 0;
  int t_text = 0;
  int x_text = 0;
  int y_text = 0;
  Pixmap textPixMap = XCreatePixmap(d, w, width, height, vinfo.depth);

  enum
  {
    KeyMod_LShift = 1 << 0,
    KeyMod_LAlt = 1 << 1,
  };
  int key_mods = 0;

  // Main event loop
  while (1)
  {
    XNextEvent(d, &e);

    // Let XIM process the event for dead key composition (é, á, ã, etc.)
    if (XFilterEvent(&e, None))
      continue; // Event was consumed by XIM, skip processing

    // Handle focus events to ensure we keep keyboard focus
    if (e.type == FocusOut)
    {
      // Regain focus if we lose it
      XSetInputFocus(d, w, RevertToParent, CurrentTime);
      continue;
    }
    switch (e.type)
    {
    case ButtonPress:
      rect[p].x = e.xbutton.x;
      rect[p].y = e.xbutton.y;
      p++;
      if (shape == 'p')
      {
        drawing = 1;
        path.count = 0;
        addPoint(&path, e.xbutton.x, e.xbutton.y);
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
      }
      break;

    case Expose:
      break;

    case ButtonRelease:
      rect[p].x = e.xbutton.x;
      rect[p].y = e.xbutton.y;
      switch (shape)
      {
      case 'p':
        if (drawing)
        {
          drawing = 0;
          XCopyArea(d, undoStack[undoLevel], w, gc, 0, 0, width, height, 0, 0);
          smoothPath(&path, SMOOTHING_LEVEL);
          drawPath(d, w, gc, &path);
          undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
          maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
          maxRedo = 0;
          redoLevel = 0;
        }
        break;

      case 'c':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawCircle(d, w, gcPreDraw, rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
        }
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawCircle(d, w, gc, rect[0].x, rect[0].y, abs(rect[1].x - rect[0].x));
        break;

      case 'r':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          if (roundedRect && !f_screenshot)
            drawRoundedRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          else
            drawRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        if (f_screenshot)
        {
          // Sync display and wait for compositor to update before capture
          XSync(d, False);
          usleep(50000); // 50ms delay for compositor
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
          XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
          undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
          maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
          maxRedo = 0;
          redoLevel = 0;
          if (roundedRect)
            drawRoundedRetangle(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
          else
            drawRetangle(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
        }
        f_screenshot = 0;
        setShapeCursor(d, w, &cursor, shape);
        break;

      case 'a':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawArrow(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
        }
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawArrow(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y, ARROW_SIZE);
        break;

      case 'l':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawLine(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawLine(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
        break;

      case 'k':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawBrace(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawBrace(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
        break;

      case 'b':
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
        {
          drawBracket(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        }
        XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
        undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : ++undoLevel;
        maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : ++maxUndo;
        drawBracket(d, w, gc, rect[0].x, rect[0].y, rect[1].x, rect[1].y);
        break;
      }
      p = 0;
      pointPreDraw.x = -1;
      pointPreDraw.y = -1;
      break;

    case MotionNotify:
      if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0)
      {
        switch (shape)
        {
        case 'c':
          drawCircle(d, w, gcPreDraw, rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
          break;
        case 'r':
          if (roundedRect && !f_screenshot)
            drawRoundedRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          else
            drawRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'a':
          drawArrow(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
          break;
        case 'l':
          drawLine(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'k':
          drawBrace(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'b':
          drawBracket(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
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
          addPoint(&path, e.xmotion.x, e.xmotion.y);
          if (path.count > 1)
          {
            XDrawLine(d, w, gcPreDraw,
                      path.items[path.count - 2].x, path.items[path.count - 2].y,
                      path.items[path.count - 1].x, path.items[path.count - 1].y);
          }
        }
        break;
      case 'c':
        drawCircle(d, w, gcPreDraw, rect[0].x, rect[0].y, abs(pointPreDraw.x - rect[0].x));
        break;
      case 'r':
        if (roundedRect && !f_screenshot)
          drawRoundedRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        else
          drawRetangle(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'a':
        drawArrow(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y, ARROW_SIZE);
        break;
      case 'l':
        drawLine(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'k':
        drawBrace(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'b':
        drawBracket(d, w, gcPreDraw, rect[0].x, rect[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      }
      XFlush(d);
      break;

    case KeyPress:
      if (t_text)
      {
        KeySym key = NoSymbol;
        Status status;
        char ltext[64];
        int n = 0;

        // Use XIM if available for composed character support (ç, á, ã, etc.)
        if (xic)
        {
          n = Xutf8LookupString(xic, &e.xkey, ltext, sizeof(ltext) - 1, &key, &status);
          if (status == XBufferOverflow)
            n = 0; // Buffer too small, ignore
        }
        else
        {
          n = XLookupString(&e.xkey, ltext, sizeof(ltext) - 1, &key, NULL);
        }
        ltext[n] = 0x00;

        if (key == XK_Return || e.xkey.keycode == 104)
        {
          // Enter: commit current line and start new line below
          // First redraw without cursor to commit clean text
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          if (fontset && strlen(text) > 0)
            XmbDrawString(d, w, fontset, gc, x_text, y_text, text, strlen(text));
          XCopyArea(d, w, textPixMap, gc, 0, 0, width, height, 0, 0);
          l_text = 0;
          *text = 0x00;
          y_text += 24; // Move to next line (approx line height for 18pt font)
          // Draw cursor on new line
          drawTextWithCursor(d, w, gc, fontset, x_text, y_text, text, 18);
          XFlush(d);
        }
        else if (key == XK_BackSpace && l_text > 0)
        {
          // Remove last UTF-8 character (may be multiple bytes)
          while (l_text > 0 && (text[l_text - 1] & 0xC0) == 0x80)
            l_text--; // Skip continuation bytes
          if (l_text > 0)
            l_text--; // Remove the start byte
          text[l_text] = 0x00;
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          drawTextWithCursor(d, w, gc, fontset, x_text, y_text, text, 18);
          XFlush(d);
        }
        else if (n > 0 && (unsigned char)ltext[0] >= 32 && l_text + n < sizeof(text) - 1)
        {
          // Accept any printable character (including UTF-8 multi-byte)
          // First clear previous cursor
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          strcat(text, ltext);
          l_text += n;
          drawTextWithCursor(d, w, gc, fontset, x_text, y_text, text, 18);
          XFlush(d);
        }
        if (e.xkey.keycode == 0x09)
        {
          // ESC: commit text and return to previous drawing tool
          XClearWindow(d, w);
          XCopyArea(d, textPixMap, w, gc, 0, 0, width, height, 0, 0);
          if (fontset && strlen(text) > 0)
            XmbDrawString(d, w, fontset, gc, x_text, y_text, text, strlen(text));
          t_text = 0;
          l_text = 0;
          *text = 0x00;
          shape = prv_shape;
          setShapeCursor(d, w, &cursor, shape);
          skipNextEsc = 1;
        }
      }
      else
      {
        if (e.xkey.keycode == 0x09)
        {
          if (skipNextEsc)
            skipNextEsc = 0;
          else
            bye(d, w);
        }
        else if (e.xkey.keycode == 50)
        {
          key_mods |= KeyMod_LShift;
        }
        else if (e.xkey.keycode == 64)
        {
          key_mods |= KeyMod_LAlt;
        }
        else if (e.xkey.keycode == 54)
        {
          shape = 'c';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 27)
        {
          if (shape == 'r')
          {
            roundedRect = !roundedRect;
          }
          else
          {
            shape = 'r';
            p = 0;
          }
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 33 && !(key_mods & (KeyMod_LShift | KeyMod_LAlt)))
        {
          shape = 'p';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 38)
        {
          shape = 'a';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 46)
        {
          shape = 'l';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 45)
        {
          shape = 'k';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 41)
        {
          prv_shape = shape;
          shape = 'r';
          p = 0;
          f_screenshot = 1;
          setCursor(d, w, &cursor, XC_icon);
          XSetForeground(d, gcPreDraw, 0xFFFFFFFF);
        }
        else if (e.xkey.keycode == 39)
        {
          prv_shape = shape;
          shape = 'r';
          p = 0;
          f_screenshot = 2;
          setCursor(d, w, &cursor, XC_icon);
          XSetForeground(d, gcPreDraw, 0xFFFFFFFF);
        }
        else if (e.xkey.keycode == 28)
        {
          prv_shape = shape;
          XCopyArea(d, w, textPixMap, gc, 0, 0, width, height, 0, 0);
          setCursor(d, w, &cursor, XC_xterm);
          x_text = e.xbutton.x;
          y_text = e.xbutton.y;
          t_text = 1;
          // Draw initial cursor
          drawTextWithCursor(d, w, gc, fontset, x_text, y_text, text, 18);
          XFlush(d);
        }
        else if (e.xkey.keycode == 65)
        {
          color_index = (color_index + 1) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawColorPalette(d, w, gc, width, height, color_list, color_index);
        }
        else if (e.xkey.keycode == 114)
        {
          color_index = (color_index + 1) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawColorPalette(d, w, gc, width, height, color_list, color_index);
        }
        else if (e.xkey.keycode == 113)
        {
          color_index = (color_index - 1 + MAX_COLORS) % MAX_COLORS;
          XSetForeground(d, gc, color_list[color_index]);
          XSetForeground(d, gcPreDraw, color_list[color_index]);
          drawColorPalette(d, w, gc, width, height, color_list, color_index);
        }
        else if (e.xkey.keycode == 21 || e.xkey.keycode == 86)
        {
          // + key or numpad +: increase pen thickness
          if (thickness < 20)
          {
            thickness++;
            XSetLineAttributes(d, gc, thickness, LineSolid, CapRound, JoinMiter);
            XSetLineAttributes(d, gcPreDraw, thickness > 2 ? thickness - 2 : 1, LineDoubleDash, CapRound, JoinMiter);
          }
        }
        else if (e.xkey.keycode == 20 || e.xkey.keycode == 82)
        {
          // - key or numpad -: decrease pen thickness
          if (thickness > 1)
          {
            thickness--;
            XSetLineAttributes(d, gc, thickness, LineSolid, CapRound, JoinMiter);
            XSetLineAttributes(d, gcPreDraw, thickness > 2 ? thickness - 2 : 1, LineDoubleDash, CapRound, JoinMiter);
          }
        }
        else if (e.xkey.keycode == 19 || e.xkey.keycode == 90)
        {
          // 0 key or numpad 0: reset pen thickness to default
          thickness = THICKNESS;
          XSetLineAttributes(d, gc, thickness, LineSolid, CapRound, JoinMiter);
          XSetLineAttributes(d, gcPreDraw, thickness > 2 ? thickness - 2 : 1, LineDoubleDash, CapRound, JoinMiter);
        }
        else if (e.xkey.keycode == 56)
        {
          shape = 'b';
          p = 0;
          setShapeCursor(d, w, &cursor, shape);
        }
        else if (e.xkey.keycode == 57)
        {
          XFontStruct *ft = XLoadQueryFont(d, FONT);
          XSetFont(d, gc, ft->fid);
          char s[4] = {'(', stepCnt + '0', ')', '\0'};
          stepCnt++;
          if (stepCnt >= 9)
            stepCnt = 0;
          XDrawString(d, w, gc, e.xbutton.x, e.xbutton.y, s, strlen(s));
        }
        else if ((e.xkey.state & ControlMask) && (e.xkey.state & ShiftMask) &&
                 (e.xkey.keycode == 52 || e.xkey.keycode == 29))
        {
          if (maxRedo > 0)
          {
            XCopyArea(d, w, undoStack[undoLevel], gc, 0, 0, width, height, 0, 0);
            undoLevel = (undoLevel >= UNDO_MAX - 1) ? 0 : undoLevel + 1;
            maxUndo = (maxUndo >= UNDO_MAX) ? UNDO_MAX : maxUndo + 1;
            redoLevel = (redoLevel == 0) ? UNDO_MAX - 1 : redoLevel - 1;
            maxRedo = (maxRedo < 0) ? 0 : maxRedo - 1;
            XCopyArea(d, redoStack[redoLevel], w, gc, 0, 0, width, height, 0, 0);
          }
        }
        else if (e.xkey.keycode == 30 ||
                 ((e.xkey.state & ControlMask) && !(e.xkey.state & ShiftMask) &&
                  (e.xkey.keycode == 52 || e.xkey.keycode == 29)))
        {
          if (maxUndo > 0)
          {
            XCopyArea(d, w, redoStack[redoLevel], gc, 0, 0, width, height, 0, 0);
            redoLevel = (redoLevel >= UNDO_MAX - 1) ? 0 : redoLevel + 1;
            maxRedo = (maxRedo >= UNDO_MAX) ? UNDO_MAX : maxRedo + 1;
            undoLevel = (undoLevel == 0) ? UNDO_MAX - 1 : undoLevel - 1;
            maxUndo = (maxUndo < 0) ? 0 : maxUndo - 1;
            XCopyArea(d, undoStack[undoLevel], w, gc, 0, 0, width, height, 0, 0);
          }
        }
      }
      break;

    case KeyRelease:
      if (t_text)
      {
      }
      else
      {
        if (e.xkey.keycode == 0x09)
        {
          if (skipNextEsc)
            skipNextEsc = 0;
          else
            bye(d, w);
        }
        else if (e.xkey.keycode == 50)
        {
          key_mods &= ~KeyMod_LShift;
        }
        else if (e.xkey.keycode == 64)
        {
          key_mods &= ~KeyMod_LAlt;
        }
        /*
        else if (e.xkey.keycode == 33 && (key_mods & (KeyMod_LShift|KeyMod_LAlt))==(KeyMod_LShift|KeyMod_LAlt))
        {
          printf("KEKW\n");
          attrs.event_mask = ExposureMask | FocusChangeMask | StructureNotifyMask;
          XChangeWindowAttributes(d, w, CWEventMask, &attrs);
          XLowerWindow(d, w);
          XSetInputFocus(d, PointerRoot, RevertToPointerRoot, CurrentTime);
        }
        */
      }
      break;
    }
  }
  return 0;
}
