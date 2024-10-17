#!/bin/bash

# Part of the LLGL project
# Written by L. Hermanns 9/14/2024
# --------------------------------
# Generates the bundles HTML5 web pages for all example projects.

if [ "$#" -lt 2 ]; then
    echo "Usage: GenerateHTML5Examples.sh SOURCE TARGET [-d|--debug] [-v|--verbose] [--pthreads]"
    exit 1
fi

# Parse input arguments
SOURCE_DIR=$1
TARGET_DIR=$2
shift 2

DEBUG_BUILD=0
VERBOSE=0
ENABLE_PTHREADS=0

for ARG in "$@"; do
    if [ "$ARG" = "-d" ] || [ "$ARG" = "--debug" ]; then
        DEBUG_BUILD=1
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [ "$ARG" = "--pthreads" ]; then
        ENABLE_PTHREADS=1
    fi
done

# Find Emscripten SDK
if [ -z "$EMSDK" ]; then
    echo "Error: Missing EMSDK environment variable. Run 'source <PATH-TO-EMSDK>/emsdk_env.sh' to fix it."
    exit 1
fi

EMSCRIPTEN_FILE_PACKAGER="$EMSDK/upstream/emscripten/tools/file_packager"

# Copys the input file to the output and removes '\r' EOL characters from text files.
# Web page will run on Linux server and Git must not convert EOL for this output file.
# Otherwise, data offsets in the *.data.js script won't match with the *.data file after Git uploaded it.
copy_file_preserve_linux_eol()
{
    INPUT=$1
    OUTPUT=$2
    FILE_EXT=${INPUT##*.}
    if [[ "${FILE_EXT,,}" =~ ^(txt|vert|frag|obj)$ ]]; then
        if [ $VERBOSE -ne 0 ]; then
            echo "Copy asset (convert EOL): $(basename $INPUT)"
        fi
        tr -d "\r" < "$INPUT" > "$OUTPUT"
    else
        if [ $VERBOSE -ne 0 ]; then
            echo "Copy asset: $(basename $INPUT)"
        fi
        cp "$INPUT" "$OUTPUT"
    fi
}

# Generate HTML pages
generate_html5_page()
{
    CURRENT_PROJECT=$1

    echo "Generate HTML5 page: $CURRENT_PROJECT"

    # Get source folder
    ASSETS_SOURCE_DIR="$SOURCE_DIR/examples/Shared/Assets"
    PROJECT_SOURCE_DIR="$SOURCE_DIR/examples/Cpp"
    if [[ "$CURRENT_PROJECT" == *D ]]; then
        PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/${CURRENT_PROJECT:0:-1}"
    else
        PROJECT_SOURCE_DIR="$PROJECT_SOURCE_DIR/$CURRENT_PROJECT"
    fi

    # Get destination folder
    HTML5_ROOT="${TARGET_DIR}/html5/Example_$CURRENT_PROJECT"
    BIN_ROOT=${TARGET_DIR}/build

    # Create folder structure
    mkdir -p "$HTML5_ROOT"
    BASE_FILENAME="Example_$CURRENT_PROJECT"
    cp "$BIN_ROOT/$BASE_FILENAME.js" "$HTML5_ROOT/$BASE_FILENAME.js"
    cp "$BIN_ROOT/$BASE_FILENAME.wasm" "$HTML5_ROOT/$BASE_FILENAME.wasm"
    if [ $ENABLE_PTHREADS -ne 0 ]; then
        cp "$BIN_ROOT/$BASE_FILENAME.worker.js" "$HTML5_ROOT/$BASE_FILENAME.worker.js"
    fi

    # Replace meta data
    sed -e "s/LLGL_EXAMPLE_NAME/${CURRENT_PROJECT}/g" \
        -e "s/LLGL_EXAMPLE_PROJECT/Example_${CURRENT_PROJECT}/g" \
        "$SOURCE_DIR/examples/Shared/Platform/Wasm/index.html" > "$HTML5_ROOT/index.html"
    
    # Find all required assets in *.assets.txt file of respective project directory and copy them into app folder
    ASSET_DIR="$HTML5_ROOT/assets"
    mkdir -p "$ASSET_DIR"

    ASSETS_LIST_FILE=$(find "$PROJECT_SOURCE_DIR" -type f -name *.assets.txt)
    if [ -f "$ASSETS_LIST_FILE" ]; then
        # Read asset filenames to copy to package output
        # and remove '\r' characters when reading the *.asset.txt file from WSL
        ASSET_FILES=()
        for FILTER in $(cat $ASSETS_LIST_FILE | tr -d '\r'); do
            # Search for patterns in both the shared assets and current project folder
            for FILE in $ASSETS_SOURCE_DIR/$FILTER; do
                if [ -f "$FILE" ]; then
                    ASSET_FILES+=( "$FILE" )
                fi
            done
            for FILE in $PROJECT_SOURCE_DIR/$FILTER; do
                if [ -f "$FILE" ]; then
                    ASSET_FILES+=( "$FILE" )
                fi
            done
        done

        # Copy all asset file into destination folder
        for FILE in ${ASSET_FILES[@]}; do
            copy_file_preserve_linux_eol "$FILE" "$ASSET_DIR/$(basename $FILE)"
        done
    fi

    # Find all shaders and copy them into app folder
    for FILE in $PROJECT_SOURCE_DIR/*.vert \
                $PROJECT_SOURCE_DIR/*.frag; do
        if [ -f "$FILE" ]; then
            copy_file_preserve_linux_eol "$FILE" "$ASSET_DIR/$(basename $FILE)"
        fi
    done

    # Package assets into .data.js file with Emscripten packager tool
    (cd "$HTML5_ROOT" && "$EMSCRIPTEN_FILE_PACKAGER" "$BASE_FILENAME.data" --preload assets "--js-output=$BASE_FILENAME.data.js" >/dev/null 2>&1)
}

BIN_FILE_BASE="${TARGET_DIR}/build/Example_"
BIN_FILE_BASE_LEN=${#BIN_FILE_BASE}

if [ $DEBUG_BUILD -ne 0 ]; then
    EXAMPLE_BIN_FILES=(${BIN_FILE_BASE}*D.wasm)
else
    EXAMPLE_BIN_FILES=(${BIN_FILE_BASE}*.wasm)
fi

for BIN_FILE in ${EXAMPLE_BIN_FILES[@]}; do
    BIN_FILE_LEN=${#BIN_FILE}
    PROJECT_NAME=${BIN_FILE:BIN_FILE_BASE_LEN:BIN_FILE_LEN-BIN_FILE_BASE_LEN-5}
    generate_html5_page $PROJECT_NAME
done
