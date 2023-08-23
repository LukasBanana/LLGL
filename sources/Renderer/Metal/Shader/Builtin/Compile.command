#!/bin/sh

LLGL_ROOT_DIR="$(dirname "$0")/../../../../../"
HEX_CONVERSION_SCRIPT="$LLGL_ROOT_DIR/scripts/ReadFileAsHexString.py"

# Check if xcrun tool is available
which xcrun &> /dev/null
if [ $? -ne 0 ]; then
    echo fatal: cannot compile Metal shaders without \'xcrun\'
    exit 1
fi

# Compile shaders for macOS
METAL_SDKS=(macosx iphoneos iphonesimulator)

for SDK in ${METAL_SDKS[@]}; do
    echo Compile Metal shaders for $SDK
    xcrun -sdk $SDK metal -c FillBufferByte4.metal -o FillBufferByte4.${SDK}.air
    xcrun -sdk $SDK metallib FillBufferByte4.${SDK}.air -o FillBufferByte4.${SDK}.metallib
done

# Check if Python 3 is avilable
which python3 &> /dev/null
if [ $? -ne 0 ]; then
    echo fatal: cannot convert MetalLib files to hex strings without \'python3\'
    exit 1
fi

# Convert shader byte code into C header files
for SDK in ${METAL_SDKS[@]}; do
    python3 "$HEX_CONVERSION_SCRIPT" FillBufferByte4.${SDK}.metallib -offsets cxx > FillBufferByte4.${SDK}.metallib.bin.h
    python3 "$HEX_CONVERSION_SCRIPT" FillBufferByte4.${SDK}.metallib -len -paren > FillBufferByte4.${SDK}.metallib.len.h
done

echo Done
