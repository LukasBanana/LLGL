#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_wasm"
CLEAR_CACHE=0
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
ENABLE_PTHREADS="OFF"
BUILD_TYPE="Release"
PROJECT_ONLY=0
VERBOSE=0
GENERATOR="CodeBlocks - Unix Makefiles"

# Check whether we are on a Linux distribution or MSYS on Windows
print_help()
{
    echo "USAGE:"
    echo "  BuildWasm.sh OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -p, --project-only [=G] ... Build project with CMake generator (default is CodeBlocks)"
    echo "  -v, --verbose ............. Print additional information"
    echo "  --no-examples ............. Exclude example projects"
    echo "  --no-tests ................ Exclude test projects"
    echo "  --pthreads ................ Enable pthreads (limits browser availability)"
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
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [ "$ARG" = "--null" ]; then
        ENABLE_NULL="ON"
    elif [ "$ARG" = "--vulkan" ]; then
        ENABLE_VULKAN="ON"
    elif [ "$ARG" = "--no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    elif [ "$ARG" = "--no-tests" ]; then
        ENABLE_TESTS="OFF"
    elif [ "$ARG" = "--pthreads" ]; then
        ENABLE_PTHREADS="ON"
    else
        OUTPUT_DIR="$ARG"
    fi
done

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

# Wrapper for 'realpath' when it's not available (like on macOS)
get_realpath()
{
    if command -v realpath &> /dev/null; then
        echo $(realpath "$1")
    else
        echo "$1"
    fi
}

# Checkout external depenencies
GAUSSIAN_LIB_DIR="GaussianLib/include"

if [ -f "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR/Gauss/Gauss.h" ]; then
    GAUSSIAN_LIB_DIR=$(get_realpath "$SOURCE_DIR/external/$GAUSSIAN_LIB_DIR")
else
    if [ ! -d "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR" ]; then
        (cd "$OUTPUT_DIR" && git clone https://github.com/LukasBanana/GaussianLib.git)
    fi
    GAUSSIAN_LIB_DIR=$(get_realpath "$OUTPUT_DIR/$GAUSSIAN_LIB_DIR")
fi

# Print additional information if in verbose mode
if [ $VERBOSE -ne 0 ]; then
    echo "GAUSSIAN_LIB_DIR=$GAUSSIAN_LIB_DIR"
    if [ $PROJECT_ONLY -eq 0 ]; then
        echo "BUILD_TYPE=$BUILD_TYPE"
    else
        echo "GENERATOR=$GENERATOR"
    fi
fi

# Find Emscripten SDK
if [ -z "$EMSDK" ]; then
    echo "Error: Missing EMSDK environment variable. Run 'source <PATH-TO-EMSDK>/emsdk_env.sh' to fix it."
    exit 1
fi

EMSCRIPTEN_CMAKE_TOOLCHAIN="$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"

if [ ! -f "$EMSCRIPTEN_CMAKE_TOOLCHAIN" ]; then
    echo "Error: Could not find file $EMSCRIPTEN_CMAKE_TOOLCHAIN"
    exit 1
fi

# Build into output directory (this syntax requried CMake 3.13+)
OPTIONS=(
    -DCMAKE_TOOLCHAIN_FILE="$EMSCRIPTEN_CMAKE_TOOLCHAIN"
    -DLLGL_BUILD_RENDERER_WEBGL=ON
    -DLLGL_GL_ENABLE_OPENGL2X=OFF
    -DLLGL_BUILD_RENDERER_NULL=OFF
    -DLLGL_BUILD_RENDERER_VULKAN=OFF
    -DLLGL_BUILD_RENDERER_DIRECT3D11=OFF
    -DLLGL_BUILD_RENDERER_DIRECT3D12=OFF
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS
    -DLLGL_BUILD_STATIC_LIB=ON
    -DLLGL_ENABLE_EMSCRIPTEN_PTHREADS=$ENABLE_PTHREADS
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
    -B "$OUTPUT_DIR"
)

if [ $PROJECT_ONLY -eq 0 ]; then
    cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]}
    cmake --build "$OUTPUT_DIR" -- -j 20
else
    cmake ${OPTIONS[@]} -G "$GENERATOR"
fi

# Copys the input file to the output and removes '\r' EOL characters from text files.
# Web page will run on Linux server and Git must not convert EOL for this output file.
# Otherwise, data offsets in the *.data.js script won't match with the *.data file after Git uploaded it.
copy_file_preserve_linux_eol()
{
    INPUT=$1
    OUTPUT=$2
    FILE_EXT=${INPUT##*.}
    if [[ "${FILE_EXT,,}" =~ ^(txt|vert|frag|obj)$ ]]; then
        if [ $VERBOSE -ne 0 ]; then
            echo "Copy asset (convert EOL): $(basename $INPUT)"
        fi
        tr -d "\r" < "$INPUT" > "$OUTPUT"
    else
        if [ $VERBOSE -ne 0 ]; then
            echo "Copy asset: $(basename $INPUT)"
        fi
        cp "$INPUT" "$OUTPUT"
    fi
}

# Generate HTML pages
if [ $PROJECT_ONLY -eq 0 ] && [ $ENABLE_EXAMPLES == "ON" ]; then
    scripts/GenerateHTML5Examples.sh                    \
        "${SOURCE_DIR}"                                 \
        "${OUTPUT_DIR}"                                 \
        $([ $BUILD_TYPE = "Debug" ] && echo "--debug")  \
        $([ $VERBOSE -ne 0 ] && echo "--verbose")
fi
