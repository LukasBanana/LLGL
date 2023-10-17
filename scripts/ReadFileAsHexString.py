#
# ReadFileAsHexString.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import sys

def printHelp():
    print("help:")
    print("  prints the input file as hex encoded string in C/C++ syntax")
    print("usage:")
    print("  ReadFileAsHexString.py FILE [flags]")
    print("flags:")
    print("  -len:           prints the size in bytes of the input file only")
    print("  -col N:         prints N hex encoded bytes for each row (16 by default)")
    print("  -spaces N:      prints N spaces at the beginning of each row (0 by default)")
    print("  -paren:         prints parenthesis around the output (disabled by default)")
    print("  -offsets STYLE: prints address offsets for each row (disabled by default); accepted styles: 'c', 'cxx'/'c++'")

def fatal(msg):
    print(sys.argv[0] + ': ' + msg)
    sys.exit(1)

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
        fatal('failed to open file: ' + filename)

# Generates the C/C++ header file with a string that contains the binary content
def printHexString(filename, columns=16, spaces=0, offsets='', paren=False):
    if paren:
        print('(')
    byteRange = (0, 0)
    def writeNewline():
        if offsets in ['c++', 'cxx']:
            print('" // 0x{0:0{2}X} - 0x{1:0{2}X}'.format(byteRange[0], byteRange[1], 8))
        elif offsets == 'c':
            print('" /* 0x{0:0{2}X} - 0x{1:0{2}X} */'.format(byteRange[0], byteRange[1], 8))
        else:
            print('"')
    c = 0
    for b in readBytes(filename):
        if c == 0:
            if spaces > 0:
                print(' '*spaces, end='')
            print('"', end='')
        print('\\x{:02X}'.format(b), end='')
        c += 1
        if c >= columns:
            c = 0
            writeNewline()
            byteRange = (byteRange[1] + 1, byteRange[1])
        byteRange = (byteRange[0], byteRange[1] + 1)
    if c > 0:
        writeNewline()
    if paren:
        print(')')

def printFileSize(filename, paren=False):
    n = 0
    for _ in readBytes(filename):
        n += 1
    if paren:
        print(f'( {str(n)} )')
    else:
        print(str(n))

args = sys.argv
if len(args) < 2:
    printHelp()
    sys.exit(1)

def main():
    printLenOnly = False
    columns = 16
    spaces = 0
    filename = ""
    paren = False
    offsets = ''
    i = 1

    def argValue(argName):
        nonlocal i
        if i + 1 < len(args):
            i += 1
            return args[i]
        else:
            fatal('missing value after argument ' + argName)

    while i < len(args):
        arg = args[i]
        if arg == "-len":
            printLenOnly = True
        elif arg == "-paren":
            paren = True
        elif arg == "-offsets":
            offsets = argValue('-offsets')
            if not offsets in ['c', 'cxx', 'c++']:
                fatal(f"accepted offset styles are 'c', 'cxx', and 'c++', but got '{offsets}'");
        elif arg == "-col":
            columns = int(argValue('-col'))
        elif arg == "-spaces":
            spaces = int(argValue('-spaces'))
        else:
            if filename == "":
                filename = arg
            else:
                fatal(f"cannot process more than one filename at a time, but got '{arg}'")
        i += 1

    if filename == "":
        fatal('missing filename')
    if printLenOnly:
        printFileSize(filename, paren)
    else:
        printHexString(filename, columns, spaces, offsets, paren)

main()
