#!/bin/sh

if [ "$#" -gt 2 ]; then
    echo "error: invalid number of arguments"
    echo "usage: GenerateProjectAndroid.sh [BUILD_DIR [SOURCE_DIR]]"
#   echo "usage: GenerateProjectAndroid.sh [-abi=ARCH_ABI] [-api=API_LEVEL] [BUILD_DIR [SOURCE_DIR]]"
#   echo "\tARCH_ABI: aarch64, arm, i686, x86_64; default=x86_64"
#   echo "\tAPI_LEVEL: 21 or higher; default=21"
    exit 1
fi

# Check if environment variable "ANDROID_NDK_HOME" is set
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "error: environemnt variable 'ANDROID_NDK_HOME' not set"
    exit 1
fi

# Store intermediate variables
#ANDROID_ABI=armeabi-v7a
ANDROID_ABI=x86_64
ANDROID_API_LEVEL=21
SOURCE_DIR="."
BUILD_DIR="build"
ANDROID_CMAKE_TOOLCHAIN="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"

if [ "$#" -ge 1 ]; then
    BUILD_DIR=$1
fi

if [ "$#" -ge 2 ]; then
    SOURCE_DIR=$2
fi

# Validate input arguments
if [ ! -f "$ANDROID_CMAKE_TOOLCHAIN" ]; then
    echo "error: CMake toolchain not found: ${ANDROID_CMAKE_TOOLCHAIN}"
    exit 1
fi

if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
    echo "error: CMakeLists.txt not found: $SOURCE_DIR/CMakeLists.txt"
    exit 1
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Build into output directory (for CMake 10.12 or earlier)
RELATIVE_SOURCE_DIR=$(realpath --relative-to="$BUILD_DIR" "$SOURCE_DIR")

(cd "$BUILD_DIR";

cmake "$RELATIVE_SOURCE_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_CMAKE_TOOLCHAIN" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_PLATFORM=$ANDROID_API_LEVEL \
    -DANDROID_STL=c++_shared \
    -DANDROID_CPP_FEATURES="rtti exceptions" \
    -DLLGL_BUILD_RENDERER_OPENGLES3=ON \
    -DLLGL_BUILD_TESTS=ON \
    -DLLGL_BUILD_EXAMPLES=ON \
    -DGaussLib_INCLUDE_DIR="/home/lh/Development/Projects/GaussianLib/repo/include" \
    -DLLGL_BUILD_STATIC_LIB=OFF \
    -DLLGL_ANDROID_PLATFORM=ON \
    -G "CodeBlocks - Unix Makefiles"

)

