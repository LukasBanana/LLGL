#!/bin/sh

SOURCE_DIR="$(dirname $0)"
BUILD_DIR="$SOURCE_DIR/build_ios"

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
        echo "usage: BuildIOS.command [BUILD_DIR]"
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
    -S "$SOURCE_DIR" \
    -B "$BUILD_DIR" -G Xcode

cmake --build "$BUILD_DIR" -- -sdk iphonesimulator
