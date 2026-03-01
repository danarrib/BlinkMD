#!/usr/bin/env bash
set -e

# Clean previous build
rm -rf build-linux

# Generate icon if ImageMagick is available
if command -v magick &>/dev/null; then
    echo "Generating blinkmd-256.png..."
    magick convert -background none blinkmd-icon.svg -resize 256x256 blinkmd-256.png
elif command -v convert &>/dev/null; then
    echo "Generating blinkmd-256.png..."
    convert -background none blinkmd-icon.svg -resize 256x256 blinkmd-256.png
else
    echo "Warning: ImageMagick not found. blinkmd-256.png must exist for the build." >&2
fi

cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -- -j"$(nproc)"

echo ""
echo "Build complete: build-linux/BlinkMD"
