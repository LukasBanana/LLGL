#!/bin/sh

SOURCE_DIR="$(dirname $0)"
OUTPUT_DIR="$SOURCE_DIR/build_macos"
CLEAR_CACHE=0
ENABLE_NULL="OFF"
ENABLE_OPENGL="OFF"
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
BUILD_TYPE="Release"
PROJECT_ONLY=0
STATIC_LIB="OFF"
VERBOSE=0
GENERATOR=""

# When this .command script is launched from Finder, we have to change to the source directory explicitly
cd $SOURCE_DIR

print_help()
{
    echo "USAGE:"
    echo "  BuildMacOS.command OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -c, --clear-cache ......... Clear CMake cache and rebuild"
    echo "  -d, --debug ............... Configure Debug build (default is Release)"
    echo "  -g, --generator=G ......... Select CMake generator (or cmake's default)"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -p, --project-only ........ Build project solution only (no compilation)"
    echo "  -s, --static-lib .......... Build static lib (default is shared lib)"
    echo "  -v, --verbose ............. Print additional information"
    echo "  --null .................... Include Null renderer"
    echo "  --gl ...................... Include OpenGL renderer"
    echo "  --no-examples ............. Exclude example projects"
    echo "  --no-tests ................ Exclude test projects"
    echo "NOTES:"
    echo "  Default output directory is 'build_macos'"
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
    elif [[ "$ARG" == -g=* ]]; then
        GENERATOR="${ARG:3}"
    elif [[ "$ARG" == --generator=* ]]; then
        GENERATOR="${ARG:12}"
    elif [ "$ARG" = "-p" ] || [ "$ARG" = "--project-only" ]; then
        PROJECT_ONLY=1
    elif [ "$ARG" = "-s" ] || [ "$ARG" = "--static-lib" ]; then
        STATIC_LIB="ON"
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [ "$ARG" = "--null" ]; then
        ENABLE_NULL="ON"
    elif [ "$ARG" = "--gl" ]; then
        ENABLE_OPENGL="ON"
    elif [ "$ARG" = "--no-examples" ]; then
        ENABLE_EXAMPLES="OFF"
    elif [ "$ARG" = "--no-tests" ]; then
        ENABLE_TESTS="OFF"
    else
        OUTPUT_DIR="$ARG"
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
if [ $VERBOSE -ne 0 ]; then
    echo "GAUSSIAN_LIB_DIR=$GAUSSIAN_LIB_DIR"
    if [ $PROJECT_ONLY -eq 0 ]; then
        echo "BUILD_TYPE=$BUILD_TYPE"
    fi
fi

# Build into output directory (this syntax requires CMake 3.13+)
OPTIONS=(
    -DLLGL_BUILD_WRAPPER_C99=ON
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_OPENGL=$ENABLE_OPENGL
    -DLLGL_BUILD_RENDERER_METAL=ON
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS
    -DLLGL_BUILD_STATIC_LIB=$STATIC_LIB
    -DLLGL_BUILD_WRAPPER_C99=ON
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
    -B "$OUTPUT_DIR"
)

# Compiles the Metal shaders for all examples into the default.metallib file
compile_example_metal_shaders()
{
    EXAMPLE_BASE_DIR=$OUTPUT_DIR/build
    EXAMPLE_PROJECTS=($EXAMPLE_BASE_DIR/Example_*.app)

    for PROJECT in ${EXAMPLE_PROJECTS[@]}; do
        SHADER_FILES=()
        for FILE in $PROJECT/Contents/Resources/*.metal; do
            if [ -f $FILE ]; then
                SHADER_FILES+=($FILE)
            fi
        done
        if [ ! -z "$SHADER_FILES" ]; then
            if [ $VERBOSE -ne 0 ]; then
                echo "Compile Metal shaders: ${SHADER_FILES[@]}"
            fi
            xcrun -sdk macosx metal ${SHADER_FILES[@]} -o "$PROJECT/Contents/Resources/default.metallib"
        fi
    done
}

if [ $PROJECT_ONLY -eq 0 ]; then
    if [ "$GENERATOR" = "" ]; then
        cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} # CMake default generator
    else
        cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} -G "$GENERATOR"
    fi
    cmake --build "$OUTPUT_DIR"

    # Compile Metal shaders for all examples into the default.metallib.
    # This is done by Xcode automatically when built from within the IDE,
    # so we only do this when no project files are generated.
    compile_example_metal_shaders
else
    if [ "$GENERATOR" = "" ]; then
        GENERATOR="Xcode" # Default to Xcode for project solution
    fi
    cmake ${OPTIONS[@]} -G "$GENERATOR"
fi
