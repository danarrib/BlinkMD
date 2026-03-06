# BlinkMD

<img width="256" height="256" alt="Image" src="https://github.com/user-attachments/assets/461b9adf-4966-409c-bb7a-bf5f5c9a9ae0" />

A minimal, native Markdown viewer built with C++ and Qt 6. Open a file from the command line, drag and drop it onto the window, or use **File → Open** — BlinkMD renders it instantly and stays out of your way.

<img width="1663" height="1202" alt="image" src="https://github.com/user-attachments/assets/ff800102-df38-415e-bea2-b3af79df69b4" />

## Features

- Renders Markdown via Qt's built-in `QTextBrowser`
- Drag-and-drop support
- Command-line usage: `BlinkMD file.md`
- Remote images loaded asynchronously; graceful placeholder for unavailable images
- Links open in the default browser (with confirmation); local `.md` links navigate within the app
- Adapts to system dark/light mode — icon and rendering update live when the theme changes
- No Electron, no installer — extract the ZIP/DMG/AppImage and run
- Native look and feel on every platform

## Downloads

Pre-built binaries are attached to each [GitHub release](../../releases):

| Platform | File |
|----------|------|
| Windows (x64) | `BlinkMD-windows-x64.zip` — extract and run `BlinkMD.exe`, no installer needed |
| macOS (universal) | `BlinkMD-macos-universal.dmg` — runs on both Apple Silicon and Intel |
| Linux (x86_64) | `BlinkMD-linux-x86_64.AppImage` — single portable file |

## Building from source

### Linux

**Install dependencies:**

```bash
sudo apt-get install cmake qt6-base-dev libgl1-mesa-dev libxkbcommon-dev libxkbcommon-x11-dev librsvg2-bin
```

**Build:**

```bash
./build.sh
./build-linux/BlinkMD file.md
```

---

### macOS

**Install dependencies** ([Homebrew](https://brew.sh)):

```bash
brew install cmake qt librsvg
```

**Build:**

```bash
./build.sh
open ./build-mac/BlinkMD.app
```

---

### Windows

Four tools are required. Install them in order — each step has a detail that is easy to miss.

#### 1. CMake 3.20+

Download from [cmake.org/download](https://cmake.org/download/). During installation, select **"Add CMake to the system PATH for all users"** (or current user). Without this, the build script won't find `cmake`.

#### 2. Visual Studio 2022 with the C++ workload

Download [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (the free Community edition works) or the lighter [VS 2022 Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022).

During installation, select the **"Desktop development with C++"** workload. This is the step most people miss — without it there is no C++ compiler and no `vcvarsall.bat`, and the build will fail. Installing Visual Studio without this workload is not enough.

> **Qt 6.9+ requires MSVC 2022. MSVC 2019 is not supported.**

#### 3. Qt 6 — MSVC 2022 64-bit component

Download the [Qt Online Installer](https://www.qt.io/download-qt-installer) (requires a free Qt account). During installation, expand the Qt version you want and select the **"MSVC 2022 64-bit"** component. Other components (MinGW, 32-bit, etc.) will not work.

`build.bat` auto-detects Qt under `C:\Qt\`. If Qt is installed elsewhere, set `QT_DIR` before running:

```bat
set QT_DIR=C:\Qt\6.10.2\msvc2022_64
```

#### 4. ImageMagick

Download from [imagemagick.org](https://imagemagick.org/script/download.php#windows). During installation, check **"Add application directory to your system path"** so the `magick` command is available.

> **Note:** ImageMagick is used on Windows to generate the `.ico` file. On Linux and macOS, `rsvg-convert` (from `librsvg`) is used instead because it renders SVG filters and gradients correctly.

#### Build

```bat
build.bat
```

The script will:
1. Generate the app icon from `blinkmd-icon.svg`
2. Auto-detect VS 2022 and initialise the MSVC compiler environment
3. Configure and build with CMake + Ninja (or NMake as fallback)
4. Run `windeployqt` to copy all required Qt DLLs and plugins into `build-windows\`

After a successful build, `build-windows\BlinkMD.exe` is self-contained and can be run directly:

```bat
build-windows\BlinkMD.exe file.md
```

> **Tip:** If VS 2022 is already active in your terminal (e.g. you opened a **Developer Command Prompt for VS 2022**), the script detects `cl.exe` on the PATH and skips the VS detection step entirely.

> **Antivirus note:** Windows Defender and other antivirus tools can lock files inside the build directory while scanning freshly compiled objects, causing "Access denied" errors during cleanup. If this happens, add the project folder to your antivirus exclusions: **Windows Security → Virus & threat protection → Exclusions → Add a folder**.

---

## Project structure

```
BlinkMD/
├── .github/workflows/
│   ├── ci.yml              # Fast compile check on push/PR (Windows + Linux)
│   └── release.yml         # Full packaging on tag push (Windows + macOS + Linux)
├── main.cpp
├── MainWindow.h
├── MainWindow.cpp
├── MarkdownBrowser.h       # QTextBrowser subclass — async remote image loading
├── MarkdownBrowser.cpp
├── DebugWindow.h           # Debug log window (View → Debug Log)
├── DebugWindow.cpp
├── CMakeLists.txt
├── resources.qrc           # Embeds the app icon
├── resources.rc            # Windows resource file (icon)
├── blinkmd-icon.svg        # Source icon (platform icons generated at build time)
├── build.sh                # Local build script for Linux/macOS
└── build.bat               # Local build script for Windows
```

## CI / Release

- **Push to `master` or open a PR** → fast compile check on Windows and Linux via GitHub Actions
- **Push a tag** (e.g. `git tag v1.0.0 && git push --tags`) → full release build that produces and attaches a ZIP, DMG, and AppImage to the GitHub release

## License

See [LICENSE](LICENSE).
