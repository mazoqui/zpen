# zpen

**zPen** is a powerful drawing tool for Linux systems written using Xlib. It allows you to create various shapes, lines, freehand drawings, and special characters on your screen. Additionally, you can take screenshots, navigate colors efficiently, and use full undo/redo functionality with standard keyboard shortcuts.
## Features

- **Drawing Tools:**
    
    - **Freehand Drawing:** Draw freehand on the screen with smoothing capabilities.
    - **Rectangles:** Draw rectangles by clicking and dragging.
    - **Circles:** Draw circles by defining the center and radius.
    - **Lines:** Draw straight lines by specifying start and end points.
    - **Arrows:** Draw arrows to point out specific areas on the screen.
    - **Curly Braces:** Draw opening `{` and closing `}` braces with automatic direction detection.
    - **Square Brackets:** Draw opening `[` and closing `]` brackets with automatic direction detection.
    - **Text Input:** Add text annotations at any position on the screen.
- **Screenshot Capture:**
    
    - Capture the current state of the screen as a screenshot and save it as a `.png` file.
- **Undo/Redo System:**
    
    - Full undo/redo functionality with up to 20 levels of history.
    - Standard keyboard shortcuts: `Ctrl+Z` for undo, `Shift+Ctrl+Z` for redo.
    - Backward compatibility: `u` key still works for undo.
- **Color Selection:**
    
    - Choose from a palette of 6 predefined colors (red, green, blue, yellow, orange, white).
    - Enhanced navigation: Use arrow keys (`←`/`→`) or spacebar to cycle colors.
- **Step Counter:**
    
    - Display a step counter for drawings made with the freehand tool.

## Usage

- Use the mouse to draw on the screen.
- Click and hold to start drawing freehand shapes.
- Release the mouse button to complete the shape.
### Drawing Tools
- Press the following keys to switch between drawing tools:
    - `p`: Pen (Freehand drawing)
    - `l`: Line
    - `a`: Arrow
    - `r`: Rectangle
    - `c`: Circle
    - `k`: Curly Braces `{` `}` (direction auto-detected by drag direction)
    - `b`: Square Brackets `[` `]` (direction auto-detected by drag direction)
    - `t`: Text input at cursor position

### Color Controls
- `Space`: Cycle to next color
- `←` / `→`: Navigate backward/forward through color palette

### Undo/Redo
- `Ctrl+Z`: Undo last action
- `Shift+Ctrl+Z`: Redo last undone action
- `u`: Undo (backward compatibility)

### Screenshot Tools
- `s`: Capture screenshot, copy to clipboard, and exit
- `f`: Capture screenshot and save as PNG file

### Other
- `n`: Add numbered step counter
- `ESC`: Exit zPen

## Requirements:
- Linux system
- X Window System

## Install dependencies

`sudo apt install libx11-dev xclip`

> **xclip** is useful for clipboard management and is pre-installed in most common linux distributions

## Installation

To compile and run **zPen**, you need to have the **Xlib** development libraries installed on your system. Then, compile the source code with the following command:

Compile

```
cd src
gcc zpen.c -o zpen -lX11 -lm
```

## Usage

After compiling, run the program using:

`./zpen`

### XFCE Users (like me)

You may want to add a shortcut 

`xfconf-query -c xfce4-keyboard-shortcuts -n -t 'string' -p '/commands/custom/<Super>p' -s $PWD/zpen`

### Debian - Linux Mint cinnamon users

```
dconf write /org/cinnamon/desktop/keybindings/custom-list "['custom0']"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/command "'$PWD/zpen'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/name "'zpen'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/binding "['<Super>p']"
```

### Direction Detection
- **Curly Braces (`k`)**: Drag left-to-right for `{`, drag right-to-left for `}`
- **Square Brackets (`b`)**: Drag left-to-right for `[`, drag right-to-left for `]`

### Mouse Controls

- **Left Click:** Set the starting point of the tool or add a point in freehand mode.
- **Drag:** Draw the shape.
- **Release:** Finish drawing the shape.

## Screenshot Saving

Screenshots are automatically saved in the `~/.zpen` directory with timestamp-based filenames in the format `imgYYYYMMDDHHMMSS.png`. The directory is created automatically if it doesn't exist.

## License

This project is licensed under the terms of the MIT license. See the LICENSE file for details.

## Contributing

Contributions are welcome! Please submit a pull request or open an issue to discuss potential changes.

## Quick Reference

| Category | Key | Function |
|----------|-----|----------|
| **Drawing** | `p` | Pen (freehand) |
| | `l` | Line |
| | `a` | Arrow |
| | `r` | Rectangle |
| | `c` | Circle |
| | `k` | Curly braces `{` `}` |
| | `b` | Square brackets `[` `]` |
| | `t` | Text input |
| **Colors** | `Space` | Next color |
| | `←` `→` | Navigate colors |
| **Actions** | `Ctrl+Z` | Undo |
| | `Shift+Ctrl+Z` | Redo |
| | `u` | Undo (legacy) |
| | `n` | Add number |
| **Screenshot** | `s` | Save & exit |
| | `f` | Save to file |
| **System** | `ESC` | Exit |

---

**zPen** combines the simplicity of a drawing tool with powerful features for annotation, documentation, and screen capture. Perfect for presentations, tutorials, code reviews, and quick visual communication.
