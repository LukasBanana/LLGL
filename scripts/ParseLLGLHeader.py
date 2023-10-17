#
# ParseLLGLHeader.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import os
import sys
import re
from enum import Enum

def printHelp():
    print("help:")
    print("  parses LLGL headers and stores the meta data")
    print("usage:")
    print("  ParseLLGLHeader.py FILE+ [flags]")
    print("flags:")
    print("  -c99 ......... Translate header to C99")
    print("  -csharp ...... Translate header to C#")
    print("  -name=NAME ... Override name for consolidated headers")
    print("  -fn .......... Also parse exported C function declarations")

def iterate(func, cont):
    return list(map(func, cont))

def fatal(msg):
    print(sys.argv[0] + ': ' + msg)
    sys.exit(1)

class StdType(Enum):
    UNDEFINED = 0
    VOID = 1
    BOOL = 2
    CHAR = 3
    INT8 = 4
    INT16 = 5
    INT32 = 6
    INT64 = 7
    UINT8 = 8
    UINT16 = 9
    UINT32 = 10
    UINT64 = 11
    LONG = 12
    SIZE_T = 13
    FLOAT = 14
    ENUM = 15
    FLAGS = 16
    STRUCT = 17
    CONST = 18 # static constexpr int

class ConditionalType:
    name = ''
    cond = None
    include = None

    def __init__(self, name, cond = None, include = None):
        self.name = name
        self.cond = cond
        self.include = include

class LLGLMeta:
    UTF8STRING = 'UTF8String'
    STRING = 'string'
    externals = [
        ConditionalType('android_app', 'defined LLGL_OS_ANDROID', '<android_native_app_glue.h>')
    ]
    builtins = {
        'void': StdType.VOID,
        'bool': StdType.BOOL,
        'char': StdType.CHAR,
        'int8_t': StdType.INT8,
        'int16_t': StdType.INT16,
        'short': StdType.INT16,
        'int32_t': StdType.INT32,
        'int': StdType.INT32,
        'int64_t': StdType.INT64,
        'uint8_t': StdType.UINT8,
        'uint16_t': StdType.UINT16,
        'uint32_t': StdType.UINT32,
        'uint64_t': StdType.UINT64,
        'long': StdType.LONG,
        'size_t': StdType.SIZE_T,
        'float': StdType.FLOAT,
        'const': StdType.CONST
    }
    containers = [
        'vector',
        'ArrayView'
    ]
    interfaces = [
        'Buffer',
        'BufferArray',
        'Canvas',
        'CommandBuffer',
        'CommandQueue',
        'Display',
        'Fence',
        'Image',
        'PipelineLayout',
        'PipelineState',
        'QueryHeap',
        'RenderPass',
        'RenderSystem',
        'RenderTarget',
        'RenderingDebugger',
        'RenderingProfiler',
        'Report',
        'Resource',
        'ResourceHeap',
        'Sampler',
        'Shader',
        'Surface',
        'SwapChain',
        'Texture',
        'Window'
    ]
    includes = {
        '<LLGL-C/Types.h>'
    }
    copyright = [
        'Copyright (c) 2015 Lukas Hermanns. All rights reserved.',
        'Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).'
    ]
    info = [
        'AUTO GENERATED CODE - DO NOT EDIT'
    ]
    funcPrefix = 'llgl'
    typePrefix = 'LLGL'

class LLGLMacros:
    def translateArraySize(ident):
        if ident == 'LLGL_MAX_NUM_COLOR_ATTACHMENTS':
            return 8
        else:
            return int(ident)

class LLGLType:
    baseType = StdType.UNDEFINED
    typename = '' # E.g. "Format" or "BufferDescriptor"
    arraySize = 0 # 0 for non-array, -1 for dynamic array, anything else for fixed size array
    isConst = False
    isPointer = False
    externalCond = None # Conditional expression string for external typenames (see LLGLMeta.externals)

    DYNAMIC_ARRAY = -1

    def __init__(self, typename = '', isConst = False, isPointer = False):
        self.baseType = LLGLType.toBaseType(typename)
        self.typename = typename
        self.arraySize = 0
        self.isConst = isConst
        self.isPointer = isPointer
        self.externalCond = next((external.cond for external in LLGLMeta.externals if external.name == typename), None)

    def setArraySize(self, arraySize):
        if isinstance(arraySize, str):
            self.arraySize = LLGLMacros.translateArraySize(arraySize)
        else:
            self.arraySize = arraySize

    def __str__(self):
        s = self.typename
        if self.arraySize > 0:
            s += '[{}]'.format(self.arraySize)
        elif self.arraySize == -1:
            s += '[]'
        if self.isPointer:
            s += '*'
        if self.isConst:
            s += '+'
        return s

    def toBaseType(typename):
        if typename != '':
            builtin = LLGLMeta.builtins.get(typename)
            return builtin if builtin else StdType.STRUCT
        return StdType.UNDEFINED

    # Returns true if this type is a custom LLGL enum, flags, or struct declaration
    def isCustomType(self):
        return self.baseType == StdType.STRUCT and not self.typename in ([LLGLMeta.UTF8STRING, LLGLMeta.STRING] + LLGLMeta.containers)

    # Returns true if this type is an LLGL interface type such as PipelineState
    def isInterface(self):
        return self.baseType == StdType.STRUCT and self.typename in LLGLMeta.interfaces

    def isDynamicArray(self):
        return self.arraySize == LLGLType.DYNAMIC_ARRAY

    def isPointerOrString(self):
        return self.isPointer or self.typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING]

    def getFixedBitsize(self):
        if self.baseType in [StdType.INT8, StdType.UINT8]:
            return 8
        elif self.baseType in [StdType.INT16, StdType.UINT16]:
            return 16
        elif self.baseType in [StdType.INT32, StdType.UINT32]:
            return 32
        elif self.baseType in [StdType.INT64, StdType.UINT64]:
            return 64
        return 0

class LLGLField:
    name = ''
    type = LLGLType()
    init = None

    def __init__(self, inName, inType = LLGLType()):
        self.name = inName
        self.type = inType
        self.init = None

    def __str__(self):
        s = ''
        if self.type.baseType != StdType.UNDEFINED:
            s += f'{self.name}:{self.type}'
        else:
            s += f'{self.name}'
        if self.init:
            s += f'({self.init})'
        return s

class LLGLRecord:
    name = ''
    base = None
    fields = []
    deps = set() # Set of record names this record depends on

    def __init__(self, name):
        self.name = name
        self.base = None
        self.fields = []
        self.deps = set()

    def hasConstFieldsOnly(self):
        for field in self.fields:
            if field.type.baseType != StdType.CONST:
                return False
        return True

    # Returns set of struct names that this record depends on
    def deriveDependencies(self):
        for field in self.fields:
            if field.type.isCustomType() and not field.type.isInterface() and field.type.typename != self.name:
                self.deps.add(field.type.typename)

class LLGLFunction:
    returnType = LLGLType()
    name = ''
    params = [] # Array of LLGLField

    def __init__(self, name, returnType = LLGLType()):
        self.returnType = returnType
        self.name = name
        self.params = []

class LLGLModule:
    name = ''
    enums = [] # Array of LLGLRecord
    flags = [] # Array of LLGLRecord
    structs = [] # Array of LLGLRecord
    funcs = [] # Array of LLGLFunction
    typeDeps = set() # Set of types used in this header

    def __init__(self):
        self.name = ''
        self.enums = []
        self.flags = []
        self.structs = []
        self.funcs = []
        self.typeDeps = set()

    def deriveDependencies(self):
        for struct in self.structs:
            for field in struct.fields:
                self.typeDeps.add(field.type)

    def merge(self, other):
        self.enums.extend(other.enums)
        self.flags.extend(other.flags)
        self.structs.extend(other.structs)
        self.funcs.extend(other.funcs)
        self.typeDeps.update(other.typeDeps)

    def findStructByName(self, name):
        for struct in self.structs:
            if struct.name == name:
                return struct
        return None

    def sortStructsByDependencies(self):
        # Derive dependencies for all structs
        for struct in self.structs:
            struct.deriveDependencies()

        # Start with structs that have no dependencies
        knownTypenames = set(external.name for external in LLGLMeta.externals)
        baseTypenames = set(enum.name for enum in self.enums) | set(flag.name for flag in self.flags) | knownTypenames
        sortedStructs = []
        pendingStructs = []

        for struct in self.structs:
            if len(struct.deps) == 0:
                sortedStructs.append(struct)
            else:
                pendingStructs.append(struct)

        # Continue with remaining structs
        while len(pendingStructs) > 0:
            wasAnyPendingStructSorted = False
            pendingStructIndex = 0
            declaredTypenames = set(struct.name for struct in sortedStructs) | baseTypenames

            while pendingStructIndex < len(pendingStructs):
                struct = pendingStructs[pendingStructIndex]
                if struct.deps.issubset(declaredTypenames):
                    sortedStructs.append(struct)
                    pendingStructs.pop(pendingStructIndex)
                    wasAnyPendingStructSorted = True
                else:
                    pendingStructIndex += 1
            if not wasAnyPendingStructSorted:
                def printCyclicDependencies(struct, typenames):
                    print(f"Cyclic dependency in struct '{struct.name}':")
                    for dep in struct.deps:
                        if not dep in typenames:
                            print(f" ==> Missing '{dep}'")

                printCyclicDependencies(pendingStructs[0], declaredTypenames)
                fatal('error: failed to resolve dependencies')

        return sortedStructs

def scanTokens(filename):
    def preprocessSource(text):
        def removeRange(text, start, end):
            pos = 0
            while True:
                pos = text.find(start, pos)
                if pos >= 0:
                    posEnd = text.find(end, pos + len(start))
                    if posEnd >= pos:
                        blanks = ' ' * (posEnd + len(end) - pos)
                        text = text[:pos] + blanks + text[posEnd if end == '\n' else posEnd + len(end):]
                    else:
                        pos += 1
                else:
                    break
            return text

        # Remove comments and preprocessor directives
        text = removeRange(text, '#', '\n')

        text = removeRange(text, '//', '\n') #TMP
        text = removeRange(text, '/*', '*/') #TMP

        # Replace multi-line comments with single line comments
        """
        def convertComments(text, start, end, filler):
            assert len(filler) == len(start)
            assert len(filler) == len(end)
            pos = 0
            while True:
                pos = text.find(start, pos)
                if pos >= 0:
                    posEnd = text.find(end, pos + len(start))
                    if posEnd >= pos:
                        text = text[:pos] + filler + text[pos + len(start):]
                        while True:
                            lineEnd = text.find('\n', pos + len(start))
                            if lineEnd >= pos and lineEnd < posEnd:
                                text = text[:lineEnd] + filler + text[lineEnd:]
                                pos = lineEnd
                            else:
                                text = text[:posEnd] + filler + text[posEnd + len(end):]
                                pos = posEnd
                                break
                    else:
                        pos += 1
                else:
                    break
            return text

        #text = convertComments(text, '/*', '*/', '//')
        """

        return text
    
    # Scan tokens from source file
    try:
        with open(filename, 'r') as file:
            text = preprocessSource(file.read())

            #print(text) # ~~~~~~~~~~~~ TEST ~~~~~~~~~~~~
            #sys.exit(0)

            #text = file.read()
            return re.findall(r'//[^\n]*|[a-zA-Z_]\w*|\d+\.\d+[fF]|\d+[uU]|\d+|[{}\[\]]|::|:|<<|>>|[+-=,;<>\|]|[*]|[(]|[)]', text)
    except UnicodeDecodeError:
        fatal('UnicodeDecodeError exception while reading file: ' + filename)
    return None

def reduceTokens(tokens):
    reduced = []
    tok = 0
    while tok < len(tokens):
        if tokens[tok] in ['std', 'LLGL'] and tok + 1 < len(tokens) and tokens[tok + 1] == '::':
            tok += 2 # Ignore std:: and LLGL:: namespace resolutions
        elif tokens[tok] in ['inline']:
            tok += 1 # Ignore keywords: inline
        else:
            reduced.append(tokens[tok])
            tok += 1
    return reduced

class Scanner:
    filename = ''
    tokens = []
    readPos = 0

    def __init__(self):
        self.filename = ''
        self.tokens = []
        self.readPos = 0

    def good(self):
        return self.readPos < len(self.tokens)

    def scan(self, filename):
        self.filename = filename
        self.tokens = reduceTokens(scanTokens(filename))

        #iterate(print, self.tokens) # ~~~~~~~~~~~~ TEST ~~~~~~~~~~~~
        #sys.exit(0)
    
    def tok(self, lookAhead = 0):
        return self.tokens[self.readPos + lookAhead] if self.readPos + lookAhead < len(self.tokens) else ''
    
    def accept(self, count = 1):
        tok = self.tok()
        self.readPos += count
        return tok
    
    def match(self, search, equality=True):
        if (self.tok() == search) == equality:
            return 1
        elif hasattr(search, '__len__'):
            filterIndex = 0
            while filterIndex < len(search):
                if not ((self.tok(filterIndex) == search[filterIndex]) == equality):
                    return 0
                filterIndex += 1
            return filterIndex
        return 0

    def acceptIf(self, search):
        count = self.match(search)
        if count > 0:
            self.accept(count)
            return True
        return False

    def acceptIfNot(self, search):
        count = self.match(search, equality=False)
        if count > 0:
            self.accept(count)
            return True
        return False

    def acceptOrFail(self, search):
        if not self.acceptIf(search):
            fatal(f"{self.filename}: error: expected token '{search}', but got '{self.tok()}'; predecessors: {self.tokens[self.readPos - 5:self.readPos]}")

    def ignoreUntil(self, search):
        while self.good():
            if self.acceptIf(search):
                break
            self.accept()

class Parser:
    scanner = None

    def __init__(self):
        self.scanner = Scanner()

    def parseInitializer(self):
        value = ''
        if self.scanner.acceptIf('{'):
            value += '{'
            while not self.scanner.match('}'):
                value += self.parseInitializer()
                if self.scanner.match(','):
                    value += self.scanner.accept()
                else:
                    break
            self.scanner.acceptOrFail('}')
            value += '}'
        else:
            while not self.scanner.tok() in [',', ';', '}']:
                value += self.scanner.accept()
        return value

    def parseEnumEntries(self):
        entries = []
        while self.scanner.tok() != '}':
            entry = LLGLField(self.scanner.accept())
            if self.scanner.acceptIf('='):
                entry.init = self.parseInitializer()
            entries.append(entry)
            if not self.scanner.acceptIf(','):
                break
        return entries

    def parseType(self):
        if self.scanner.acceptIf(['static', 'constexpr', 'int']):
            return LLGLType('const')
        else:
            isConst = self.scanner.acceptIf('const')
            typename = self.scanner.accept()
            isConst = self.scanner.acceptIf('const') or isConst
            if typename in LLGLMeta.containers and self.scanner.acceptIf('<'):
                isConst = self.scanner.acceptIf('const') or isConst
                typename = self.scanner.accept()
                isPointer = self.scanner.acceptIf('*')
                self.scanner.acceptOrFail('>')
                outType = LLGLType(typename, isConst, isPointer)
                outType.setArraySize(LLGLType.DYNAMIC_ARRAY)
                return outType
            else:
                isPointer = self.scanner.acceptIf('*')
                return LLGLType(typename, isConst, isPointer)

    def parseStructMembers(self, structName):
        members = []
        while self.scanner.tok() != '}':
            fieldType = self.parseType()
            isCtor = fieldType.typename == structName
            isOper = self.scanner.tok() == 'operator'
            isFunc = self.scanner.tok(1) == '('
            if isCtor or isOper or isFunc:
                # Ignore operators
                if isOper:
                    self.scanner.accept(2)
                elif isFunc:
                    self.scanner.accept()

                # Ingore constructs
                self.scanner.acceptOrFail('(')
                self.scanner.ignoreUntil(')')
                if self.scanner.acceptIf(':'):
                    # Ignore initializer list
                    while self.scanner.good():
                        self.scanner.accept() # Member
                        self.scanner.acceptOrFail('{')
                        self.scanner.ignoreUntil('}')
                        if not self.scanner.acceptIf(','):
                            break

                    # Ignore c'tor body
                    self.scanner.acceptOrFail('{')
                    self.scanner.ignoreUntil('}')
                else:
                    # Ignore tokens until end of declaration ';', e.g. 'Ctor();' or 'Ctor() = default;'
                    self.scanner.ignoreUntil(';')
            else:
                member = LLGLField(self.scanner.accept())
                member.type = fieldType
                if self.scanner.acceptIf('['):
                    member.type.setArraySize(self.scanner.accept())
                    self.scanner.acceptOrFail(']')
                if self.scanner.acceptIf('='):
                    member.init = self.parseInitializer()
                members.append(member)
                self.scanner.acceptOrFail(';')
        return members

    def parseParameter(self):
        # Only parse return type name parameter name as C does not support default arguments
        paramType = self.parseType()
        param = LLGLField(self.scanner.accept(), paramType)

        # Parse optional fixed size array
        if self.scanner.acceptIf('['):
            param.type.setArraySize(self.scanner.accept())
            self.scanner.acceptOrFail(']')

        self.scanner.acceptIf('LLGL_NULLABLE')

        return param

    def parseFunctionDecl(self):
        # Parse return type
        returnType = self.parseType()

        # Parse function name
        name = self.scanner.accept()

        # Parse parameter list
        func = LLGLFunction(name, returnType)

        self.scanner.acceptOrFail('(')
        if not self.scanner.match(')'):
            if self.scanner.match(['void', ')']):
                # Ignore explicit empty parameter list
                self.scanner.accept()
            else:
                # Parse parameters until no more ',' is scanned
                while True:
                    func.params.append(self.parseParameter())
                    if not self.scanner.acceptIf(','):
                        break
        self.scanner.acceptOrFail(')')
        self.scanner.acceptOrFail(';')

        return func

    # Parses input file by filename and returns LLGLModule
    def parseHeader(self, filename, processFunctions = False):
        mod = LLGLModule()
        mod.name = os.path.splitext(os.path.basename(filename))[0]

        self.scanner.scan(filename)

        while self.scanner.good():
            if processFunctions and self.scanner.acceptIf('LLGL_C_EXPORT'):
                # Parse function declaration
                mod.funcs.append(self.parseFunctionDecl())
            elif self.scanner.acceptIf(['enum', 'class']):
                # Parse enumeration
                name = self.scanner.accept()
                enum = LLGLRecord(name)
                if self.scanner.acceptIf(':'):
                    enum.base = self.parseType()
                self.scanner.acceptOrFail('{')
                enum.fields = self.parseEnumEntries()
                self.scanner.acceptOrFail('}')
                mod.enums.append(enum)
            elif self.scanner.acceptIf('struct'):
                self.scanner.acceptIf('LLGL_EXPORT')

                # Ignore deprecated records
                ignoreRecord = False
                if self.scanner.acceptIf('LLGL_DEPRECATED'):
                    ignoreRecord = True
                    while self.scanner.accept() != ')':
                        pass
                
                # Parse record name
                name = self.scanner.accept()
                self.scanner.acceptOrFail('{')
                if self.scanner.acceptIf('enum'):
                    # Parse flags
                    flag = LLGLRecord(name)
                    if self.scanner.acceptIf(':'):
                        flag.base = self.parseType()
                    self.scanner.acceptOrFail('{')
                    flag.fields = self.parseEnumEntries()
                    if not ignoreRecord:
                        mod.flags.append(flag)
                else:
                    # Parse structure
                    struct = LLGLRecord(name)
                    struct.fields = self.parseStructMembers(name)
                    if not ignoreRecord:
                        mod.structs.append(struct)
                self.scanner.acceptOrFail('}')
            else:
                self.scanner.accept()

        return mod

def parseFile(filename, processFunctions = False):
    parser = Parser()
    mod = parser.parseHeader(filename, processFunctions)
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
    print('@END')

class Translator:
    indent = 0
    tabSize = 4

    class Declaration:
        type = ''
        name = ''
        init = None
        directive = None

        def __init__(self, inType = '', inName = '', inInit = None, inDirective = None):
            self.type = inType
            self.name = inName
            self.init = inInit
            self.directive = inDirective

    class DeclarationList:
        decls = []
        maxLen = [0, 0, 0]

        def __init__(self):
            self.decls = []
            self.maxLen = [0, 0, 0]

        def append(self, decl):
            self.decls.append(decl)
            if not decl.directive:
                self.maxLen[0] = max(self.maxLen[0], len(decl.type) if decl.type else 0)
                self.maxLen[1] = max(self.maxLen[1], len(decl.name))
                self.maxLen[2] = max(self.maxLen[2], len(decl.init) if decl.init else 0)

        def spaces(self, index, s):
            return ' ' * (self.maxLen[index] - len(s) + 1)

    def indentation(self):
        return ' ' * (self.indent * self.tabSize)

    def statement(self, line):
        if len(line) > 0 and line[0] == '#':
            print(line)
        else:
            print(self.indentation() + line)

    def openScope(self, stmt = '{'):
        self.statement(stmt)
        self.indent += 1

    def closeScope(self, stmt = '}'):
        self.indent -= 1
        self.statement(stmt)

    def convertNameToHeaderGuard(name):
        return re.sub(r'([A-Z]+)', r'_\1', name).upper()

    def translateModuleToC99(self, doc):
        def translateDependency(inType):
            if inType.baseType in [StdType.BOOL]:
                return '<stdbool.h>', True
            elif inType.baseType in [StdType.INT8, StdType.INT16, StdType.INT32, StdType.INT64, StdType.UINT8, StdType.UINT16, StdType.UINT32, StdType.UINT64]:
                return '<stdint.h>', True
            elif inType.baseType in [StdType.SIZE_T]:
                return '<stddef.h>', True
            elif inType.baseType in [StdType.CHAR, StdType.LONG, StdType.FLOAT]:
                return None, True
            return f'<LLGL-C/{inType.typename}Flags.h>', False

        def translateIncludes(typeDeps):
            stdIncludes = set()
            llglIncludes = LLGLMeta.includes.copy()
            for dep in typeDeps:
                inc = translateDependency(dep)
                if inc and inc[0]:
                    if inc[1]:
                        stdIncludes.add(inc[0])
                    #else:
                    #    llglIncludes.add(inc[0])
            return stdIncludes, llglIncludes

        self.statement('/*')
        self.statement(' * {}.h'.format(doc.name))
        self.statement(' *')
        for line in LLGLMeta.copyright:
            self.statement(' * ' + line)
        self.statement(' */')
        self.statement('')
        for line in LLGLMeta.info:
            self.statement('/* {} */'.format(line))
        self.statement('')

        # Write header guard
        headerGuardName = 'LLGL_C99{}_H'.format(Translator.convertNameToHeaderGuard(doc.name))
        self.statement('#ifndef ' + headerGuardName)
        self.statement('#define ' + headerGuardName)
        self.statement('')
        self.statement('')

        # Write all include directives
        includeHeaders = translateIncludes(doc.typeDeps)
        if len(includeHeaders[0]) > 0 or len(includeHeaders[1]) > 0:
            for headers in includeHeaders:
                for inc in headers:
                    self.statement('#include {}'.format(inc))

            for external in LLGLMeta.externals:
                if external.cond and external.include:
                    self.statement('')
                    self.statement(f'#if {external.cond}')
                    self.statement(f'#   include {external.include}')
                    self.statement(f'#endif /* {external.cond} */')

            self.statement('')
            self.statement('')

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement('')

            for struct in constStructs:
                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    declList.append(Translator.Declaration('', 'LLGL_{}_{}'.format(struct.name.upper(), field.name.upper()), field.init))

                for decl in declList.decls:
                    self.statement('#define ' + decl.name + declList.spaces(1, decl.name) + ' ( ' + decl.init + ' )')
                self.statement('')

            self.statement('')

        # Write all enumerations
        sizedTypes = dict()

        if len(doc.enums) > 0:
            self.statement('/* ----- Enumerations ----- */')
            self.statement('')

            for enum in doc.enums:
                if enum.base:
                    bitsize = enum.base.getFixedBitsize()
                    if bitsize > 0:
                        sizedTypes[enum.name] = bitsize

                self.statement('typedef enum LLGL{}'.format(enum.name))
                self.openScope()

                # Write enumeration entry declarations
                declList = Translator.DeclarationList()
                for field in enum.fields:
                    declList.append(Translator.Declaration('', 'LLGL{}{}'.format(enum.name, field.name), field.init))

                for decl in declList.decls:
                    if decl.init:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('LLGL{};'.format(enum.name))
                self.statement('')

            self.statement('')

        # Write all flags
        if len(doc.flags) > 0:
            def translateFlagInitializer(basename, init):
                s = init
                s = re.sub(r'([a-zA-Z_]\w*)', 'LLGL{}{}'.format(basename, r'\1'), s)
                s = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', s)
                return s

            def translateFieldName(name):
                exceptions = [
                    ('LLGLCPUAccessReadWrite', None) # Identifier for LLGL::CPUAccessFlags::ReadWrite is already used for LLGL::CPUAccess::ReadWrite
                ]
                for exception in exceptions:
                    if name == exception[0]:
                        return exception[1]
                return name


            self.statement('/* ----- Flags ----- */')
            self.statement('')

            for flag in doc.flags:
                self.statement('typedef enum LLGL{}'.format(flag.name))
                basename = flag.name[:-len('Flags')]
                self.openScope()

                # Write flag entry declarations
                declList = Translator.DeclarationList()
                for field in flag.fields:
                    fieldName = translateFieldName(f'LLGL{basename}{field.name}')
                    if fieldName:
                        declList.append(Translator.Declaration('', fieldName, translateFlagInitializer(basename, field.init) if field.init else None))

                for decl in declList.decls:
                    if decl.init:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('LLGL{};'.format(flag.name))
                self.statement('')

            self.statement('')

        # Write all structures
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        if len(commonStructs) > 0:
            def translateStructField(fieldType, name):
                nonlocal sizedTypes
                typeStr = ''
                declStr = ''

                # Write type specifier
                if fieldType.isDynamicArray() and not fieldType.isPointerOrString():
                    typeStr += 'const '

                if fieldType.typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING]:
                    typeStr += 'const char*'
                elif fieldType.baseType == StdType.STRUCT and fieldType.typename in LLGLMeta.interfaces:
                    typeStr += 'LLGL' + fieldType.typename
                else:
                    if fieldType.isConst:
                        typeStr += 'const '
                    if fieldType.baseType == StdType.STRUCT and not fieldType.externalCond:
                        typeStr += 'LLGL'
                    typeStr += fieldType.typename
                    if fieldType.isPointer:
                        typeStr += '*'
                
                if fieldType.isDynamicArray():
                    typeStr += ' const*' if fieldType.isPointerOrString() else '*'

                # Write field name
                declStr += name

                # Write optional bit size for enumerations with underlying type (C does not support explicit underlying enum types)
                bitsize = sizedTypes.get(fieldType.typename)
                if bitsize:
                    declStr += f' : {bitsize}'

                # Write fixed size array dimension
                if fieldType.arraySize > 0:
                    declStr += f'[{fieldType.arraySize}]'

                return (typeStr, declStr)

            def translateFieldInitializer(fieldType, init):
                if fieldType.isDynamicArray():
                    return 'NULL'
                if init:
                    if init == 'nullptr':
                        return 'LLGL_NULL_OBJECT' if fieldType.isInterface() else 'NULL'
                    else:
                        return re.sub(r'(\w+::)', r'LLGL\1', init).replace('::', '').replace('|', ' | ').replace('Flags', '')
                return None

            self.statement('/* ----- Structures ----- */')
            self.statement('')

            for struct in commonStructs:
                self.statement('typedef struct LLGL{}'.format(struct.name))
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    # Write two fields for dynamic arrays
                    externalCond = field.type.externalCond
                    if externalCond:
                        declList.append(Translator.Declaration(directive = f'#if {externalCond}'))
                    if field.type.isDynamicArray():
                        declList.append(Translator.Declaration('size_t', f'num{field.name[0].upper()}{field.name[1:]}', '0'))
                    declStr = translateStructField(field.type, field.name)
                    declList.append(Translator.Declaration(declStr[0], declStr[1], translateFieldInitializer(field.type, field.init)))
                    if externalCond:
                        declList.append(Translator.Declaration(directive = f'#endif /* {externalCond} */'))

                for decl in declList.decls:
                    if decl.directive:
                        self.statement(decl.directive)
                    elif decl.init:
                        self.statement(f'{decl.type}{declList.spaces(0, decl.type)}{decl.name};{declList.spaces(1, decl.name)}/* = {decl.init} */')
                    else:
                        self.statement(f'{decl.type}{declList.spaces(0, decl.type)}{decl.name};')
                self.closeScope()
                self.statement('LLGL{};'.format(struct.name))
                self.statement('')

            self.statement('')

        self.statement('#endif /* {} */'.format(headerGuardName))
        self.statement('')
        self.statement('')
        self.statement('')
        self.statement('/* ================================================================================ */')
        self.statement('')

    def translateModuleToCsharp(self, doc):
        builtinTypenames = {
            StdType.VOID: 'void',
            StdType.BOOL: 'bool',
            StdType.CHAR: 'byte',
            StdType.INT8: 'sbyte',
            StdType.INT16: 'short',
            StdType.INT32: 'int',
            StdType.INT64: 'long',
            StdType.UINT8: 'byte',
            StdType.UINT16: 'ushort',
            StdType.UINT32: 'uint',
            StdType.UINT64: 'ulong',
            StdType.LONG: 'long',
            StdType.SIZE_T: 'UIntPtr',
            StdType.FLOAT: 'float'
        }

        self.statement('/*')
        self.statement(' * {}.cs'.format(doc.name))
        self.statement(' *')
        for line in LLGLMeta.copyright:
            self.statement(' * ' + line)
        self.statement(' */')
        self.statement('')
        for line in LLGLMeta.info:
            self.statement('/* {} */'.format(line))
        self.statement('')
        self.statement('using System;')
        self.statement('using System.Runtime.InteropServices;')
        self.statement('')
        self.statement('namespace LLGLModule')
        self.openScope()
        self.statement('public static partial class LLGL')
        self.openScope()

        # Write DLL name
        self.statement('#if DEBUG')
        self.statement('const string DllName = "LLGLD.dll";')
        self.statement('#else')
        self.statement('const string DllName = "LLGL.dll";')
        self.statement('#endif')
        self.statement('')

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement('')

            for struct in constStructs:
                self.statement('public enum {} : int'.format(struct.name))
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    declList.append(Translator.Declaration('', field.name, field.init))

                for decl in declList.decls:
                    self.statement(decl.name + declList.spaces(1, decl.name) + ' = ' + decl.init + ',')

                self.closeScope()
                self.statement('')

            self.statement('')

        # Write all interface handles
        self.statement('/* ----- Handles ----- */')
        self.statement('')

        for interface in LLGLMeta.interfaces:
            self.statement(f'public unsafe struct {interface}')
            self.openScope()
            self.statement('internal unsafe void* ptr;')
            self.closeScope()
            self.statement('')

        self.statement('')

        # Write all enumerations
        if len(doc.enums) > 0:
            self.statement('/* ----- Enumerations ----- */')
            self.statement('')

            for enum in doc.enums:
                self.statement('public enum ' + enum.name)
                self.openScope()

                # Write enumeration entry declarations
                declList = Translator.DeclarationList()
                for field in enum.fields:
                    declList.append(Translator.Declaration('', field.name, field.init))

                for decl in declList.decls:
                    if decl.init:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('')

            self.statement('')

        # Write all flags
        if len(doc.flags) > 0:
            def translateFlagInitializer(init):
                s = init
                s = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', s)
                return s

            self.statement('/* ----- Flags ----- */')
            self.statement('')

            for flag in doc.flags:
                self.statement('[Flags]')
                self.statement('public enum {} : uint'.format(flag.name))
                #basename = flag.name[:-len('Flags')]
                self.openScope()

                # Write flag entry declarations
                declList = Translator.DeclarationList()
                for field in flag.fields:
                    declList.append(Translator.Declaration('', field.name, translateFlagInitializer(field.init) if field.init else None))

                for decl in declList.decls:
                    if decl.init:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('')

            self.statement('')

        # Write all structures
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        class CsharpDeclaration:
            marshal = None
            type = ''
            ident = ''

            def __init__(self, ident):
                self.marshal = None
                self.type = ''
                self.ident = ident

        def translateDecl(declType, ident = None, isInsideStruct = False):
            decl = CsharpDeclaration(ident)

            def sanitizeTypename(typename):
                if typename.startswith(LLGLMeta.typePrefix):
                    return typename[len(LLGLMeta.typePrefix):]
                elif typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING]:
                    return 'string'
                else:
                    return typename

            nonlocal builtinTypenames

            if declType.baseType == StdType.STRUCT and declType.typename in LLGLMeta.interfaces:
                decl.type = sanitizeTypename(declType.typename)
            else:
                builtin = builtinTypenames.get(declType.baseType)
                if isInsideStruct:
                    if declType.arraySize > 0 and builtin:
                        decl.type += 'fixed '
                    decl.type += builtin if builtin else sanitizeTypename(declType.typename)
                    if declType.isPointer:
                        decl.type += '*'
                    elif declType.arraySize > 0:
                        if builtin:
                            decl.ident += f'[{declType.arraySize}]'
                        else:
                            decl.marshal = f'MarshalAs(UnmanagedType.ByValArray, SizeConst = {declType.arraySize})'
                            decl.type += '[]'
                else:
                    decl.type += builtin if builtin else sanitizeTypename(declType.typename)
                    if declType.isPointer or declType.arraySize > 0:
                        decl.type += '*'

            return decl

        if len(commonStructs) > 0:
            self.statement('/* ----- Structures ----- */')
            self.statement('')

            for struct in commonStructs:
                self.statement('public unsafe struct ' + struct.name)
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    if not field.type.externalCond:
                        # Write two fields for dynamic arrays
                        if field.type.arraySize == -1:
                            declList.append(Translator.Declaration('UIntPtr', 'num{}{}'.format(field.name[0].upper(), field.name[1:])))
                        fieldDecl = translateDecl(field.type, field.name, isInsideStruct = True)
                        if fieldDecl.marshal:
                            declList.append(Translator.Declaration(None, fieldDecl.marshal))
                        declList.append(Translator.Declaration(fieldDecl.type, fieldDecl.ident, field.init))

                for decl in declList.decls:
                    if not decl.type:
                        self.statement(f'[{decl.name}]')
                    elif decl.init:
                        self.statement(f'public {decl.type}{declList.spaces(0, decl.type)}{decl.name};{declList.spaces(1, decl.name)}/* = {decl.init} */')
                    else:
                        self.statement(f'public {decl.type}{declList.spaces(0, decl.type)}{decl.name};')
                self.closeScope()
                self.statement('')

            self.statement('')

        # Write all functions
        if len(doc.funcs) > 0:
            self.statement('/* ----- Functions ----- */')
            self.statement('')

            for func in doc.funcs:
                self.statement(f'[DllImport(DllName, EntryPoint="{func.name}", CallingConvention=CallingConvention.Cdecl)]');
                returnTypeStr = translateDecl(func.returnType).type
                paramListStr = ''
                for param in func.params:
                    if len(paramListStr) > 0:
                        paramListStr += ', '
                    paramDecl = translateDecl(param.type, param.name)
                    paramListStr += f'{paramDecl.type} {paramDecl.ident}'
                funcName = func.name[len(LLGLMeta.funcPrefix):]
                self.statement(f'public static extern unsafe {returnTypeStr} {funcName}({paramListStr});');
                self.statement('')

            self.statement('')

        self.closeScope()
        self.closeScope()
        self.statement('')
        self.statement('')
        self.statement('')
        self.statement('')
        self.statement('// ================================================================================')

def main():
    args = sys.argv[1:]
    translator = Translator()
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
            iterate(translator.translateModuleToC99, modules)
        elif '-csharp' in args:
            iterate(translator.translateModuleToCsharp, modules)
        else:
            iterate(printModule, modules)
    else:
        printHelp()

main()
