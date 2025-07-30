# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

To compile the zPen application:
```bash
cd src
gcc zpen.c -o zpen -lX11 -lm
```

The build requires X11 development libraries (`libx11-dev`) and math library (`-lm`).

## Architecture Overview

zPen is a single-file C application (`src/zpen.c`) that creates a fullscreen X11 drawing overlay on Linux systems. The architecture consists of:

### Core Components
- **X11 Window Management**: Creates a fullscreen transparent overlay window using Xlib
- **Drawing Engine**: Implements various drawing tools (pen, rectangle, circle, line, arrow, text)
- **Event Loop**: Handles mouse and keyboard events for tool selection and drawing operations
- **Undo System**: Maintains up to 20 pixmap snapshots for undo functionality
- **Screenshot System**: Captures screen regions and saves as PNG files using stb_image_write

### Key Data Structures
- `Point`: Basic coordinate structure (x, y)
- `Path`: Collection of points for freehand drawing with smoothing
- Pixmap array for undo stack (`undoStack[UNDO_MAX]`)

### Drawing Tools
- **Pen ('p')**: Freehand drawing with path smoothing
- **Rectangle ('r')**: Click-and-drag rectangles
- **Circle ('c')**: Radius-based circles
- **Line ('l')**: Straight lines
- **Arrow ('a')**: Lines with arrowheads
- **Text ('t')**: Text input at cursor position

### File Management
- Screenshots saved to `~/.zpen/` directory
- Filenames: `img[YYYYMMDDHHMMSS].png`
- Uses stb_image_write.h for PNG export
- Temporary PPM files for conversion process

### Dependencies
- X11 libraries (`-lX11`)
- Math library (`-lm`)
- xclip program for clipboard operations
- stb_image_write.h (included as header-only library)

### Key Features
- Real-time drawing preview using XOR graphics context
- Color palette with 6 predefined colors
- Keyboard shortcuts for all tools and colors
- Screenshot capture with clipboard integration
- Fullscreen overlay with transparency support