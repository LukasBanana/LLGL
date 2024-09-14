#!/bin/sh

# Part of the LLGL project
# Written by L. Hermanns 9/9/2024
# -------------------------------
# Compiles Metal shaders to a single default.metallib file for app bundles.
# The input can either be a single Metal shader file, an app folder to compile all of its *.metal files,
# or a directory with multiple *.app/ subfolders for which a separate default.metallib is generated each.

if [ "$#" -lt 2 ]; then
    echo "Usage: CompileMetalToMetallib.sh SDK INPUT [-v|--verbose]"
    exit 1
fi

# Parse input parameters
METAL_SDK=$1
INPUT_PATH=$2
VERBOSE=0

if [ "$#" -eq 3 ]; then
    if [ "$3" = "-v" ] || [ "$3" = "--verbose" ]; then
        VERBOSE=1
    fi
fi

# Compiles input Metal shaders to a single default.metallib output.
# Example: compile_metal_shaders OUTPUT INPUT1 INPUT2 ...
compile_metal_shaders()
{
    OUTPUT_DIR=$1
    shift # get remaining arguments (compatible with sh)
    INPUT_FILES=$@
    if [ ! -z "$INPUT_FILES" ]; then
        if [ $VERBOSE -ne 0 ]; then
            echo "Compile Metal shaders: ${INPUT_FILES[@]}"
        fi
        xcrun -sdk $METAL_SDK metal ${INPUT_FILES[@]} -o "$OUTPUT_DIR/default.metallib"
    fi
}

# Compiles all Metal shaders for the specified app folder.
# Example: compile_all_app_metal_shaders Example_HelloTriangle.app
compile_all_app_metal_shaders()
{
    APP_DIR=$1
    APP_RESOURCES_DIR="$APP_DIR/Contents/Resources"
    SHADER_FILES=()
    for FILE in $APP_RESOURCES_DIR/*.metal; do
        if [ -f $FILE ]; then
            SHADER_FILES+=($FILE)
        fi
    done
    compile_metal_shaders "$APP_RESOURCES_DIR" ${SHADER_FILES[@]}
}

if [ -f "$INPUT_PATH" ]; then
    # Compile the specified input Metal shader
    compile_metal_shaders $(dirname "$INPUT_PATH") "$INPUT_PATH"
elif [ -d "$INPUT_PATH/Contents/Resources" ]; then
    # Compile all *.metal shaders for the specified app
    compile_all_app_metal_shaders "$INPUT_PATH"
else
    # Compile all *.metal shaders for all subdirectory apps
    TARGET_APPS=($INPUT_PATH/*.app)
    for APP_DIR in ${TARGET_APPS[@]}; do
        compile_all_app_metal_shaders "$APP_DIR"
    done
fi
