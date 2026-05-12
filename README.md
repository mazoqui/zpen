# zPen

**zPen** is a powerful fullscreen drawing overlay tool for Linux systems written in C using Xlib. It creates a transparent drawing layer over your desktop, allowing you to annotate, sketch, and highlight anything on your screen. Perfect for presentations, tutorials, code reviews, and quick visual communication.

[![Latest release](https://img.shields.io/github/v/release/mazoqui/zpen?label=latest&color=brightgreen)](https://github.com/mazoqui/zpen/releases/latest) ![Language](https://img.shields.io/badge/Language-C-blue) ![Platform](https://img.shields.io/badge/Platform-Linux-green) ![License](https://img.shields.io/badge/License-MIT-red)

## Table of Contents

- [Features](#features)
- [Usage](#usage)
  - [Drawing Tools](#drawing-tools)
  - [Color Controls](#color-controls)
  - [Undo/Redo](#undoredo)
  - [Screenshot & Clipboard](#screenshot--clipboard)
  - [Line Style](#line-style)
  - [Shift Modifiers](#shift-modifiers)
  - [Pen Thickness](#pen-thickness)
  - [Other](#other)
- [Installation](#installation)
  - [Debian / Ubuntu (recommended)](#debian--ubuntu-recommended)
  - [Build from source](#build-from-source)
  - [Global installation from source](#global-installation-from-source)
- [System Requirements (build from source)](#system-requirements-build-from-source)
  - [Install build dependencies](#install-build-dependencies)
- [Desktop Integration](#desktop-integration)
  - [Keyboard Shortcuts](#keyboard-shortcuts)
  - [Direction Detection](#direction-detection)
  - [Mouse Controls](#mouse-controls)
- [File Management](#file-management)
  - [Screenshot Saving](#screenshot-saving)
  - [File Formats](#file-formats)
- [Technical Details](#technical-details)
  - [Architecture](#architecture)
  - [Performance Features](#performance-features)
- [Troubleshooting](#troubleshooting)
  - [Common Issues](#common-issues)
  - [Wayland Compatibility](#wayland-compatibility)
- [Contributing](#contributing)
  - [Development Setup](#development-setup)
  - [Building the Debian package](#building-the-debian-package)
  - [Cutting a release](#cutting-a-release)
- [License](#license)
- [Acknowledgments](#acknowledgments)
- [Quick Reference](#quick-reference)

## Features

- **Drawing Tools:**
  - **Freehand Drawing:** Draw freehand on the screen with smoothing capabilities.
  - **Rectangles:** Draw rectangles with rounded corners (default) or straight corners. Press `r` again to toggle.
  - **Circles:** Draw circles by defining the center and radius.
  - **Lines:** Draw straight lines by specifying start and end points.
  - **Arrows:** Draw straight arrows or hold `Shift` to draw freehand arrows with an auto-directed arrowhead.
  - **Curly Braces:** Draw opening `{` and closing `}` braces with automatic direction detection.
  - **Square Brackets:** Draw opening `[` and closing `]` brackets with automatic direction detection.
  - **Blur:** Freehand blur brush to obscure sensitive content on screen.
  - **Text Input:** Add text annotations at any position on the screen.
- **Screenshot & Clipboard:**
  - Capture a screen region and save as `.png` file or copy to clipboard.
  - Copy a region to clipboard, and exit (`s`).
  - Copy a region to clipboard without exiting (`Ctrl+C`).
  - Paste images from clipboard directly onto the canvas (`Ctrl+V`).
  - OCR a region with `o` to copy the recognized text to the clipboard (requires `tesseract`).

- **Undo/Redo System:**
  - Full undo/redo functionality with up to 20 levels of history.
  - Standard keyboard shortcuts: `Ctrl+Z` for undo, `Shift+Ctrl+Z` for redo.
  - Backward compatibility: `u` key still works for undo.
- **Line Style:**
  - Toggle between solid and dashed lines with the `*` key.
  - Dashed pattern: 8 pixels on, 6 pixels off.

- **Translucent Fill:**
  - Hold `Shift` while drawing a rectangle or circle to fill the shape with a translucent version (20% opacity) of the active color.
  - Acts as a highlighter/marker that lets the background show through.

- **Freehand Arrow:**
  - Hold `Shift` while drawing with the Arrow tool to draw a freehand curve with an arrowhead at the end.
  - The arrowhead direction is automatically calculated from the average of the last path samples.

- **Pen Thickness Control:**
  - Increase pen thickness with `+` key (regular or numpad).
  - Decrease pen thickness with `-` key (regular or numpad).
  - Reset to default thickness with `0` key (regular or numpad).
  - Thickness range: 1 to 20 (default: 3).

- **Frozen Desktop:**
  - Captures a screenshot of the desktop at launch and uses it as a static background, preventing background UI changes from interfering with annotations.

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
  - `a`: Arrow (hold `Shift` for freehand arrow)
  - `r`: Rectangle (press again to toggle rounded/straight corners)
  - `c`: Circle
  - `{` or `}`: Curly Braces `{` `}` (direction auto-detected by drag direction)
  - `[` or `]`: Square Brackets `[` `]` (direction auto-detected by drag direction)
  - `b`: Blur brush (freehand blur to obscure areas)
  - `t`: Text input at cursor position

### Color Controls

- `Space`: Cycle to next color
- `←` / `→`: Navigate backward/forward through color palette

### Undo/Redo

- `Ctrl+Z`: Undo last action
- `Shift+Ctrl+Z`: Redo last undone action
- `u`: Undo (backward compatibility)

### Screenshot & Clipboard

- `s`: Capture screenshot region, copy to clipboard, and exit
- `f`: Capture screenshot region and save as PNG file
- `Ctrl+C`: Copy screenshot region to clipboard (without exiting)
- `Ctrl+V`: Paste clipboard image at current mouse cursor position
- `o`: Capture region, run OCR, and copy the recognized text to the clipboard (requires `tesseract`; the `o` key is a no-op if tesseract is not installed)

### Line Style

- `*`: Toggle between solid and dashed lines

### Shift Modifiers

- Hold `Shift` while drawing a rectangle or circle to fill with a translucent color
- Hold `Shift` while drawing an arrow to draw freehand with an arrowhead at the end

### Pen Thickness

- `+`: Increase pen thickness
- `-`: Decrease pen thickness
- `0`: Reset to default thickness
- Also works with numpad `+`, `-`, and `0`

### Other

- `n`: Add numbered step counter
- `ESC`: Exit zPen

## Installation

### Debian / Ubuntu (recommended)

Pre-built `.deb` packages are published on the [GitHub Releases](https://github.com/mazoqui/zpen/releases) page. `apt` resolves all runtime dependencies for you — there's nothing else to install.

**Install the latest release:**

```bash
cd /tmp && curl -LO https://github.com/mazoqui/zpen/releases/latest/download/zpen_latest_amd64.deb && sudo apt install ./zpen_latest_amd64.deb
```

The URL above is a permanent redirect — it always resolves to the most recent release. To pin to a specific version, browse the [Releases page](https://github.com/mazoqui/zpen/releases) and grab the versioned `zpen_X.Y.Z-1_amd64.deb` instead.

The package installs the binary at `/usr/bin/zpen`, ships a manpage (`man zpen`), and registers a desktop entry (zPen appears under **Graphics** in your app menu). `xclip` is pulled in automatically; `tesseract-ocr` is recommended and installed by default unless you opt out with `--no-install-recommends`.

To uninstall:

```bash
sudo apt remove zpen
```

### Build from source

For non-Debian distros, or if you want to hack on zpen, see the [build dependencies](#system-requirements-build-from-source) below for what to install first, then:

1. **Clone the repository:**

   ```bash
   git clone https://github.com/mazoqui/zpen.git
   cd zpen
   ```

2. **Compile:**

   ```bash
   make
   ```

   (or, manually: `gcc -o zpen src/zpen.c -lX11 -lXrender -lm`)

3. **Run:**
   ```bash
   ./dist/release_zpen
   ```

### Global installation from source

```bash
sudo make install PREFIX=/usr/local
# Now you can run zPen from anywhere
zpen
```

## System Requirements (build from source)

Only relevant if you're building from source or packaging for a different distro. Users installing the `.deb` get all dependencies via `apt`.

- **Operating System**: Linux with X Window System (X11)
- **Build tools**: `gcc`, `make`
- **Libraries**: X11 + XRender development headers
- **Runtime**: `xclip` for clipboard operations, a compositor like `picom` for transparency
- **Optional runtime**: `tesseract-ocr` for the `o` (OCR to clipboard) shortcut

### Install build dependencies

**Ubuntu/Debian:**

```bash
sudo apt install build-essential libx11-dev libxrender-dev xclip
# Optional, for the `o` (OCR) shortcut:
sudo apt install tesseract-ocr
```

**Fedora/CentOS/RHEL:**

```bash
sudo dnf install gcc make libX11-devel libXrender-devel xclip
# Optional, for the `o` (OCR) shortcut:
sudo dnf install tesseract
```

**Arch Linux:**

```bash
sudo pacman -S base-devel libx11 libxrender xclip
# Optional, for the `o` (OCR) shortcut:
sudo pacman -S tesseract tesseract-data-eng
```

## Desktop Integration

### Keyboard Shortcuts

You can set up global keyboard shortcuts to launch zPen quickly. The commands below assume `zpen` is on your `$PATH` — true for both the `.deb` install (`/usr/bin/zpen`) and a source install with the default `make install` (`/usr/local/bin/zpen`).

**XFCE:**

```bash
xfconf-query -c xfce4-keyboard-shortcuts -n -t 'string' -p '/commands/custom/<Super>p' -s zpen
```

**Cinnamon (Linux Mint):**

```bash
dconf write /org/cinnamon/desktop/keybindings/custom-list "['custom0']"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/command "'zpen'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/name "'zPen Drawing Tool'"
dconf write /org/cinnamon/desktop/keybindings/custom-keybindings/custom0/binding "['<Super>p']"
```

**GNOME:**

```bash
gsettings set org.gnome.settings-daemon.plugins.media-keys custom-keybindings "['/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/']"
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ name 'zPen Drawing Tool'
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ command 'zpen'
gsettings set org.gnome.settings-daemon.plugins.media-keys.custom-keybinding:/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/custom0/ binding '<Super>p'
```

### Direction Detection

- **Curly Braces (`{` or `}`)**: Drag left-to-right for `{`, drag right-to-left for `}`
- **Square Brackets (`[` or `]`)**: Drag left-to-right for `[`, drag right-to-left for `]`

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
cd zpen

# Build release + debug binaries
make

# Or build with extra warnings
gcc -Wall -Wextra -o /tmp/zpen src/zpen.c -lX11 -lXrender -lm

# Run under gdb
make debug
```

### Building the Debian package

```bash
# One-time: install packaging tools
sudo apt install debhelper lintian devscripts

# Build (outputs land in the parent directory)
dpkg-buildpackage -us -uc -b

# Optional: lint the result
lintian ../zpen_0.1.0-1_amd64.deb
```

The build is dh-based and uses the standard Debian hardening flags (`-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, `-Wl,-z,relro`, `-Wl,-z,now`).

### Cutting a release

Releases are published to [GitHub Releases](https://github.com/mazoqui/zpen/releases) with the `.deb` attached. The version source-of-truth is `debian/changelog`.

**Prerequisites** (one-time):

```bash
sudo apt install debhelper lintian devscripts
# Install the GitHub CLI: https://cli.github.com/
gh auth login
```

**For each release:**

```bash
# 1. Bump the version. Use dch to add a new entry; -v sets the version
#    (X.Y.Z-1 for the first packaging of upstream X.Y.Z), then -r finalises
#    the date and distribution.
dch -v 0.2.0-1 "Short summary."   # opens $EDITOR for the body
dch -r ""                         # finalises (sets date + distribution)

# 2. Commit the changelog bump.
git add debian/changelog
git commit -m "Release 0.2.0-1"
git push

# 3. Build, tag, push the tag, and publish the GitHub release.
./scripts/release.sh
```

`scripts/release.sh` reads the new version from `debian/changelog`, builds the `.deb` with `dpkg-buildpackage`, prints the proposed tag and release notes for review, and only then tags `vX.Y.Z`, pushes the tag, and creates the GitHub release with the `.deb` attached. Pass `--dry-run` to see what it would do without tagging or publishing, or `--yes` to skip the interactive confirmation (handy in CI).

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for full details.

## Acknowledgments

- Built with **Xlib** for X Window System integration
- Uses **stb_image_write.h** for PNG file generation
- Uses **stb_image.h** for PNG image loading (clipboard paste)
- Uses **tesseract** (optional) for OCR text recognition
- Inspired by annotation tools for presentations and tutorials

## Quick Reference

| Category       | Key             | Function                                                               |
| -------------- | --------------- | ---------------------------------------------------------------------- |
| **Drawing**    | `p`             | Pen (freehand)                                                         |
|                | `l`             | Line                                                                   |
|                | `a`             | Arrow                                                                  |
|                | `r`             | Rectangle (toggle rounded/straight)                                    |
|                | `c`             | Circle                                                                 |
|                | `{` or `}`      | Curly braces `{` `}`                                                   |
|                | `[` or `]`      | Square brackets `[` `]`                                                |
|                | `b`             | Blur brush                                                             |
|                | `t`             | Text input                                                             |
| **Colors**     | `Space`         | Next color (9 total)                                                   |
|                | `←` `→`         | Navigate colors                                                        |
|                |                 | _Colors: red, green, blue, yellow, orange, white, magenta, pink, gray_ |
| **Style**      | `*`             | Toggle solid/dashed lines                                              |
|                | `Shift`+draw    | Fill rectangle/circle with translucent color                           |
|                | `Shift`+draw    | Freehand arrow (with Arrow tool)                                       |
| **Thickness**  | `+`             | Increase pen thickness                                                 |
|                | `-`             | Decrease pen thickness                                                 |
|                | `0`             | Reset to default thickness                                             |
| **Actions**    | `Ctrl+Z`        | Undo                                                                   |
|                | `Shift+Ctrl+Z`  | Redo                                                                   |
|                | `u`             | Undo (legacy)                                                          |
|                | `n`             | Add number                                                             |
| **Screenshot** | `s`             | Copy to clipboard & exit                                               |
|                | `f`             | Save to file                                                           |
|                | `Ctrl+C`        | Copy to clipboard (no exit)                                            |
|                | `o`             | OCR region & copy text to clipboard (requires `tesseract`)             |
| **Clipboard**  | `Ctrl+V`        | Paste image at cursor                                                  |
| **System**     | `ESC`           | Exit                                                                   |
|                | `LShift+LAlt+p` | Swap focus between zPen and applications below                         |

---

**zPen** combines the simplicity of a drawing tool with powerful features for annotation, documentation, and screen capture. Perfect for presentations, tutorials, code reviews, and quick visual communication.
