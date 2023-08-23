#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_linux"
SKIP_VALIDATION=0
CLEAR_CACHE=0
ENABLE_NULL="OFF"
ENABLE_VULKAN="OFF"
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
BUILD_TYPE="Release"

print_help()
{
    echo "USAGE:"
    echo "  BuildLinux.sh OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -h, --help .............. Print this help documentation and exit"
    echo "  -c, --clear-cache ....... Clear CMake cache and rebuild"
    echo "  -s, --skip-validation ... Skip check for missing packages (X11, OpenGL etc.)"
    echo "  -d, --debug ............. Configure Debug build (default is Release)"
    echo "  -null ................... Include Null renderer"
    echo "  -vulkan ................. Include Vulkan renderer"
    echo "  -no-examples ............ Exclude example projects"
    echo "  -no-tests ............... Exclude test projects"
    echo "NOTES:"
    echo "  Default output directory is 'build_linux'"
}

# Parse arguments
for ARG in "$@"; do
    if [ $ARG = "-h" ] || [ $ARG = "--help" ]; then
        print_help
        exit 0
    elif [ $ARG = "-c" ] || [ $ARG = "--clear-cache" ]; then
        CLEAR_CACHE=1
    elif [ $ARG = "-s" ] || [ $ARG = "--skip-validation" ]; then
        SKIP_VALIDATION=1
    elif [ $ARG = "-d" ] || [ $ARG = "--debug" ]; then
        BUILD_TYPE="Debug"
    elif [ $ARG = "-null" ]; then
        ENABLE_NULL="ON"
    elif [ $ARG = "-vulkan" ]; then
        ENABLE_VULKAN="ON"
    elif [ $ARG = "-no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    elif [ $ARG = "-no-tests" ]; then
        ENABLE_TESTS="OFF"
    else
        OUTPUT_DIR=$ARG
    fi
done

# Check packages are installed
if [ $SKIP_VALIDATION -eq 0 ]; then
    source scripts/ListMissingPackages.sh
fi

# Ensure we are inside the repository folder
if [ ! -f "CMakeLists.txt" ]; then
    echo "error: file not found: CMakeLists.txt"
    exit 1
fi

# Make output build folder
if [ $CLEAR_CACHE = 1 ] && [ -d "$OUTPUT_DIR" ]; then
    rm -rf "$OUTPUT_DIR"
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

# Checkout external depenencies
GAUSSIAN_LIB_DIR="GaussianLib/include"

if [ -f "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR/Gauss/Gauss.h" ]; then
    GAUSSIAN_LIB_DIR=$(realpath "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR")
elif [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
    pushd "$OUTPUT_DIR"
    git clone https://github.com/LukasBanana/GaussianLib.git
    popd
    GAUSSIAN_LIB_DIR=$(realpath "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR")
fi

# Build into output directory
cd "$OUTPUT_DIR"

cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DLLGL_BUILD_RENDERER_OPENGL=ON \
    -DLLGL_GL_ENABLE_OPENGL2X=ON \
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL \
    -DLLGL_BUILD_RENDERER_VULKAN=$ENABLE_VULKAN \
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES \
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS \
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" \
    -S "$SOURCE_DIR"

cmake --build .

