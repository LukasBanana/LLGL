#!/bin/sh

DEFAULT_BUILD_DIRS=("build_macos/build" "build_macos/build/Debug" "bin/macOS-x64/build")

SOURCE_DIR="$(dirname $0)"
BUILD_DIR="$SOURCE_DIR/${DEFAULT_BUILD_DIRS[0]}"

# When this .command script is launched from Finder, we have to change to the source directory explicitly
cd $SOURCE_DIR

if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
else
    for DIR in "${DEFAULT_BUILD_DIRS[@]}"; do
        if [ -d "$DIR" ]; then
            if [ -f "$DIR/libLLGL.dylib" ] || [ -f "$DIR/libLLGLD.dylib" ] || [ -f "$DIR/libLLGL.a" ] || [ -f "$DIR/libLLGLD.a" ]; then
                BUILD_DIR="$DIR"
                break
            fi
        fi
    done
fi

# Validate build folder: Check for libLLGL.dylib/.a or libLLGLD.dylib/.a files
if [ -d "$BUILD_DIR" ]; then
    if [ -f "$BUILD_DIR/libLLGL.dylib" ] || [ -f "$BUILD_DIR/libLLGLD.dylib" ] || [ -f "$BUILD_DIR/libLLGL.a" ] || [ -f "$BUILD_DIR/libLLGLD.a" ]; then
        echo "Run examples from build directory: $BUILD_DIR"
    else
        echo "Error: Missing LLGL base lib (libLLGL.dylib/.a or libLLGLD.dylib/.a) in build folder: $BUILD_DIR"
        exit 1
    fi
else
    echo "Error: Build folder not found: $BUILD_DIR"
    exit 1
fi

list_examples()
{
    EXCLUDED=() # List any examples that are specifically excluded on Mac
    EXAMPLE_DIRS=($(ls examples/Cpp))
    for DIR in "${EXAMPLE_DIRS[@]}"; do
        if ! echo "${EXCLUDED[@]}}" | grep -qw "$DIR"; then
            # Include example if its source and binary files exist
            if [ -f "examples/Cpp/$DIR/Example.cpp" ] && [ -f "$BUILD_DIR/Example_${DIR}.app/Contents/MacOS/Example_${DIR}" ]; then
                echo "$DIR"
            fi
        fi
    done
}

run_example()
(
    EXAMPLE=$1
    EXE="../../../$BUILD_DIR/Example_${EXAMPLE}.app/Contents/MacOS/Example_${EXAMPLE}"
    cd examples/Cpp/$EXAMPLE
    eval $EXE
)

EXAMPLES=($(list_examples))

PS3="Select example: "
select OPT in "${EXAMPLES[@]}"; do
    run_example $OPT
done
