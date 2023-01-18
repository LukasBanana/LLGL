#!/bin/sh

if [ $# -ne 1 ] && [ $# -ne 2 ]; then
    echo "usage: ReadFileAsHexStringDecl.sh INPUT [DECL]"
    exit 1
fi

INPUT=$1
DECL=$(basename $1)
if [ $# -eq 2 ]; then
    DECL=$2
fi

SCRIPT_PATH="$(dirname $0)/ReadFileAsHexString.py"
echo "static const char* ${DECL} =" | sed -e "s/\./_/"
python3 "$SCRIPT_PATH" -spaces 4 -offsets cxx "$INPUT"
echo ";"
echo -n "static const std::size_t ${DECL}_Len = " | sed -e "s/\./_/"
STR_LEN_LINE=$(python3 "$SCRIPT_PATH" -len -paren "$INPUT")
echo "${STR_LEN_LINE};"
