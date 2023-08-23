#!/bin/sh

SOURCE_DIR="$(dirname $0)"
OUTPUT_DIR="$SOURCE_DIR/build_ios"
CLEAR_CACHE=0
ENABLE_EXAMPLES="OFF"
ENABLE_TESTS="ON"
#BUILD_TYPE="Release"
DEPLOYMENT_TARGET="11.0"
PROJECT_ONLY=0

print_help()
{
    echo "USAGE:"
    echo "  BuildIOS.command OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -h, --help .............. Print this help documentation and exit"
    echo "  -c, --clear-cache ....... Clear CMake cache and rebuild"
#    echo "  -d, --debug ............. Configure Debug build (default is Release)"
    echo "  -t, --target T .......... Sets deployment target to T (default is 11.0)"
    echo "  -p, --project-only ...... Build project solution only (no compilation)"
#    echo "  -no-examples ............ Exclude example projects"
    echo "  -no-tests ............... Exclude test projects"
    echo "NOTES:"
    echo "  Default output directory is 'build_ios'"
}

# Parse arguments
READ_TARGET=0
for ARG in "$@"; do
    if [ $READ_TARGET = 1 ]; then
        DEPLOYMENT_TARGET="$ARG"
        echo "Deployment target set to $DEPLOYMENT_TARGET"
        READ_TARGET=0
    else
        if [ $ARG = "-h" ] || [ $ARG = "--help" ]; then
            print_help
            exit 0
        elif [ $ARG = "-c" ] || [ $ARG = "--clear-cache" ]; then
            CLEAR_CACHE=1
#        elif [ $ARG = "-d" ] || [ $ARG = "--debug" ]; then
#            BUILD_TYPE="Debug"
        elif [ $ARG = "-t" ] || [ $ARG = "--target" ]; then
            READ_TARGET=1
        elif [ $ARG = "-p" ] || [ $ARG = "--project-only" ]; then
            PROJECT_ONLY=1
#        elif [ $ARG = "-no-examples" ]; then
#            ENABLE_EXAMPLES="OFF"
        elif [ $ARG = "-no-tests" ]; then
            ENABLE_TESTS="OFF"
        else
            OUTPUT_DIR=$ARG
        fi
    fi
done

# Ensure we are inside the repository folder
if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
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
    GAUSSIAN_LIB_DIR="$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR"
elif [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
    (cd "$OUTPUT_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
    GAUSSIAN_LIB_DIR="$OUTPUT_DIR/$GAUSSIAN_LIB_DIR"
fi

# Build into output directory
cmake \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$DEPLOYMENT_TARGET" \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DCMAKE_IOS_INSTALL_COMBINED=ON \
    -DLLGL_BUILD_RENDERER_NULL=OFF \
    -DLLGL_BUILD_RENDERER_OPENGLES3=OFF \
    -DLLGL_BUILD_RENDERER_METAL=ON \
    -DLLGL_BUILD_STATIC_LIB=ON \
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES \
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS \
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR" \
    -S "$SOURCE_DIR" \
    -B "$OUTPUT_DIR" \
    -G Xcode

if [ $PROJECT_ONLY -eq 0 ]; then
    cmake --build "$OUTPUT_DIR" -- -sdk iphonesimulator
fi
