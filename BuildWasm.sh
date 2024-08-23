#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_wasm"
CLEAR_CACHE=0
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
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
    elif [ ! "$ARG" = "-msys" ]; then
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

# Find Emscripten SDK
if [ -z "$EMSDK" ]; then
    echo "Error: Missing EMSDK environment variable. Run 'source <PATH-TO-EMSDK>/emsdk_env.sh' to fix it."
    exit 1
fi

EMSCRIPTEN_CMAKE_TOOLCHAIN="$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
EMSCRIPTEN_FILE_PACKAGER="$EMSDK/upstream/emscripten/tools/file_packager"

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

# Generate HTML pages
generate_html5_page()
{
    CURRENT_PROJECT=$1

    echo "Generate HTML5 page: $CURRENT_PROJECT"

    # Get source folder
    ASSET_SOURCE_DIR="$SOURCE_DIR/examples/Media"
    PROJECT_SOURCE_DIR="$SOURCE_DIR/examples/Cpp"
    if [[ "$CURRENT_PROJECT" == *D ]]; then
        PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/${CURRENT_PROJECT:0:-1}"
    else
        PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/$CURRENT_PROJECT"
    fi

    # Get destination folder
    HTML5_ROOT="${OUTPUT_DIR}/html5/Example_$CURRENT_PROJECT"
    BIN_ROOT=${OUTPUT_DIR}/build

    # Create folder structure
    mkdir -p "$HTML5_ROOT"
    BASE_FILENAME="Example_$CURRENT_PROJECT"
    cp "$SOURCE_DIR/examples/Cpp/ExampleBase/Wasm/index.html" "$HTML5_ROOT/index.html"
    cp "$BIN_ROOT/$BASE_FILENAME.js" "$HTML5_ROOT/$BASE_FILENAME.js"
    cp "$BIN_ROOT/$BASE_FILENAME.wasm" "$HTML5_ROOT/$BASE_FILENAME.wasm"
    cp "$BIN_ROOT/$BASE_FILENAME.worker.js" "$HTML5_ROOT/$BASE_FILENAME.worker.js"

    # Replace meta data
    sed -i "s/LLGL_EXAMPLE_NAME/${CURRENT_PROJECT}/" "$HTML5_ROOT/index.html"
    sed -i "s/LLGL_EXAMPLE_PROJECT/Example_${CURRENT_PROJECT}/" "$HTML5_ROOT/index.html"
    
    # Find all required assets in Android.assets.txt file of respective project directory and copy them into app folder
    ASSET_DIR="$HTML5_ROOT/assets"
    mkdir -p "$ASSET_DIR"

    ASSET_LIST_FILE="$PROJECT_SOURCE_DIR/Android.assets.txt"
    if [ -f "$ASSET_LIST_FILE" ]; then
        # Read Android.asset.txt file line-by-line into array and make sure '\r' character is not present (on Win32 platform)
        readarray -t ASSET_FILTERS < <(tr -d '\r' < "$ASSET_LIST_FILE")
        ASSET_FILES=()
        for FILTER in ${ASSET_FILTERS[@]}; do
            for FILE in $ASSET_SOURCE_DIR/$FILTER; do
                ASSET_FILES+=( "$FILE" )
            done
        done

        # Copy all asset file into destination folder
        for FILE in ${ASSET_FILES[@]}; do
            if [ $VERBOSE -eq 1 ]; then
                echo "Copy asset: $(basename $FILE)"
            fi
            cp "$FILE" "$ASSET_DIR/$(basename $FILE)"
        done
    fi

    # Find all shaders and copy them into app folder
    for FILE in $PROJECT_SOURCE_DIR/*.vert \
                $PROJECT_SOURCE_DIR/*.frag; do
        if [ -f "$FILE" ]; then
            if [ $VERBOSE -eq 1 ]; then
                echo "Copy shader: $(basename $FILE)"
            fi
            cp "$FILE" "$ASSET_DIR/$(basename $FILE)"
        fi
    done

    # Package assets into .data.js file with Emscripten packager tool
    (cd "$HTML5_ROOT" && "$EMSCRIPTEN_FILE_PACKAGER" "$BASE_FILENAME.data" --preload assets "--js-output=$BASE_FILENAME.data.js" >/dev/null 2>&1)
}

if [ $PROJECT_ONLY -eq 0 ] && [ $ENABLE_EXAMPLES == "ON" ]; then

    BIN_FILE_BASE="${OUTPUT_DIR}/build/Example_"
    BIN_FILE_BASE_LEN=${#BIN_FILE_BASE}

    EXAMPLE_BIN_FILES_D=(${BIN_FILE_BASE}*D.wasm)
    if [ -z "$EXAMPLE_BIN_FILES_D" ]; then
        EXAMPLE_BIN_FILES=(${BIN_FILE_BASE}*.wasm)
    else
        EXAMPLE_BIN_FILES=(${EXAMPLE_BIN_FILES_D[@]})
    fi

    for BIN_FILE in ${EXAMPLE_BIN_FILES[@]}; do
        BIN_FILE_LEN=${#BIN_FILE}
        PROJECT_NAME=${BIN_FILE:BIN_FILE_BASE_LEN:BIN_FILE_LEN-BIN_FILE_BASE_LEN-5}
        generate_html5_page $PROJECT_NAME
    done

fi
