#!/bin/bash

BUILD_DIR="build_linux/build"

if [ "$#" -eq 1 ]; then
    HELLO_EXAMPLE="Example_HelloTriangle"
    BUILD_DIR=$1
    if [[ "$BUILD_DIR" == */ ]]; then
        BUILD_DIR="${BUILD_DIR::-1}" # Remove trailing '/' character from path
    fi
    if [ -f "$BUILD_DIR/build/$HELLO_EXAMPLE" ] || [ -f "$BUILD_DIR/build/${HELLO_EXAMPLE}D" ]; then
        BUILD_DIR="$BUILD_DIR/build"
    fi
elif [ ! -d "$BUILD_DIR" ]; then
    echo "error: build folder not found: $BUILD_DIR"
    exit 1
fi

list_examples()
{
    EXCLUDED=(MultiRenderer MultiThreading PBR ComputeShader UnorderedAccess)
    EXAMPLE_DIRS=($(ls examples/Cpp))
    for DIR in "${EXAMPLE_DIRS[@]}"; do
        if ! echo "${EXCLUDED[@]}}" | grep -qw "$DIR"; then
            if [ -f "examples/Cpp/$DIR/Example.cpp" ]; then
                echo "$DIR"
            fi
        fi
    done
}

run_example()
(
    EXAMPLE=$1
    EXE="../../../$BUILD_DIR/Example_$EXAMPLE"
    EXE_D="${EXE}D"
    cd examples/Cpp/$EXAMPLE
    if [ -f "$EXE_D" ]; then
        eval $EXE_D
    else
        eval $EXE
    fi
)

EXAMPLES=($(list_examples))

PS3="Select example: "
select OPT in "${EXAMPLES[@]}"; do
    run_example $OPT
done
