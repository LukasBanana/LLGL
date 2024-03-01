#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_android"
SKIP_VALIDATION=0
CLEAR_CACHE=0
ENABLE_VULKAN="OFF"
ENABLE_EXAMPLES="ON"
BUILD_TYPE="Release"
PROJECT_ONLY=0
STATIC_LIB="OFF"
VERBOSE=0
GENERATOR="CodeBlocks - Unix Makefiles"
#ANDROID_ABI=armeabi-v7a
ANDROID_ABI=x86_64
ANDROID_API_LEVEL=21

print_help()
{
    echo "USAGE:"
    echo "  BuildAndroid.sh OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -p, --project-only [=G] ... Build project with CMake generator (default is CodeBlocks)"
    echo "  -s, --static-lib .......... Build static lib (default is shared lib)"
    echo "  -v, --verbose ............. Print additional information"
    echo "  --abi=ABI ................. Set Android ABI (default is x86_64)"
    echo "  --api-level=VERSION ....... Set Android API level (default is 21)"
    echo "  --vulkan .................. Include Vulkan renderer"
    echo "  --no-examples ............. Exclude example projects"
    echo "NOTES:"
    echo "  Default output directory is '$OUTPUT_DIR'"
}

# Parse arguments
for ARG in "$@"; do
    if [ "$ARG" = "-h" ] || [ "$ARG" = "--help" ]; then
        print_help
        exit 0
    elif [ "$ARG" = "-c" ] || [ "$ARG" = "--clear-cache" ]; then
        CLEAR_CACHE=1
    elif [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
        BUILD_TYPE="Debug"
    elif [ "$ARG" = "-p" ] || [ "$ARG" = "--project-only" ]; then
        PROJECT_ONLY=1
    elif [[ "$ARG" == -p=* ]]; then
        PROJECT_ONLY=1
        GENERATOR="${ARG:3}"
    elif [[ "$ARG" == --project-only=* ]]; then
        PROJECT_ONLY=1
        GENERATOR="${ARG:15}"
    elif [ "$ARG" = "-s" ] || [ "$ARG" = "--static-lib" ]; then
        STATIC_LIB="ON"
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [[ "$ARG" == --abi=* ]]; then
        ANDROID_ABI="${ARG:6}"
    elif [ "$ARG" = "--vulkan" ]; then
        ENABLE_VULKAN="ON"
    elif [ "$ARG" = "--no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    fi
done

# Find Android NDK installation
NDK_ROOT=""
if [ -z "$ANDROID_NDK_HOME" ]; then
    if [ -z "$ANDROID_NDK_ROOT" ]; then
        echo "Error: Neither environment variable 'ANDROID_NDK_HOME' nor 'ANDROID_NDK_ROOT' are set"
        exit 1
    else
        NDK_ROOT="$ANDROID_NDK_ROOT"
    fi
else
    NDK_ROOT="$ANDROID_NDK_HOME"
fi

ANDROID_CMAKE_TOOLCHAIN="${NDK_ROOT}/build/cmake/android.toolchain.cmake"

if [ ! -f "$ANDROID_CMAKE_TOOLCHAIN" ]; then
    echo "Error: CMake toolchain not found: ${ANDROID_CMAKE_TOOLCHAIN}"
    exit 1
fi

# Ensure we are inside the repository folder
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: File not found: CMakeLists.txt"
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
else
    if [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
        (cd "$OUTPUT_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
    fi
    GAUSSIAN_LIB_DIR=$(realpath "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR")
fi

# Print additional information if in verbose mode
if [ $VERBOSE -eq 1 ]; then
    echo "GAUSSIAN_LIB_DIR=$GAUSSIAN_LIB_DIR"
    if [ $PROJECT_ONLY -eq 0 ]; then
        echo "BUILD_TYPE=$BUILD_TYPE"
    else
        echo "GENERATOR=$GENERATOR"
    fi
    echo "ANDROID_ABI=$ANDROID_ABI"
    echo "ANDROID_API_LEVEL=$ANDROID_API_LEVEL"
fi

# Build into output directory (this syntax requires CMake 3.13+)
OPTIONS=(
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_CMAKE_TOOLCHAIN" \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_PLATFORM=$ANDROID_API_LEVEL \
    -DANDROID_STL=c++_shared \
    -DANDROID_CPP_FEATURES="rtti exceptions" \
    -DLLGL_BUILD_RENDERER_OPENGLES3=ON
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_VULKAN=$ENABLE_VULKAN
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=OFF
    -DLLGL_BUILD_STATIC_LIB=$STATIC_LIB
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
    -B "$OUTPUT_DIR"
)

if [ $PROJECT_ONLY -eq 0 ]; then
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]}
    cmake --build "$OUTPUT_DIR"
else
    cmake ${OPTIONS[@]} -G "$GENERATOR"
fi
