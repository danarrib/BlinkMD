@echo off
setlocal enabledelayedexpansion

REM Check required tools
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: cmake not found. Install it from https://cmake.org/download/
    echo        and make sure it is added to PATH during installation.
    exit /b 1
)

REM Generate icon if ImageMagick is available
where magick >nul 2>&1
if %errorlevel% == 0 (
    echo Generating blinkmd-256.png and blinkmd.ico...
    magick -background none blinkmd-icon.svg -resize 256x256 blinkmd-256.png
    magick -background none blinkmd-icon.svg -define icon:auto-resize=256,48,32,16 blinkmd.ico
) else (
    echo Warning: ImageMagick not found. Icon files must exist for the build.
)

REM Locate Qt6 — honour QT_DIR if already set, otherwise scan C:\Qt
REM msvc2022_64 is checked last so it wins over msvc2019_64 when both exist
if not defined QT_DIR (
    for /d %%V in ("C:\Qt\6.*") do (
        for /d %%A in ("%%V\msvc2019_64" "%%V\msvc2022_64") do (
            if exist "%%A\lib\cmake\Qt6\Qt6Config.cmake" set "QT_DIR=%%A"
        )
    )
)

if not defined QT_DIR (
    echo Error: Qt6 not found. Either:
    echo   1. Install Qt 6 via the Qt online installer ^(https://www.qt.io/download-qt-installer^)
    echo      and select the "MSVC 2022 64-bit" component, or
    echo   2. Set the QT_DIR environment variable to your Qt MSVC directory, e.g.:
    echo      set QT_DIR=C:\Qt\6.9.0\msvc2022_64
    exit /b 1
)

echo Using Qt: %QT_DIR%

REM --- Locate MSVC environment ---
REM If cl.exe is already on PATH (e.g. running from a Developer Command Prompt), use it directly
set VCVARS=
set VCVARS_ARGS=
where cl.exe >nul 2>&1
if %errorlevel% == 0 (
    echo Using MSVC already on PATH.
    goto :found_msvc
)

REM Search for a vcvars script in each VS 2022 install location.
REM vcvarsall.bat (called with "amd64") is the canonical script; vcvars64.bat
REM is the x64-only equivalent and works equally well for our purposes.
REM Both live in VC\Auxiliary\Build\ when the C++ workload is installed.
set VCVARS_DIRS=
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -products * -version "[17,18)" -property installationPath 2^>nul`) do (
    set VCVARS_DIRS=!VCVARS_DIRS! "%%i\VC\Auxiliary\Build"
)
for %%E in (BuildTools Community Professional Enterprise) do (
    set VCVARS_DIRS=!VCVARS_DIRS! "%ProgramFiles%\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build"
)

for %%D in (!VCVARS_DIRS!) do (
    if "!VCVARS!"=="" (
        if exist "%%~D\vcvarsall.bat" (
            set VCVARS=%%~D\vcvarsall.bat
            set VCVARS_ARGS=amd64
        ) else if exist "%%~D\vcvars64.bat" (
            set VCVARS=%%~D\vcvars64.bat
            set VCVARS_ARGS=
        )
    )
)

if "!VCVARS!"=="" (
    echo.
    echo Error: Could not find vcvarsall.bat or vcvars64.bat for Visual Studio 2022.
    echo        VS 2022 may be installed but is missing the C++ workload.
    echo.
    echo Fix: Open the Visual Studio Installer, click Modify next to VS 2022,
    echo      and enable "Desktop development with C++".
    echo.
    echo Alternative: Run this script from a "Developer Command Prompt for VS 2022".
    exit /b 1
)

echo Initializing MSVC environment from: !VCVARS!
call "!VCVARS!" !VCVARS_ARGS!
if %errorlevel% neq 0 exit /b %errorlevel%

REM vcvarsall.bat may change the working directory — restore it to the script's location
cd /d "%~dp0"

:found_msvc

REM Use Ninja if available (faster), otherwise fall back to NMake
set CMAKE_GEN=NMake Makefiles
where ninja >nul 2>&1
if %errorlevel% == 0 set CMAKE_GEN=Ninja
echo Using generator: !CMAKE_GEN!

REM Clean previous build
if exist build-windows rmdir /s /q build-windows 2>nul
if exist build-windows (
    echo Retrying cleanup — build directory may be locked by antivirus...
    timeout /t 3 /nobreak >nul
    rmdir /s /q build-windows 2>nul
)
if exist build-windows (
    echo Error: Cannot delete build-windows. A file inside is locked.
    echo        Add this project folder to your antivirus exclusions and try again.
    echo        Common fix: Windows Security ^> Virus protection ^> Exclusions ^> Add folder
    exit /b 1
)

cmake -B build-windows -G "!CMAKE_GEN!" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%"
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build build-windows
if %errorlevel% neq 0 exit /b %errorlevel%

REM Deploy Qt dependencies so the exe is runnable directly from build-windows\
REM Qt bin must be on PATH so windeployqt can resolve Qt DLLs while it runs.
set PATH=%QT_DIR%\bin;%PATH%

set WINDEPLOYQT=
if exist "%QT_DIR%\bin\windeployqt6.exe" set WINDEPLOYQT=%QT_DIR%\bin\windeployqt6.exe
if exist "%QT_DIR%\bin\windeployqt.exe"  set WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe

if "!WINDEPLOYQT!"=="" (
    echo Warning: windeployqt not found in %QT_DIR%\bin — skipping deployment.
    echo          The exe may not run outside a Qt environment.
) else (
    echo Deploying Qt dependencies via !WINDEPLOYQT!...
    "!WINDEPLOYQT!" --no-translations --dir build-windows build-windows\BlinkMD.exe
    if %errorlevel% neq 0 exit /b %errorlevel%
)

echo.
echo Build complete: build-windows\BlinkMD.exe
