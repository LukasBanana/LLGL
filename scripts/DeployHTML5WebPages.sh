#!/bin/bash

# Parse input arguments
SRC=$1
DST=$2
shift 2

DEBUG_BUILD=0

for ARG in "$@"; do
    if [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
        DEBUG_BUILD=1
    fi
done

if [ -z "$SRC" ]; then
    echo "Usage: $0 SOURCE [TARGET [...]]"
    echo "Additional arguments are forwarded to BuildWasm.sh script"
    exit 1
fi

if [ -z "$DST" ]; then
    DST=$(realpath "$SRC/build_wasm")
fi

if [ ! -f "$SRC/BuildWasm.sh" ]; then
    echo "Error: Build script '$SRC/BuildWasm.sh' not found!"
    exit 1
fi

if [ ! -d "$DST" ]; then
    echo "Error: Destination directory '$DST' not found!"
    exit 1
fi

copy_example_to_target()
{
    BASENAME="$1"
    if [ $DEBUG_BUILD -ne 0 ]; then
        NAME="${BASENAME}D"
    else
        NAME="$BASENAME"
    fi
    SOURCE_FOLDER="$DST/html5/$NAME"
    TARGET_FOLDER="$SRC/docu/WebPage/$BASENAME"
    echo "Copy $NAME from '$(realpath --relative-to "$SRC" "$SOURCE_FOLDER")' to '$(realpath --relative-to "$SRC" "$TARGET_FOLDER")'"
    cp "$SOURCE_FOLDER/$NAME.data" "$TARGET_FOLDER/$NAME.data"
    cp "$SOURCE_FOLDER/$NAME.data.js" "$TARGET_FOLDER/$NAME.data.js"
    cp "$SOURCE_FOLDER/$NAME.js" "$TARGET_FOLDER/$NAME.js"
    cp "$SOURCE_FOLDER/$NAME.wasm" "$TARGET_FOLDER/$NAME.wasm"
    cp "$SOURCE_FOLDER/index.html" "$TARGET_FOLDER/index.html"
}

# Change to the source directory before building
pushd $SRC

# If this is run in WSL2 on Windows, remove carriage return characters (Windows EOL) from the script before executing it
if grep -q $'\r$' BuildWasm.sh; then
    tr -d '\r' < BuildWasm.sh | bash -s -- -u "$DST" "${@:1}"
else
    # Forward additional arguments to the build script
    BuildWasm.sh -u "$DST" "${@:1}"
fi

# Copy HTML5 examples to the target web page directory
declare -a EXAMPLES
EXAMPLES=(
    "Example_BrickPlay"
    "Example_Fonts"
    "Example_HelloGame"
    "Example_Instancing"
    "Example_MorphTargets"
    "Example_Orthogonal"
    "Example_PBR"
    "Example_PostProcessing"
    "Example_RenderTarget"
    "Example_ShadowMapping"
    "Example_StencilBuffer"
    "Example_Texturing"
    "Example_VolumeRendering"
)

for EXAMPLE in "${EXAMPLES[@]}"; do
    copy_example_to_target "$EXAMPLE"
done

popd
