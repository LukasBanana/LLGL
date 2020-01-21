#!/bin/sh

SOURCE_DIR=$PWD
OUTPUT_DIR="build"

# Check packages are installed
REQUIRED_PKG=""

dpkg -s "git" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} git"
fi

dpkg -s "cmake" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} cmake"
fi

dpkg -s "libx11-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} libx11-dev"
fi

dpkg -s "libxxf86vm-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} libxxf86vm-dev"
fi

dpkg -s "libxrandr-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} libxrandr-dev"
fi

dpkg -s "mesa-common-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} mesa-common-dev"
fi

dpkg -s "libglu1-mesa-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} libglu1-mesa-dev"
fi

dpkg -s "freeglut3-dev" 2>&1 >/dev/null
if [ $? -ne 0 ]; then
    REQUIRED_PKG="${REQUIRED_PKG} freeglut3-dev"
fi

if [ ! -z "$REQUIRED_PKG" ]; then
    echo "missing packages! enter the following command to install then:"
    echo "sudo apt install${REQUIRED_PKG}"
    exit 0
fi

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
        echo "usage: BuildLinux.sh [OUTPUT_DIR]"
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
cmake -DLLGL_BUILD_RENDERER_OPENGL=ON -DLLGL_BUILD_RENDERER_VULKAN=OFF -DLLGL_BUILD_EXAMPLES=ON -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" -S "$SOURCE_DIR"
cmake --build .

