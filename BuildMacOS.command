#!/bin/sh

SOURCE_DIR="$(dirname $0)"
BUILD_DIR="$SOURCE_DIR/build_macos"

# Ensure we are inside the repository folder
if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
    echo "error: file not found: CMakeLists.txt"
    exit 1
fi

# Make output build folder
if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
else
    if [ ! "$#" -eq 0 ]; then
        echo "error: too many arguemnts"
        echo "usage: BuildMacOS.command [BUILD_DIR]"
        exit 1
    fi
fi

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Checkout external depenencies
GAUSSIAN_LIB_DIR="$BUILD_DIR/GaussianLib/include"

if [ ! -d "$GAUSSIAN_LIB_DIR" ]; then
    (cd "$BUILD_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
fi

# Build into output directory
cmake \
    -DLLGL_BUILD_RENDERER_OPENGL=ON \
    -DLLGL_BUILD_RENDERER_METAL=ON \
    -DLLGL_BUILD_EXAMPLES=ON \
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" \
    -S "$SOURCE_DIR" \
    -B "$BUILD_DIR"
cmake --build "$BUILD_DIR"
