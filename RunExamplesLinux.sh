#!/bin/bash

BUILD_DIR="build_linux/build"

if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
elif [ -f "build_linux/build/Example_HelloTriangle" ]; then
    BUILD_DIR="build_linux/build"
elif [ -f "bin/x64/Linux/Example_HelloTriangle" ]; then
    BUILD_DIR="bin/x64/Linux"
else
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
