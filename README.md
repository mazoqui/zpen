# zPen

**zPen** is a powerful fullscreen drawing overlay tool for Linux systems written in C using Xlib. It creates a transparent drawing layer over your desktop, allowing you to annotate, sketch, and highlight anything on your screen. Perfect for presentations, tutorials, code reviews, and quick visual communication.

![zPen Demo](https://img.shields.io/badge/Language-C-blue) ![Platform](https://img.shields.io/badge/Platform-Linux-green) ![License](https://img.shields.io/badge/License-MIT-red)
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
    
    - Choose from a palette of 9 predefined colors: red, green, blue, yellow, orange, white, magenta, pink, and gray.
    - Visual color palette displayed at bottom-right corner showing all available colors.
    - Selected color is highlighted with a filled circle and white border.
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

## System Requirements

- **Operating System**: Linux with X Window System (X11)
- **Dependencies**: 
  - `libx11-dev` - X11 development libraries
  - `xclip` - Clipboard management (usually pre-installed)
  - `gcc` - C compiler
  - `make` (optional, for build automation)
  - compositer like `picom` that supports transprency

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install libx11-dev xclip build-essential
```

**Fedora/CentOS/RHEL:**
```bash
sudo dnf install libX11-devel xclip gcc
# or for older versions:
sudo yum install libX11-devel xclip gcc
```

**Arch Linux:**
```bash
sudo pacman -S libx11 xclip gcc
```

## Installation

### Quick Start

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd zpen
   ```

2. **Compile:**
   ```bash
   cd src
   gcc zpen.c -o zpen -lX11 -lm
   ```

3. **Run:**
   ```bash
   ./zpen
   ```

### Global Installation (Optional)

To install zPen system-wide:

```bash
# Compile
cd src
gcc zpen.c -o zpen -lX11 -lm

# Install to /usr/local/bin
sudo cp zpen /usr/local/bin/

# Now you can run zPen from anywhere
zpen
```

## Desktop Integration

### Keyboard Shortcuts

You can set up global keyboard shortcuts to launch zPen quickly:

**XFCE:**
```bash
xfconf-query -c xfce4-keyboard-shortcuts -n -t 'string' -p '/commands/custom/<Super>p' -s /usr/local/bin/zpen
```

**Cinnamon (Linux Mint):**
```bash
dconf write /org/cinnamon/desktop/keybindings/custom-list "['custom0']"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/command "'/usr/local/bin/zpen'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/name "'zPen Drawing Tool'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/binding "['<Super>p']"
```

**GNOME:**
```bash
gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings "['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/']"
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ name 'zPen Drawing Tool'
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ command '/usr/local/bin/zpen'
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ binding '<Super>p'
```

### Direction Detection
- **Curly Braces (`k`)**: Drag left-to-right for `{`, drag right-to-left for `}`
- **Square Brackets (`b`)**: Drag left-to-right for `[`, drag right-to-left for `]`

### Mouse Controls

- **Left Click:** Set the starting point of the tool or add a point in freehand mode.
- **Drag:** Draw the shape.
- **Release:** Finish drawing the shape.

## File Management

### Screenshot Saving

Screenshots are automatically saved in the `~/.zpen` directory with timestamp-based filenames in the format `imgYYYYMMDDHHMMSS.png`. The directory is created automatically if it doesn't exist.

**Example files:**
- `~/.zpen/img20241201143052.png` - Screenshot taken on Dec 1, 2024 at 14:30:52

### File Formats

- **Screenshots**: PNG format for best quality and transparency support
- **Clipboard**: Images are copied in PNG format for clipboard compatibility

## Technical Details

### Architecture
- **Single-file C application** using Xlib for X11 integration
- **Fullscreen transparent overlay** that doesn't interfere with desktop interaction
- **Memory-efficient** with minimal system resource usage
- **Real-time drawing** with XOR graphics for smooth preview

### Performance Features
- **Path smoothing** for freehand drawing with configurable smoothing levels
- **Efficient undo system** using pixmap snapshots (up to 20 levels)
- **Minimal latency** for responsive drawing experience

## Troubleshooting

### Common Issues

**"Cannot open display" error:**
- Ensure you're running on a system with X11 (not Wayland-only)
- Check if `$DISPLAY` environment variable is set: `echo $DISPLAY`

**Missing libraries error:**
- Install X11 development packages: `sudo apt install libx11-dev`

**Permission issues with screenshots:**
- Ensure `~/.zpen` directory has write permissions
- Check that `xclip` is installed for clipboard functionality

### Wayland Compatibility

zPen currently requires X11 and may not work on pure Wayland systems. For Wayland users:
- Use XWayland compatibility layer
- Consider running in X11 session for full functionality

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork the repository** and create a feature branch
2. **Test your changes** thoroughly on different Linux distributions
3. **Follow C coding standards** and maintain code documentation
4. **Submit a pull request** with clear description of changes

### Development Setup

```bash
# Clone and setup development environment
git clone <repository-url>
cd zpen/src

# Compile with debug flags
gcc -g -Wall -Wextra zpen.c -o zpen -lX11 -lm

# Run with debugging
gdb ./zpen
```

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details.

## Acknowledgments

- Built with **Xlib** for X Window System integration
- Uses **stb_image_write.h** for PNG file generation
- Inspired by annotation tools for presentations and tutorials

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
| **Colors** | `Space` | Next color (9 total) |
| | `←` `→` | Navigate colors |
| | | *Colors: red, green, blue, yellow, orange, white, magenta, pink, gray* |
| **Actions** | `Ctrl+Z` | Undo |
| | `Shift+Ctrl+Z` | Redo |
| | `u` | Undo (legacy) |
| | `n` | Add number |
| **Screenshot** | `s` | Save & exit |
| | `f` | Save to file |
| **System** | `ESC` | Exit |

---

**zPen** combines the simplicity of a drawing tool with powerful features for annotation, documentation, and screen capture. Perfect for presentations, tutorials, code reviews, and quick visual communication.
