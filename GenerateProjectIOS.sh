#!/bin/sh

if [ "$#" -ne 2 ]; then
	echo "error: missing input arguments"
	echo "usage: GenerateProjectIOS.sh SOURCE_DIR BUILD_DIR"
	exit 1
fi

SOURCE_DIR=$1
BUILD_DIR=$2

# Build into output directory
cmake \
    -DLLGL_BUILD_RENDERER_OPENGLES3=OFF \
    -DLLGL_BUILD_RENDERER_METAL=ON \
    -DLLGL_BUILD_TESTS=ON \
    -DLLGL_BUILD_STATIC_LIB=ON \
    -DCMAKE_SYSTEM_NAME=iOS \
    "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DCMAKE_IOS_INSTALL_COMBINED=ON \
    -DLLGL_IOS_PLATFORM=ON \
    -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Xcode

