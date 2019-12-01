#!/bin/sh

if [ "$#" -ne 1 ]; then
	echo "error: missing input arguments"
	echo "usage: GenerateProjectIOS.sh ANDROID_NDK_DIR"
	exit 1
fi

ANDROID_NDK_DIR=$1
#SOURCE_DIR="."
BUILD_DIR="build"

if [ ! -d $BUILD_DIR ]; then
	mkdir $BUILD_DIR
fi

(cd $BUILD_DIR;

# Build into output directory
cmake .. \
	-DLLGL_BUILD_RENDERER_OPENGLES3=ON \
	-DLLGL_BUILD_TESTS=ON \
	-DLLGL_BUILD_STATIC_LIB=ON \
	-DANDROID_ABI=x86_64 \
	-DANDROID_NATIVE_API_LEVEL=21 \
	-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_DIR/build/cmake/android.toolchain.cmake \
	-DANDROID_PLATFORM=ON \
	-G "CodeBlocks - Unix Makefiles"

)

