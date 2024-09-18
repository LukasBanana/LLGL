#!/bin/sh

SOURCE_DIR="$(dirname $0)"
OUTPUT_DIR="$SOURCE_DIR/build_macos"
CLEAR_CACHE=0
ENABLE_NULL="OFF"
ENABLE_METAL="ON"
ENABLE_OPENGL="OFF"
ENABLE_EXAMPLES="ON"
ENABLE_TESTS="ON"
ENABLE_WRAPPER_C99="ON"
BUILD_TYPE="Release"
PROJECT_ONLY=0
STATIC_LIB="OFF"
VERBOSE=0
GENERATOR=""

LEGACY=0
LEGACY_COMPILER=""
LEGACY_CXX=""
LEGACY_CC=""

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
    echo "  --no-wrapper .............. Exclude C99 wrapper"
    echo "  --legacy [=VER] ........... Legacy mode via MacPorts compiler (e.g. clang-11)"
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
    elif [ "$ARG" = "--no-wrapper" ]; then
        ENABLE_WRAPPER_C99="OFF"
    elif [ "$ARG" = "--legacy" ]; then
        LEGACY=1
    elif [[ "$ARG" == --legacy=* ]]; then
        LEGACY=1
        LEGACY_COMPILER="${ARG:9}"
    else
        OUTPUT_DIR="$ARG"
    fi
done

# Ensure we are inside the repository folder
if [ ! -f "$SOURCE_DIR/CMakeLists.txt" ]; then
    echo "Error: File not found: CMakeLists.txt"
    exit 1
fi

# Verify configuration for legacy mode
if [ $LEGACY -ne 0 ]; then
    # Legacy mode only supports GL2.x
    ENABLE_OPENGL="ON"
    ENABLE_METAL="OFF"

    PREFERRED_CLANG_VERSIONS=(15 14 13 12 11 16 17)
    KNOWN_BAD_VERSIONS=(16 17)

    if [ -z $LEGACY_COMPILER ]; then
        # Find available compiler (start with clang-15 as clang-16 and 17 are known to have linker problems)
        for VER in ${PREFERRED_CLANG_VERSIONS[@]}; do
            if command -v clang++-mp-$VER &> /dev/null; then
                LEGACY_CXX="clang++-mp-$VER"
                LEGACY_CC="clang-mp-$VER"
                break
            fi
        done
    else
        # Ensure MacPort of input compiler is installed (see macports.org)
        if [[ "$LEGACY_COMPILER" == clang-* ]]; then
            LEGACY_COMPILER_VER=${LEGACY_COMPILER:6}
            LEGACY_CXX="clang++-mp-$LEGACY_COMPILER_VER"
            LEGACY_CC="clang-mp-$LEGACY_COMPILER_VER"
            if ! command -v $LEGACY_CXX &> /dev/null; then
                echo "Error: MacPort of Clang $LEGACY_COMPILER_VER is not installed (see macports.org)!"
                echo "       Run the following command to install it:"
                echo "       $ sudo port install $LEGACY_COMPILER"
                exit 1
            fi
        else
            echo "Error: Unsupported compiler for legacy mode: ${LEGACY_COMPILER}"
            echo "       To select Clang 11 for instance, run the following command:"
            echo "       $ ./BuildMacOS.command --legacy=clang-11"
            exit 1
        fi
    fi

    # Check if no compiler has been found
    if [ -z $LEGACY_CXX ]; then
        echo "Error: Could not find suitable compiler for legacy mode!"
        exit 1
    fi

    if [ $VERBOSE -ne 0 ]; then
        echo "Found suitable compiler for legacy mode: clang-$VER"
    fi

    # Warn about known compiler versions to cause problems
    for VER in ${KNOWN_BAD_VERSIONS[@]}; do
        if [ "$LEGACY_COMPILER" = "clang-$VER" ]; then
            echo "Warning: Compiler $LEGACY_COMPILER is known to have problems building LLGL"
            break
        fi
    done
fi

# Make output build folder
if [ $CLEAR_CACHE = 1 ] && [ -d "$OUTPUT_DIR" ]; then
    rm -rf "$OUTPUT_DIR"
fi

if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir "$OUTPUT_DIR"
fi

# Checkout external dependencies
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
    -DLLGL_BUILD_RENDERER_NULL=$ENABLE_NULL
    -DLLGL_BUILD_RENDERER_OPENGL=$ENABLE_OPENGL
    -DLLGL_BUILD_RENDERER_METAL=$ENABLE_METAL
    -DLLGL_BUILD_EXAMPLES=$ENABLE_EXAMPLES
    -DLLGL_BUILD_TESTS=$ENABLE_TESTS
    -DLLGL_BUILD_STATIC_LIB=$STATIC_LIB
    -DLLGL_BUILD_WRAPPER_C99=$ENABLE_WRAPPER_C99
    -DGaussLib_INCLUDE_DIR:STRING="$GAUSSIAN_LIB_DIR"
    -S "$SOURCE_DIR"
    -B "$OUTPUT_DIR"
)

if [ $LEGACY -ne 0 ]; then
    # Compile in legacy mode with specific Clang compiler.
    # GNU 4 from Xcode 3 will not work, so choose Clang from MacPorts.
    # Also build for GL 2.x only and CoreVideo turned off.
    LEGACY_OPTIONS=(
        -E
        env
        CXX=$LEGACY_CXX
        CC=$LEGACY_CC
        cmake
        -DLLGL_MACOS_ENABLE_COREVIDEO=OFF
        -DLLGL_GL_ENABLE_OPENGL2X=ON
    )
    if [ $PROJECT_ONLY -eq 0 ]; then
        if [ -z "$GENERATOR" ]; then
            cmake ${LEGACY_OPTIONS[@]} -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} # CMake default generator
        else
            cmake ${LEGACY_OPTIONS[@]} -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} -G "$GENERATOR"
        fi
        cmake --build "$OUTPUT_DIR"
    else
        if [ -z "$GENERATOR" ]; then
            GENERATOR="Xcode" # Default to Xcode for project solution
        fi
        cmake ${LEGACY_OPTIONS[@]} ${OPTIONS[@]} -G "$GENERATOR"
    fi
else
    if [ $PROJECT_ONLY -eq 0 ]; then
        if [ -z "$GENERATOR" ]; then
            cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} # CMake default generator
        else
            cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ${OPTIONS[@]} -G "$GENERATOR"
        fi
        cmake --build "$OUTPUT_DIR"

        # Compile Metal shaders for all examples into the default.metallib.
        # This is done by Xcode automatically when built from within the IDE,
        # so we only do this when no project files are generated.
        COMPILE_METAL_SCRIPT=$SOURCE_DIR/scripts/CompileMetalToMetallib.sh
        if [ $VERBOSE -ne 0 ]; then
            $COMPILE_METAL_SCRIPT macosx "$OUTPUT_DIR/build" -v
        else
            $COMPILE_METAL_SCRIPT macosx "$OUTPUT_DIR/build"
        fi
    else
        if [ -z "$GENERATOR" ]; then
            GENERATOR="Xcode" # Default to Xcode for project solution
        fi
        cmake ${OPTIONS[@]} -G "$GENERATOR"
    fi
fi
