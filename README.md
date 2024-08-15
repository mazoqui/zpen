# zpen

**zPen** is a drawing tool for Linux systems written using Xlib. It allows you to create various shapes, lines, and freehand drawings on your screen. Additionally, you can take screenshots, change pen colors, and undo/redo your actions.
## Features

- **Drawing Tools:**
    
    - **Freehand Drawing:** Draw freehand on the screen with smoothing capabilities.
    - **Rectangles:** Draw rectangles by clicking and dragging.
    - **Circles:** Draw circles by defining the center and radius.
    - **Lines:** Draw straight lines by specifying start and end points.
    - **Arrows:** Draw arrows to point out specific areas on the screen.
- **Screenshot Capture:**
    
    - Capture the current state of the screen as a screenshot and save it as a `.png` file.
- **Undo Functionality:**
    
    - Undo up to 20 previous drawing actions.
- **Color Selection:**
    
    - Choose from a palette of predefined colors to customize your drawings.
- **Step Counter:**
    
    - Display a step counter for drawings made with the freehand tool.

## Usage

- Use the mouse to draw on the screen.
- Click and hold to start drawing freehand shapes.
- Release the mouse button to complete the shape.
- Press the following keys to switch between tools:
    - `c`: Circle
    - `r`: Rectangle
    - `p`: Pen (Freehand drawing)
    - `a`: Arrow
    - `l`: Line
- Press `space` to change the pen color or press:
	- `y`: Yellow
	- `g`: Green
	- `b`: Blue
	- `w`: White
	- `0`: Reset to Red
- Press `s` to take a screenshot and save it as a PNG file.
- Press `u` to undo the last action (limited history).
## Requirements:

- Linux system
- X Window System
## Installation

To compile and run **zPen**, you need to have the **Xlib** development libraries installed on your system. Then, compile the source code with the following command:

Compile

`gcc zpen.c -o zpen -lX11 -lm`

## Usage

After compiling, run the program using:

`./zpen`

### Key Controls

- `p`: Select the freehand pen tool.
- `r`: Select the rectangle tool.
- `c`: Select the circle tool.
- `l`: Select the line tool.
- `a`: Select the arrow tool.
- `s`: Capture a screenshot.

### Mouse Controls

- **Left Click:** Set the starting point of the tool or add a point in freehand mode.
- **Drag:** Draw the shape.
- **Release:** Finish drawing the shape.

## Screenshot Saving

Screenshots are saved in the `~/Pictures` directory with the format `screenshot_YYYY-MM-DDTHH-MM-SS.png`.

## License

This project is licensed under the terms of the MIT license. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please submit a pull request or open an issue to discuss potential changes.

---

This `README.md` provides an overview of the project, features, installation instructions, usage guidelines, and information on how to contribute. Feel free to modify it to better suit your project's needs or to include additional details.
