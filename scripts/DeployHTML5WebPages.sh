#!/bin/bash

# Validate input arguments
SRC=$1
DST=$2
FILTERS=("${@:3}")

if [ -z "$SRC" ]; then
    echo "Usage: $0 <source> [<destination> [<example>...]]"
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
    NAME=$1
    SOURCE_FOLDER="$DST/html5/$NAME"
    TARGET_FOLDER="$SRC/docu/WebPage/$NAME"
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
    tr -d '\r' < BuildWasm.sh | bash -s -- -u "$DST"
else
    BuildWasm.sh -u "$DST"
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
    if [ ${#FILTERS[@]} -gt 0 ]; then
        SELECTED=0
        for FILTER in "${FILTERS[@]}"; do
            # Accept filters with or without "Example_" prefix
            if [ "$EXAMPLE" == "$FILTER" ] || [ "$EXAMPLE" == "Example_$FILTER" ]; then
                SELECTED=1
                break
            fi
        done
        if [ $SELECTED -eq 0 ]; then
            continue
        fi
    fi
    copy_example_to_target "$EXAMPLE"
done

popd
