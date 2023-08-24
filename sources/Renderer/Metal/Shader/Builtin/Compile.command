#!/bin/sh

LLGL_ROOT_DIR="$(dirname "$0")/../../../../../"
HEX_CONVERSION_SCRIPT="$LLGL_ROOT_DIR/scripts/ReadFileAsHexString.py"

cd $(dirname "$0")

# Get shader sources
SOURCES=($(ls *.metal))

if [ "$#" -ne 0 ]; then
    SOURCES=("$@")
fi

for SRC in ${SOURCES[@]}; do
    if [ ! -f "$SRC" ]; then
        echo Fatal: Cannot find source file: $SRC
        exit 1
    fi
done

# Check if xcrun tool is available
which xcrun &> /dev/null
if [ $? -ne 0 ]; then
    echo Fatal: Cannot compile Metal shaders without \'xcrun\'
    exit 1
fi

# Compile shaders for macOS
METAL_SDKS=(macosx iphoneos iphonesimulator)

TASK_COUNT=$((${#SOURCES[@]} * ${#METAL_SDKS[@]}))
TASK_INDEX=0

print_percentage()
{
    VALUE=$1
    COUNT=$2
    OUT="$((VALUE * 100 / COUNT))"
    printf "%*s%s" $((3 - ${#OUT})) '' "${OUT}%"
}

for SRC in ${SOURCES[@]}; do
    for SDK in ${METAL_SDKS[@]}; do
        FILE="${SRC%.*}.${SDK}"
        echo "[$(print_percentage $TASK_INDEX $TASK_COUNT)] Compiling Metal shader $FILE"
        xcrun -sdk $SDK metal -c ${SRC} -o ${FILE}.air
        xcrun -sdk $SDK metallib ${FILE}.air -o ${FILE}.metallib
        TASK_INDEX=$((TASK_INDEX + 1))
    done

done

# Check if Python 3 is avilable
which python3 &> /dev/null
if [ $? -ne 0 ]; then
    echo Fatal: Cannot convert MetalLib files to hex strings without \'python3\'
    exit 1
fi

# Convert shader byte code into C header files
for SRC in ${SOURCES[@]}; do
    for SDK in ${METAL_SDKS[@]}; do
        FILE="${SRC%.*}.${SDK}"
        python3 "$HEX_CONVERSION_SCRIPT" ${FILE}.metallib -offsets cxx > ${FILE}.metallib.bin.inl
        python3 "$HEX_CONVERSION_SCRIPT" ${FILE}.metallib -len -paren > ${FILE}.metallib.len.inl
    done
done

echo "[100%] DONE"
