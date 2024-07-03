#!/usr/bin/env python3
#
# __main__.py (WrapperGen)
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import sys
import llgl_parser as parser
import llgl_translator_c99 as translator_c99
import llgl_translator_csharp as translator_csharp
import llgl_translator_golang as translator_golang

def iterate(func, cont):
    return list(map(func, cont))

def printHelp():
    print("help:")
    print("  parses LLGL headers and stores the meta data")
    print("usage:")
    print("  WrapperGen FILE+ [flags]")
    print("flags:")
    print("  -c99 ......... Translate header to C99")
    print("  -csharp ...... Translate header to C#")
    print("  -golang ...... Translate header to Go")
    print("  -name=NAME ... Override name for consolidated headers")
    print("  -fn .......... Also parse exported C function declarations")
    print("  -tok ......... Scan tokens only")

def scanAndPrintTokens(filename):
    prs = parser.Parser()
    prs.scanner.scan(filename)
    for tok in prs.scanner.tokens:
        print(tok)

def parseFile(filename, processFunctions = False):
    prs = parser.Parser()
    mod = prs.parseHeader(filename, processFunctions)
    mod.deriveDependencies()
    return mod

def printModule(module):
    def printField(field, fieldType):
        print('@' + fieldType + '{' + str(field) + '}')

    def printRecord(record, recordType):
        print('@' + recordType + '{' + record.name + '}')
        iterate(lambda field: printField(field, 'FIELD'), record.fields)
        print('@END')

    def printFunc(func, funcType):
        print('@' + funcType + '{' + func.name + '}=>' + str(func.returnType))
        iterate(lambda param: printField(param, 'PARAM'), func.params)
        print('@END')

    print('@HEADER{' + module.name + '}')
    iterate(lambda record: printRecord(record, 'CONST'), filter(lambda record: record.hasConstFieldsOnly(), module.structs))
    iterate(lambda record: printRecord(record, 'ENUM'), module.enums)
    iterate(lambda record: printRecord(record, 'FLAG'), module.flags)
    iterate(lambda record: printRecord(record, 'STRUCT'), filter(lambda record: not record.hasConstFieldsOnly(), module.structs))
    iterate(lambda func: printFunc(func, 'FUNC'), module.funcs)
    iterate(lambda delegate: printFunc(delegate, 'DELEGATE'), module.delegates)
    print('@END')

args = sys.argv[1:]
files = list(filter(lambda arg: len(arg) > 0 and arg[0] != '-', args))
if len(files) > 0:
    # Is there an override name to use as single header output?
    def findArgValue(args, search):
        argIndex = 0
        while argIndex < len(args):
            arg = args[argIndex]
            if len(arg) > len(search) + 1 and arg[:len(search)] == search and arg[len(search)] == '=':
                return arg[len(search) + 1:]
            argIndex += 1
        return None

    singleName = findArgValue(args, '-name')

    # Scan and print tokens only?
    if '-tok' in args:
        for file in files:
            scanAndPrintTokens(file)
    else:
        # Are function declarations includes?
        processFunctions = '-fn' in args
        
        # Parse input headers
        modules = iterate(lambda filename: parseFile(filename, processFunctions), files)
        if singleName and len(modules) > 0:
            singleModule = modules[0]
            singleModule.name = singleName
            if len(modules) > 1:
                for module in modules[1:]:
                    singleModule.merge(module)
            singleModule.structs = singleModule.sortStructsByDependencies()
            modules = [singleModule]

        # Translate or just print meta data of input header files
        if '-c99' in args:
            trans = translator_c99.C99Translator()
            iterate(trans.translateModule, modules)
        elif '-csharp' in args:
            trans = translator_csharp.CsharpTranslator()
            iterate(trans.translateModule, modules)
        elif '-golang' in args:
            trans = translator_golang.GolangTranslator()
            iterate(trans.translateModule, modules)
        else:
            iterate(printModule, modules)
else:
    printHelp()
