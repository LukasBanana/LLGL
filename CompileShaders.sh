#!/bin/bash

SOURCE_DIR=$PWD
OUTPUT_DIR="build_linux"
VERBOSE=0
SKIP_PROMPT=0
NO_COLORS=0

COL_RED='\033[0;31m' # Red color for errors
COL_YELLOW='\033[0;33m' # Yellow color for warnings
COL_END='\033[0m' # No Color

print_help()
{
    echo "USAGE:"
    echo "  CompileShaders.sh OPTIONS* [OUTPUT_DIR]"
    echo "OPTIONS:"
    echo "  -h, --help ................ Print this help documentation and exit"
    echo "  -v, --verbose ............. Print additional information"
    echo "  -y, --yes ................. Skip the prompt and proceed automatically"
    echo "  --no-colors ............... Disable colored output"
    echo ""
    echo "NOTES:"
    echo "  Default output directory is '$OUTPUT_DIR'"
}

print_tools_info()
{
    echo "Tools to compile shaders for LLGL examples *before* configuring the CMake build:"
    echo " Required:"
    echo "  -> \`dxc\` (DirectXShaderCompiler) with *SPIR-V* support:"
    echo "        https://github.com/microsoft/DirectXShaderCompiler"
    echo " Optional:"
    echo "  -> \`spirv-cross\` (SPIRV-Cross):"
    echo "        https://github.com/KhronosGroup/SPIRV-Cross"
    echo "  -> \`spirv-opt\` (SPIRV-Tools):"
    echo "        https://github.com/KhronosGroup/SPIRV-Tools"
}

prompt_continue()
{
    if [ $SKIP_PROMPT -eq 0 ]; then
        echo "Would you like to proceed? (Y/n)"
        read -r RESPONSE
        if [[ "$RESPONSE" =~ ^[Nn]$ ]]; then
            exit 0
        fi
    fi
}

# Parse arguments
for ARG in "$@"; do
    if [ "$ARG" = "-h" ] || [ "$ARG" = "--help" ]; then
        print_help
        exit 0
    elif [ "$ARG" = "-v" ] || [ "$ARG" = "--verbose" ]; then
        VERBOSE=1
    elif [ "$ARG" = "-y" ] || [ "$ARG" = "--yes" ]; then
        SKIP_PROMPT=1
    elif [ "$ARG" = "--no-colors" ]; then
        NO_COLORS=1
    else
        OUTPUT_DIR="$ARG"
    fi
done

if [ $NO_COLORS -eq 1 ]; then
    COL_RED=''
    COL_YELLOW=''
    COL_END=''
fi

# Ensure we are inside the repository folder
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${COL_RED}Error: File not found: CMakeLists.txt${COL_END}"
    exit 1
fi

# Check whether the output directory exists
if [ ! -d "$OUTPUT_DIR" ]; then
    echo -e "${COL_RED}Error: Output directory does not exist: $OUTPUT_DIR${COL_END}"
    echo "Run ./BuildLinux.sh first!"
    exit 1
fi

# Check what tools are installed
if ! command -v dxc &> /dev/null; then
    echo -e "${COL_RED}Error: \`dxc\` (DirectXShaderCompiler) is not installed!${COL_END}"
    print_tools_info
    exit 1
fi

if ! command -v spirv-cross &> /dev/null; then
    echo -e "${COL_YELLOW}Warning: \`spirv-cross\` (SPIRV-Cross) is not installed!${COL_END}"
    print_tools_info
    prompt_continue
fi

if ! command -v spirv-opt &> /dev/null; then
    echo -e "${COL_YELLOW}Warning: \`spirv-opt\` (SPIRV-Tools) is not installed!${COL_END}"
    print_tools_info
    prompt_continue
fi

# Run the build command for compiling shaders
if [ $VERBOSE -ne 0 ]; then
    cmake --build "$OUTPUT_DIR" --target COMPILE_SHADERS
    echo "DONE"
else
    cmake --build "$OUTPUT_DIR" --target COMPILE_SHADERS
fi
