#!/bin/bash

# Deployment configuration
declare -ar EXAMPLES=(
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
declare -ar FULLSCREEN_EXAMPLES=(
    "Example_BrickPlay"
    "Example_Fonts"
)

# Parse input arguments
LLGL_ROOT=$1
OUTPUT_DIR=$2
BUILD_DIR=$3
shift 3

DEBUG_BUILD=0
DEPLOY_ONLY=0

for ARG in "$@"; do
    if [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
        DEBUG_BUILD=1
    elif [ "$ARG" = "-s" ] || [ "$ARG" = "--skip-build" ]; then
        DEPLOY_ONLY=1
    fi
done

if [ -z "$LLGL_ROOT" ] || [ -z "$OUTPUT_DIR" ]; then
    echo "Usage: $0 LLGL_ROOT OUTPUT_DIR [WASM_BUILD_DIR [...]]"
    echo "  Deploys HTML5 web pages from the build output to the target directory"
    echo "  Additional arguments are forwarded to BuildWasm.sh script"
    echo "  Use -d/--debug to enable debug build"
    echo "  Use -s/--skip-build to skip the build step and only deploy the web pages"
    exit 1
fi

if [ -z "$BUILD_DIR" ]; then
    BUILD_DIR=$(realpath "$LLGL_ROOT/build_wasm")
fi

if [ ! -f "$LLGL_ROOT/BuildWasm.sh" ]; then
    echo "Error: Build script '$LLGL_ROOT/BuildWasm.sh' not found!"
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Destination directory '$BUILD_DIR' not found!"
    exit 1
fi

copy_example_thumbnail_to_target()
{
    BASENAME="$1"
    TARGET_FOLDER="$2"
    EXAMPLE_BASENAME=Example_
    NAME="${BASENAME:${#EXAMPLE_BASENAME}}"

    if [ -f "$LLGL_ROOT/examples/Cpp/$NAME/$NAME.gif" ]; then
        cp "$LLGL_ROOT/examples/Cpp/$NAME/$NAME.gif" "$TARGET_FOLDER/Example.gif"
    elif [ -f "$LLGL_ROOT/examples/Cpp/$NAME/$NAME.png" ]; then
        cp "$LLGL_ROOT/examples/Cpp/$NAME/$NAME.png" "$TARGET_FOLDER/Example.png"
    elif [ -f "$LLGL_ROOT/examples/Cpp/$NAME/Example.png" ]; then
        cp "$LLGL_ROOT/examples/Cpp/$NAME/Example.png" "$TARGET_FOLDER/Example.png"
    else
        echo "Warning: Missing thumbnail for '$NAME'!"
    fi
}

# This substitutes the href to the stylesheet for the given example to the appropriate one (fullscreen or regular)
substitute_stylesheet()
{
    BASENAME="$1"
    WEBPAGE_FILENAME="$OUTPUT_DIR/$BASENAME/index.html"

    if [ -f "$WEBPAGE_FILENAME" ]; then
        sed -i 's/style.webapp.css/style.webapp.fullscreen.css/' "$WEBPAGE_FILENAME"
    else
        echo "Warning: Web page '$WEBPAGE_FILENAME' not found!"
    fi
}

copy_example_to_target()
{
    BASENAME="$1"
    if [ $DEBUG_BUILD -ne 0 ]; then
        NAME="${BASENAME}D"
    else
        NAME="$BASENAME"
    fi

    # Use $BUILD_DIR here as the copy-source is the build output directory
    SOURCE_FOLDER="$BUILD_DIR/html5/$NAME"

    # Use $OUTPUT_DIR here as the copy-target is the web page output directory
    TARGET_FOLDER="$OUTPUT_DIR/$BASENAME"

    if [ ! -d "$TARGET_FOLDER" ]; then
        mkdir -p "$TARGET_FOLDER"
    fi

    echo "Copy $NAME from '$(realpath --relative-to "$LLGL_ROOT" "$SOURCE_FOLDER")' to $TARGET_FOLDER"
    cp "$SOURCE_FOLDER/$NAME.data" "$TARGET_FOLDER/$NAME.data"
    cp "$SOURCE_FOLDER/$NAME.data.js" "$TARGET_FOLDER/$NAME.data.js"
    cp "$SOURCE_FOLDER/$NAME.js" "$TARGET_FOLDER/$NAME.js"
    cp "$SOURCE_FOLDER/$NAME.wasm" "$TARGET_FOLDER/$NAME.wasm"
    cp "$SOURCE_FOLDER/index.html" "$TARGET_FOLDER/index.html"
    copy_example_thumbnail_to_target "$BASENAME" "$TARGET_FOLDER"
}

# Change to the source directory before building
pushd $LLGL_ROOT

if [ $DEPLOY_ONLY -eq 0 ]; then
    # If this is run in WSL2 on Windows, remove carriage return characters (Windows EOL) from the script before executing it
    if grep -q $'\r$' BuildWasm.sh; then
        tr -d '\r' < BuildWasm.sh | bash -s -- -u "$BUILD_DIR" "${@:1}"
    else
        # Forward additional arguments to the build script
        BuildWasm.sh -u "$BUILD_DIR" "${@:1}"
    fi
fi

# Copy HTML5 examples to the target web page directory
for EXAMPLE in "${EXAMPLES[@]}"; do
    copy_example_to_target "$EXAMPLE"
done

# Substitute the stylesheet for fullscreen examples
for EXAMPLE in "${FULLSCREEN_EXAMPLES[@]}"; do
    substitute_stylesheet "$EXAMPLE"
done

popd
