#!/bin/sh

SOURCE_DIR="$(dirname $0)"
BUILD_DIR="$SOURCE_DIR/build_macos/build"

# When this .command script is launched from Finder, we have to change to the source directory explicitly
cd $SOURCE_DIR

if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
elif [ -d "$SOURCE_DIR/build_macos/build/Example_HelloTriangle.app" ]; then
    BUILD_DIR="build_macos/build"
elif [ -d "$SOURCE_DIR/bin/x64/MacOS/Example_HelloTriangle.app" ]; then
    BUILD_DIR="bin/x64/MacOS"
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
    EXE="../../../$BUILD_DIR/Example_${EXAMPLE}.app/Contents/MacOS/Example_${EXAMPLE}"
    cd examples/Cpp/$EXAMPLE
    eval $EXE
)

EXAMPLES=($(list_examples))

PS3="Select example: "
select OPT in "${EXAMPLES[@]}"; do
    run_example $OPT
done
