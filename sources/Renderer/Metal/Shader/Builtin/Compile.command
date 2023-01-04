#!/bin/sh
SCRIPT_PATH="$(realpath $0)"
LLGL_ROOT_DIR="$(dirname "$SCRIPT_PATH")/../../../../../"
HEX_CONVERSION_SCRIPT="$LLGL_ROOT_DIR/scripts/ReadFileAsHexString.py"

which xcrun > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo fatal: cannot compile Metal shaders without \'xcrun\'
    exit 1
fi
xcrun -sdk macosx metal -c FillBufferByte4.metal -o FillBufferByte4.air
xcrun -sdk macosx metallib FillBufferByte4.air -o FillBufferByte4.metallib

which python3 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo fatal: cannot convert MetalLib files to hex strings without \'python3\'
    exit 1
fi
python3 "$HEX_CONVERSION_SCRIPT" FillBufferByte4.metallib -offsets cxx > FillBufferByte4.metallib.bin.h
python3 "$HEX_CONVERSION_SCRIPT" FillBufferByte4.metallib -len -paren > FillBufferByte4.metallib.len.h
