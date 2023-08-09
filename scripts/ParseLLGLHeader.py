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
    builtins = [
        ConditionalType('android_app', 'defined LLGL_OS_ANDROID', '<android_native_app_glue.h>')
    ]
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
    builtinCond = None # Conditional expression string for builtin typenames (see LLGLMeta.builtins)

    DYNAMIC_ARRAY = -1

    def __init__(self, typename = '', isConst = False, isPointer = False):
        self.baseType = LLGLType.toBaseType(typename)
        self.typename = typename
        self.arraySize = 0
        self.isConst = isConst
        self.isPointer = isPointer
        self.builtinCond = next((builtin.cond for builtin in LLGLMeta.builtins if builtin.name == typename), None)

    def setArraySize(self, arraySize):
        if isinstance(arraySize, str):
            self.arraySize = LLGLMacros.translateArraySize(arraySize)
        else:
            self.arraySize = arraySize

    def __str__(self):
        str = self.typename
        if self.arraySize > 0:
            str += '[{}]'.format(self.arraySize)
        elif self.arraySize == -1:
            str += '[]'
        if self.isPointer:
            str += '*'
        if self.isConst:
            str += '+'
        return str

    def toBaseType(typename):
        if typename != '':
            if typename == 'void':
                return StdType.VOID
            elif typename == 'bool':
                return StdType.BOOL
            elif typename == 'char':
                return StdType.CHAR
            elif typename == 'int8_t':
                return StdType.INT8
            elif typename in ['int16_t', 'short']:
                return StdType.INT16
            elif typename in ['int32_t', 'int']:
                return StdType.INT32
            elif typename == 'int64_t':
                return StdType.INT64
            elif typename == 'uint8_t':
                return StdType.UINT8
            elif typename == 'uint16_t':
                return StdType.UINT16
            elif typename == 'uint32_t':
                return StdType.UINT32
            elif typename == 'uint64_t':
                return StdType.UINT64
            elif typename == 'long':
                return StdType.LONG
            elif typename == 'size_t':
                return StdType.SIZE_T
            elif typename == 'float':
                return StdType.FLOAT
            elif typename == 'const':
                return StdType.CONST
            else:
                return StdType.STRUCT
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
        elif type.baseType in [StdType.INT16, StdType.UINT16]:
            return 16
        elif type.baseType in [StdType.INT32, StdType.UINT32]:
            return 32
        elif type.baseType in [StdType.INT64, StdType.UINT64]:
            return 64
        return 0

class LLGLField:
    name = ''
    type = LLGLType()
    init = None

    def __init__(self, name):
        self.name = name

    def __str__(self):
        str = ''
        if self.type.baseType != StdType.UNDEFINED:
            str += '{}:{}'.format(self.name, self.type)
        else:
            str += '{}'.format(self.name)
        if self.init != None:
            str += '({})'.format(self.init)
        return str

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

class LLGLModule:
    name = ''
    enums = []
    flags = []
    structs = []
    typeDeps = set() # Set of types used in this header

    def __init__(self):
        self.name = ''
        self.enums = []
        self.flags = []
        self.structs = []
        self.typeDeps = set()

    def deriveDependencies(self):
        for struct in self.structs:
            for field in struct.fields:
                self.typeDeps.add(field.type)

    def merge(self, other):
        self.enums.extend(other.enums)
        self.flags.extend(other.flags)
        self.structs.extend(other.structs)
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
        knownTypenames = set(builtin.name for builtin in LLGLMeta.builtins)
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
    
    def match(self, filter, equality=True):
        if (self.tok() == filter) == equality:
            return 1
        elif hasattr(filter, '__len__'):
            filterIndex = 0
            while filterIndex < len(filter):
                if not ((self.tok(filterIndex) == filter[filterIndex]) == equality):
                    return 0
                filterIndex += 1
            return filterIndex
        return 0

    def acceptIf(self, filter):
        count = self.match(filter)
        if count > 0:
            self.accept(count)
            return True
        return False

    def acceptIfNot(self, match):
        count = self.match(filter, equality=False)
        if count > 0:
            self.accept(count)
            return True
        return False

    def acceptOrFail(self, match):
        if not self.acceptIf(match):
            fatal(f"{self.filename}: error: expected token '{match}', but got '{self.tok()}'; predecessors: {self.tokens[self.readPos - 5:self.readPos]}")

    def ignoreUntil(self, filter):
        while self.good():
            if self.acceptIf(filter):
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
            if typename in LLGLMeta.containers and self.scanner.acceptIf('<'):
                isConst = self.scanner.acceptIf('const')
                typename = self.scanner.accept()
                isPointer = self.scanner.acceptIf('*')
                self.scanner.acceptOrFail('>')
                type = LLGLType(typename, isConst, isPointer)
                type.setArraySize(LLGLType.DYNAMIC_ARRAY)
                return type
            else:
                isPointer = self.scanner.acceptIf('*')
                type = LLGLType(typename, isConst, isPointer)
                return type

    def parseStructMembers(self, structName):
        members = []
        while self.scanner.tok() != '}':
            type = self.parseType()
            isCtor = type.typename == structName
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
                member.type = type
                if self.scanner.acceptIf('['):
                    member.type.setArraySize(self.scanner.accept())
                    self.scanner.acceptOrFail(']')
                if self.scanner.acceptIf('='):
                    member.init = self.parseInitializer()
                members.append(member)
                self.scanner.acceptOrFail(';')
        return members

    # Parses input file by filename and returns LLGLModule
    def parseHeader(self, filename):
        mod = LLGLModule()
        mod.name = os.path.splitext(os.path.basename(filename))[0]

        self.scanner.scan(filename)

        while self.scanner.good():
            if self.scanner.acceptIf(['enum', 'class']):
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
                name = self.scanner.accept()
                self.scanner.acceptOrFail('{')
                if self.scanner.acceptIf('enum'):
                    # Parse flags
                    flag = LLGLRecord(name)
                    if self.scanner.acceptIf(':'):
                        flag.base = self.parseType()
                    self.scanner.acceptOrFail('{')
                    flag.fields = self.parseEnumEntries()
                    mod.flags.append(flag)
                else:
                    # Parse structure
                    struct = LLGLRecord(name)
                    struct.fields = self.parseStructMembers(name)
                    mod.structs.append(struct)
                self.scanner.acceptOrFail('}')
            else:
                self.scanner.accept()

        return mod

def parseFile(filename):
    parser = Parser()
    mod = parser.parseHeader(filename)
    mod.deriveDependencies()
    return mod

def printModule(module):
    def printField(field):
        print('@FIELD{' + str(field) + '}')

    def printRecord(record, type):
        print('@' + type + '{' + record.name + '}')
        iterate(printField, record.fields)
        print('@END')

    print('@HEADER{' + module.name + '}')
    iterate(lambda record: printRecord(record, 'CONST'), filter(lambda record: record.hasConstFieldsOnly(), module.structs))
    iterate(lambda record: printRecord(record, 'ENUM'), module.enums)
    iterate(lambda record: printRecord(record, 'FLAG'), module.flags)
    iterate(lambda record: printRecord(record, 'STRUCT'), filter(lambda record: not record.hasConstFieldsOnly(), module.structs))
    print('@END')

class Translator:
    indent = 0
    tabSize = 4

    class Declaration:
        type = ''
        name = ''
        init = None
        directive = None

        def __init__(self, type = '', name = '', init = None, directive = None):
            self.type = type
            self.name = name
            self.init = init
            self.directive = directive

    class DeclarationList:
        decls = []
        maxLen = [0, 0, 0]

        def __init__(self):
            self.decls = []
            self.maxLen = [0, 0, 0]

        def append(self, decl):
            self.decls.append(decl)
            if not decl.directive:
                self.maxLen[0] = max(self.maxLen[0], len(decl.type))
                self.maxLen[1] = max(self.maxLen[1], len(decl.name))
                self.maxLen[2] = max(self.maxLen[2], len(decl.init) if decl.init != None else 0)

        def spaces(self, index, str):
            return ' ' * (self.maxLen[index] - len(str) + 1)

    def indentation(self):
        return ' ' * (self.indent * self.tabSize)

    def statement(self, line):
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
        def translateDependency(type):
            if type.baseType in [StdType.BOOL]:
                return '<stdbool.h>', True
            elif type.baseType in [StdType.INT8, StdType.INT16, StdType.INT32, StdType.INT64, StdType.UINT8, StdType.UINT16, StdType.UINT32, StdType.UINT64]:
                return '<stdint.h>', True
            elif type.baseType in [StdType.SIZE_T]:
                return '<stddef.h>', True
            elif type.baseType in [StdType.CHAR, StdType.LONG, StdType.FLOAT]:
                return None, True
            return f'<LLGL-C/{type.typename}Flags.h>', False

        def translateIncludes(typeDeps):
            stdIncludes = set()
            llglIncludes = LLGLMeta.includes.copy()
            for dep in typeDeps:
                inc = translateDependency(dep)
                if inc and inc[0] != None:
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
            for i in range(0, len(includeHeaders)):
                for inc in includeHeaders[i]:
                    self.statement('#include {}'.format(inc))

            for builtin in LLGLMeta.builtins:
                if builtin.cond and builtin.include:
                    self.statement('')
                    self.statement(f'#if {builtin.cond}')
                    self.statement(f'#   include {builtin.include}')
                    self.statement(f'#endif /* {builtin.cond} */')

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
                    if decl.init != None:
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
                str = init
                str = re.sub(r'([a-zA-Z_]\w*)', 'LLGL{}{}'.format(basename, r'\1'), str)
                str = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', str)
                return str

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
                    if decl.init != None:
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
            def translateStructField(type, name):
                nonlocal sizedTypes
                typeStr = ''
                declStr = ''

                # Write type specifier
                if type.isDynamicArray() and not type.isPointerOrString():
                    typeStr += 'const '

                if type.typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING]:
                    typeStr += 'const char*'
                elif type.baseType == StdType.STRUCT and type.typename in LLGLMeta.interfaces:
                    typeStr += 'LLGL' + type.typename
                else:
                    if type.isConst:
                        typeStr += 'const '
                    if type.baseType == StdType.STRUCT and type.builtinCond == None:
                        typeStr += 'LLGL'
                    typeStr += type.typename
                    if type.isPointer:
                        typeStr += '*'
                
                if type.isDynamicArray():
                    typeStr += ' const*' if type.isPointerOrString() else '*'

                # Write field name
                declStr += name

                # Write optional bit size for enumerations with underlying type (C does not support explicit underlying enum types)
                bitsize = sizedTypes.get(type.typename)
                if bitsize:
                    declStr += f' : {bitsize}'

                # Write fixed size array dimension
                if type.arraySize > 0:
                    declStr += f'[{type.arraySize}]'

                return (typeStr, declStr)

            self.statement('/* ----- Structures ----- */')
            self.statement('')
            for struct in commonStructs:
                self.statement('typedef struct LLGL{}'.format(struct.name))
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    # Write two fields for dynamic arrays
                    builtinCond = field.type.builtinCond
                    if builtinCond:
                        declList.append(Translator.Declaration(directive = f'#if {builtinCond}'))
                    if field.type.isDynamicArray():
                        declList.append(Translator.Declaration('size_t', f'num{field.name[0].upper()}{field.name[1:]}'))
                    declStr = translateStructField(field.type, field.name)
                    declList.append(Translator.Declaration(declStr[0], declStr[1], field.init))
                    if builtinCond:
                        declList.append(Translator.Declaration(directive = f'#endif /* {builtinCond} */'))

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
        self.statement('namespace LLGL')
        self.openScope()

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
                    if decl.init != None:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('')
            self.statement('')

        # Write all flags
        if len(doc.flags) > 0:
            def translateFlagInitializer(init):
                str = init
                str = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', str)
                return str

            self.statement('/* ----- Flags ----- */')
            self.statement('')
            for flag in doc.flags:
                self.statement('[Flags]')
                self.statement('public enum {} : uint'.format(flag.name))
                basename = flag.name[:-len('Flags')]
                self.openScope()

                # Write flag entry declarations
                declList = Translator.DeclarationList()
                for field in flag.fields:
                    declList.append(Translator.Declaration('', field.name, translateFlagInitializer(field.init) if field.init else None))

                for decl in declList.decls:
                    if decl.init != None:
                        self.statement(decl.name + declList.spaces(1, decl.name) + '= ' + decl.init + ',')
                    else:
                        self.statement(decl.name + ',')

                self.closeScope()
                self.statement('')
            self.statement('')

        # Write all structures
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        if len(commonStructs) > 0:
            def translateStructField(type, name):
                def translateType(type):
                    if type.baseType == StdType.VOID:
                        return 'void'
                    elif type.baseType == StdType.BOOL:
                        return 'bool'
                    elif type.baseType == StdType.CHAR:
                        return 'byte'
                    elif type.baseType == StdType.INT8:
                        return 'sbyte'
                    elif type.baseType == StdType.INT16:
                        return 'short'
                    elif type.baseType == StdType.INT32:
                        return 'int'
                    elif type.baseType == StdType.INT64:
                        return 'long'
                    elif type.baseType == StdType.UINT8:
                        return 'byte'
                    elif type.baseType == StdType.UINT16:
                        return 'ushort'
                    elif type.baseType == StdType.UINT32:
                        return 'uint'
                    elif type.baseType == StdType.UINT64:
                        return 'ulong'
                    elif type.baseType == StdType.LONG:
                        return 'long'
                    elif type.baseType == StdType.SIZE_T:
                        return 'UIntPtr'
                    elif type.baseType == StdType.FLOAT:
                        return 'float'
                    else:
                        return type.typename

                typeStr = ''
                declStr = ''
                if type.baseType == StdType.STRUCT and type.typename in LLGLMeta.interfaces:
                    typeStr += type.typename
                else:
                    if type.arraySize > 0:
                        typeStr += 'fixed '
                    typeStr += translateType(type)
                    if type.isPointer:
                        typeStr += '*'
                declStr += name
                if type.arraySize > 0:
                    declStr += '[{}]'.format(type.arraySize)
                return (typeStr, declStr)

            self.statement('/* ----- Structures ----- */')
            self.statement('')
            for struct in commonStructs:
                self.statement('public unsafe struct ' + struct.name)
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    # Write two fields for dynamic arrays
                    if field.type.arraySize == -1:
                        declList.append(Translator.Declaration('UIntPtr', 'num{}{}'.format(field.name[0].upper(), field.name[1:])))
                    declStr = translateStructField(field.type, field.name)
                    declList.append(Translator.Declaration(declStr[0], declStr[1], field.init))

                for decl in declList.decls:
                    if decl.init != None:
                        self.statement('public ' + decl.type + declList.spaces(0, decl.type) + decl.name + ';' + declList.spaces(1, decl.name) + '/* = ' + decl.init + ' */')
                    else:
                        self.statement('public ' + decl.type + declList.spaces(0, decl.type) + decl.name + ';')
                self.closeScope()
                self.statement('')
            self.statement('')

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
        
        # Parse input headers
        modules = iterate(parseFile, files)
        if singleName != None and len(modules) > 0:
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
