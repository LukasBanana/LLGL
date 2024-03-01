#!/bin/sh

SOURCE_DIR="$(dirname $0)"
OUTPUT_DIR="$SOURCE_DIR/build_ios"
CLEAR_CACHE=0
ENABLE_NULL="OFF"
ENABLE_GLES3="OFF"
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="OFF"
BUILD_TYPE="Release"
DEPLOYMENT_TARGET="11.0"
PROJECT_ONLY=0
STATIC_LIB="OFF"
VERBOSE=0

# When this .command script is launched from Finder, we have to change to the source directory explicitly
cd $SOURCE_DIR

print_help()
{
    echo "USAGE:"
    echo "  BuildIOS.command OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -p, --project-only ........ Build project solution only (no compilation)"
    echo "  -s, --static-lib .......... Build static lib (default is shared lib)"
    echo "  -t, --target T ............ Sets deployment target to T (default is 11.0)"
    echo "  -v, --verbose ............. Print additional information"
    echo "  --null .................... Include Null renderer"
    echo "  --gles .................... Include OpenGL ES 3 renderer"
    echo "  --no-examples ............. Exclude example projects"
    echo "NOTES:"
    echo "  Default output directory is 'build_ios'"
}

# Parse arguments
READ_TARGET=0
for ARG in "$@"; do
    if [ $READ_TARGET = 1 ]; then
        DEPLOYMENT_TARGET="$ARG"
        READ_TARGET=0
    else
        if [ "$ARG" = "-h" ] || [ "$ARG" = "--help" ]; then
            print_help
            exit 0
        elif [ "$ARG" = "-c" ] || [ "$ARG" = "--clear-cache" ]; then
            CLEAR_CACHE=1
        elif [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
            BUILD_TYPE="Debug"
        elif [ "$ARG" = "-p" ] || [ "$ARG" = "--project-only" ]; then
            PROJECT_ONLY=1
        elif [ "$ARG" = "-s" ] || [ "$ARG" = "--static-lib" ]; then
            STATIC_LIB="ON"
        elif [ "$ARG" = "-t" ] || [ "$ARG" = "--target" ]; then
            READ_TARGET=1
        elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
            VERBOSE=1
        elif [ "$ARG" = "--null" ]; then
            ENABLE_NULL="ON"
        elif [ "$ARG" = "--gles" ]; then
            ENABLE_GLES3="ON"
        elif [ "$ARG" = "--no-examples" ]; then
            ENABLE_EXAMPLES="OFF"
        else
            OUTPUT_DIR="$ARG"
        fi
    fi
done

# Ensure we are inside the repository folder
if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
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
    GAUSSIAN_LIB_DIR="$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR"
else
    if [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
        (cd "$OUTPUT_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
    fi
    GAUSSIAN_LIB_DIR="$OUTPUT_DIR/$GAUSSIAN_LIB_DIR"
fi

# Print additional information if in verbose mode
if [ $VERBOSE -eq 1 ]; then
    echo "DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET"
    echo "GAUSSIAN_LIB_DIR=$GAUSSIAN_LIB_DIR"
    if [ $PROJECT_ONLY -eq 0 ]; then
        echo "BUILD_TYPE=$BUILD_TYPE"
    fi
fi

# Build into output directory (this syntax requires CMake 3.13+)
OPTIONS=(
    -DCMAKE_SYSTEM_NAME=iOS
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$DEPLOYMENT_TARGET"
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO
    -DCMAKE_IOS_INSTALL_COMBINED=ON
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_OPENGLES3=$ENABLE_GLES3
    -DLLGL_BUILD_RENDERER_METAL=ON
    -DLLGL_BUILD_STATIC_LIB=$STATIC_LIB
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
    -B "$OUTPUT_DIR"
)

if [ $PROJECT_ONLY -eq 0 ]; then
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]}
    cmake --build "$OUTPUT_DIR" -- -sdk iphonesimulator
else
    cmake ${OPTIONS[@]} -G Xcode
fi
