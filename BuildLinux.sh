#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_linux"
SKIP_VALIDATION=0
CLEAR_CACHE=0
ENABLE_NULL="OFF"
ENABLE_VULKAN="OFF"
ENABLE_D3D11="OFF"
ENABLE_D3D12="OFF" # Not supported in MSYS yet
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
BUILD_TYPE="Release"
PROJECT_ONLY=0
STATIC_LIB="OFF"
VERBOSE=0
GENERATOR="CodeBlocks - Unix Makefiles"

# Check whether we are on a Linux distribution or MSYS on Windows
[ "$#" -ge 2 ] && [ "$1" = "-msys" ] && PLATFORM_MSYS=1 || PLATFORM_MSYS=0

if [ $PLATFORM_MSYS -eq 1 ]; then
    OUTPUT_DIR="build_msys2"
fi

print_help()
{
    echo "USAGE:"
if [ $PLATFORM_MSYS -eq 1 ]; then
    echo "  BuildMsys2.sh OPTIONS* [OUTPUT_DIR]"
else
    echo "  BuildLinux.sh OPTIONS* [OUTPUT_DIR]"
fi
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -p, --project-only [=G] ... Build project with CMake generator (default is CodeBlocks)"
    echo "  -s, --static-lib .......... Build static lib (default is shared lib)"
    echo "  -S, --skip-validation ..... Skip check for missing packages (X11, OpenGL etc.)"
    echo "  --null .................... Include Null renderer"
    echo "  --vulkan .................. Include Vulkan renderer"
if [ $PLATFORM_MSYS -eq 1 ]; then
    echo "  --d3d11 ................... Include D3D11 renderer (MSYS only) "
fi
    echo "  --no-examples ............. Exclude example projects"
    echo "  --no-tests ................ Exclude test projects"
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
    elif [ "$ARG" = "-S" ] || [ "$ARG" = "--skip-validation" ]; then
        SKIP_VALIDATION=1
    elif [ "$ARG" = "--null" ]; then
        ENABLE_NULL="ON"
    elif [ "$ARG" = "--vulkan" ]; then
        ENABLE_VULKAN="ON"
    elif [ "$ARG" = "--d3d11" ]; then
        if [ $PLATFORM_MSYS -eq 1 ]; then
            ENABLE_D3D11="ON"
        else
            echo "Warning: D3D11 backend is only supported for MSYS"
        fi
    elif [ "$ARG" = "--no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    elif [ "$ARG" = "--no-tests" ]; then
        ENABLE_TESTS="OFF"
    elif [ ! "$ARG" = "-msys" ]; then
        OUTPUT_DIR="$ARG"
    fi
done

# Check packages are installed
if [ $SKIP_VALIDATION -eq 0 ]; then
    source scripts/ListMissingPackages.sh
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
fi

# Build into output directory (this syntax requried CMake 3.13+)
OPTIONS=(
    -DLLGL_BUILD_RENDERER_OPENGL=ON
    -DLLGL_GL_ENABLE_OPENGL2X=ON
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_VULKAN=$ENABLE_VULKAN
    -DLLGL_BUILD_RENDERER_DIRECT3D11=$ENABLE_D3D11
    -DLLGL_BUILD_RENDERER_DIRECT3D12=$ENABLE_D3D12
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS
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
