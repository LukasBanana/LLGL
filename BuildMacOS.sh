#!/bin/sh

SOURCE_DIR=$PWD
OUTPUT_DIR="build"

# Ensure we are inside the repository folder
if [ ! -f "CMakeLists.txt" ]; then
    echo "error: file not found: CMakeLists.txt"
    exit 1
fi

# Make output build folder
if [ "$#" -eq 1 ]; then
    OUTPUT_DIR=$1
else
    if [ ! "$#" -eq 0 ]; then
        echo "error: too many arguemnts"
        echo "usage: BuildMacOS.sh [OUTPUT_DIR]"
        exit 1
    fi
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

cd "$OUTPUT_DIR"

# Checkout external depenencies
GAUSSIAN_LIB_DIR="$OUTPUT_DIR/GaussianLib/include"

if [ ! -d "$GAUSSIAN_LIB_DIR" ]; then
    git clone https://github.com/LukasBanana/GaussianLib.git
fi

# Build into output directory
cmake -DLLGL_BUILD_RENDERER_OPENGL=ON -DLLGL_BUILD_RENDERER_METAL=ON -DLLGL_BUILD_EXAMPLES=ON -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" -S "$SOURCE_DIR"
cmake --build .
