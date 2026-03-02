#!/usr/bin/env bash
set -e

# Clean previous build
rm -rf build-linux

# Generate icon — rsvg-convert is preferred (renders SVG filters correctly)
if command -v rsvg-convert &>/dev/null; then
    echo "Generating blinkmd-256.png..."
    rsvg-convert -w 256 -h 256 blinkmd-icon.svg -o blinkmd-256.png
elif command -v magick &>/dev/null; then
    echo "Generating blinkmd-256.png (ImageMagick fallback)..."
    magick -background none blinkmd-icon.svg -resize 256x256 blinkmd-256.png
elif command -v convert &>/dev/null; then
    echo "Generating blinkmd-256.png (ImageMagick fallback)..."
    convert -background none blinkmd-icon.svg -resize 256x256 blinkmd-256.png
else
    echo "Warning: rsvg-convert and ImageMagick not found. blinkmd-256.png must exist for the build." >&2
fi

cmake -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -- -j"$(nproc)"

echo ""
echo "Build complete: build-linux/BlinkMD"
