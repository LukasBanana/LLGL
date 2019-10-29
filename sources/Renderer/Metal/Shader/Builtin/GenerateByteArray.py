#
# GenerateByteArray.py
#
# This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
# See "LICENSE.txt" for license information.
#

import os
import sys
import struct

def printHelp():
    print("help: encodes the input file to a single byte array in a C/C++ header file")
    print("usage: GenerateByteArray.py FILE+")

def readBytes(filename, chunkSize=8192):
    try:
        with open(filename, mode="rb") as file:
            while True:
                chunk = file.read(chunkSize)
                if chunk:
                    for b in chunk:
                        yield b
                else:
                    break
    except IOError:
        print("failed to open file: " + filename)
        sys.exit(0)

# Generates the C/C++ header file with a string that contains the binary content
def generateHeader(filename, columns=16, dumpAddressOffset=True):
    shaderName = os.path.basename(filename)
    shaderName = os.path.splitext(shaderName)[0]

    arrayFilename = shaderName + ".h"
    arrayLenFilename = shaderName + ".Len.h"

    print("generate '" + arrayFilename + "' and '" + arrayLenFilename + "'")

    with open(arrayFilename, mode="w") as output:
        range = (0, 0)
        def writeNewline():
            if dumpAddressOffset:
                output.write('" // 0x{0:0{2}X} - 0x{1:0{2}X}\n'.format(range[0], range[1], 8))
            else:
                output.write("\"\n")
        c = 0
        for b in readBytes(filename):
            if c == 0:
                output.write("\"")
            output.write("\\x" + b.encode("hex").upper())
            c += 1
            if c >= columns:
                c = 0
                writeNewline()
                range = (range[1] + 1, range[1])
            range = (range[0], range[1] + 1)
        if c > 0:
            writeNewline()

    with open(arrayLenFilename, mode="w") as output:
        output.write("( " + str(range[1]) + " )\n")

if len(sys.argv) >= 2:
    for i in range(len(sys.argv) - 1):
        generateHeader(sys.argv[i + 1])
else:
    printHelp()
