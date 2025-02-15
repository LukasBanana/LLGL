#!/bin/bash

DEFAULT_BUILD_DIRS=("build_linux/build" "build_linux/build/Debug" "bin/Linux-x86_64/build")

BUILD_DIR="${DEFAULT_BUILD_DIRS[0]}"

if [ "$#" -ge 1 ]; then
    if [ "$1" != "--" ]; then
        HELLO_EXAMPLE="Example_HelloTriangle"
        BUILD_DIR=$1
        if [[ "$BUILD_DIR" == */ ]]; then
            BUILD_DIR="${BUILD_DIR::-1}" # Remove trailing '/' character from path
        fi
        if [ -f "$BUILD_DIR/build/$HELLO_EXAMPLE" ] || [ -f "$BUILD_DIR/build/${HELLO_EXAMPLE}D" ]; then
            BUILD_DIR="$BUILD_DIR/build"
        fi
    fi
    shift
else
    for DIR in "${DEFAULT_BUILD_DIRS[@]}"; do
        if [ -d "$DIR" ]; then
            if [ -f "$DIR/libLLGL.so" ] || [ -f "$DIR/libLLGLD.so" ] || [ -f "$DIR/libLLGL.a" ] || [ -f "$DIR/libLLGLD.a" ]; then
                BUILD_DIR="$DIR"
                break
            fi
        fi
    done
fi

# Validate build folder: Check for libLLGL.so/.a or libLLGLD.so/.a files
if [ -d "$BUILD_DIR" ]; then
    if [ -f "$BUILD_DIR/libLLGL.so" ] || [ -f "$BUILD_DIR/libLLGLD.so" ] || [ -f "$BUILD_DIR/libLLGL.a" ] || [ -f "$BUILD_DIR/libLLGLD.a" ]; then
        echo "Run examples from build directory: $BUILD_DIR"
    else
        echo "Error: Missing LLGL base lib (libLLGL.so/.a or libLLGLD.so/.a) in build folder: $BUILD_DIR"
        exit 1
    fi
else
    echo "Error: Build folder not found: $BUILD_DIR"
    exit 1
fi

list_examples()
{
    EXCLUDED=(MultiRenderer) # List any examples that are specifically excluded on Linux
    EXAMPLE_DIRS=($(ls examples/Cpp))
    for DIR in "${EXAMPLE_DIRS[@]}"; do
        if ! echo "${EXCLUDED[@]}}" | grep -qw "$DIR"; then
            # Include example if its source and binary files exist
            if [ -f "examples/Cpp/$DIR/Example.cpp" ] || [ -f "examples/Cpp/$DIR/$DIR.cpp" ]; then
                if [ -f "$BUILD_DIR/Example_$DIR" ] || [ -f "$BUILD_DIR/Example_${DIR}D" ]; then
                    echo "$DIR"
                fi
            fi
        fi
    done
}

run_example()
(
    EXAMPLE=$1
    shift
    EXE="../../../$BUILD_DIR/Example_$EXAMPLE"
    EXE_D="${EXE}D"
    cd examples/Cpp/$EXAMPLE
    if [ -f "$EXE_D" ]; then
        eval $EXE_D $@
    else
        eval $EXE $@
    fi
)

EXAMPLES=($(list_examples))

PS3="Select example: "
select OPT in "${EXAMPLES[@]}"; do
    run_example $OPT $@
done
