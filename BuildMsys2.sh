#!/bin/sh

SOURCE_DIR=$PWD
OUTPUT_DIR="build_msys2"

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
        echo "usage: BuildMsys2.sh [OUTPUT_DIR]"
        exit 1
    fi
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

cd "$OUTPUT_DIR"

# Checkout external depenencies
GAUSSIAN_LIB_DIR="GaussianLib/include"

if [ ! -d "$GAUSSIAN_LIB_DIR" ]; then
    git clone https://github.com/LukasBanana/GaussianLib.git
fi

# Build into output directory
cmake -DLLGL_BUILD_RENDERER_OPENGL=ON -DLLGL_BUILD_RENDERER_VULKAN=OFF -DLLGL_BUILD_RENDERER_DIRECT3D11=OFF -DLLGL_BUILD_RENDERER_DIRECT3D12=OFF -DLLGL_BUILD_EXAMPLES=ON -DGaussLib_INCLUDE_DIR:STRING="$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" -DCMAKE_BUILD_TYPE=Release -S "$SOURCE_DIR"
cmake --build .

