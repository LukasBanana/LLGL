#!/bin/sh

SOURCE_DIR=$PWD
OUTPUT_DIR="build_ios"

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
        echo "usage: BuildIOS.sh [OUTPUT_DIR]"
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
cmake \
    -DLLGL_BUILD_RENDERER_OPENGLES3=OFF \
    -DLLGL_BUILD_RENDERER_METAL=ON \
    -DLLGL_BUILD_TESTS=ON \
    -DLLGL_BUILD_STATIC_LIB=ON \
    -DLLGL_BUILD_EXAMPLES=ON \
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" \
    -DCMAKE_SYSTEM_NAME=iOS \
    "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DCMAKE_IOS_INSTALL_COMBINED=ON \
    -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Xcode

cmake --build . -- -sdk iphonesimulator
