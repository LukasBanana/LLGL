#
# llgl_module.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from enum import Enum

class StdType(Enum):
    UNDEFINED = 0
    VOID = 1
    BOOL = 2
    CHAR = 3
    WCHAR = 4
    INT8 = 5
    INT16 = 6
    INT32 = 7
    INT64 = 8
    UINT8 = 9
    UINT16 = 10
    UINT32 = 11
    UINT64 = 12
    LONG = 13
    SIZE_T = 14
    FLOAT = 15
    ENUM = 16
    FLAGS = 17
    STRUCT = 18
    FUNC = 19 # Function pointer
    CONST = 20 # static constexpr int
    VARGS = 21 # variadic arguments

class StdTypeLimits:
    MAX_UINT8 = 0xFF
    MAX_UINT16 = 0xFFFF
    MAX_UINT32 = 0xFFFF_FFFF
    MAX_UINT64 = 0xFFFF_FFFF_FFFF_FFFF

class ConditionalType:
    name = ''
    cond = None
    include = None

    def __init__(self, name, cond = None, include = None):
        self.name = name
        self.cond = cond
        self.include = include

class LLGLAnnotation(Enum):
    UNDEFINED = 0
    NULLABLE = 1
    ARRAY = 2

class LLGLMeta:
    UTF8STRING = 'UTF8String'
    STRING = 'string'
    externals = [
        ConditionalType('android_app', '__ANDROID__', '<android_native_app_glue.h>')
    ]
    builtins = {
        'void': StdType.VOID,
        'bool': StdType.BOOL,
        'char': StdType.CHAR,
        'wchar_t': StdType.WCHAR,
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
        'const': StdType.CONST,
        '...': StdType.VARGS
    }
    containers = [
        'vector',
        'ArrayView',
        'SmallVector',
        'DynamicVector'
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
        'PipelineCache',
        'PipelineLayout',
        'PipelineState',
        'QueryHeap',
        'RenderPass',
        'RenderTarget',
        'RenderSystemChild',
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
    handles = [
        'LLGLLogHandle'
    ]
    structFlags = {
        'AttachmentClear': 'ClearFlags',
        'BlendTargetDescriptor': 'ColorMaskFlags',
        'ColorCodes': 'ColorFlags'
    }
    structFlagProperties = [
        'ColorMask'
    ]
    constants = {
        'LLGL_MAX_NUM_COLOR_ATTACHMENTS': 8,
        'LLGL_MAX_NUM_ATTACHMENTS': 9,
        'LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS': 16,
        'LLGL_MAX_NUM_SAMPLES': 64,
        'LLGL_MAX_NUM_SO_BUFFERS': 4,
        'LLGL_MAX_THREAD_COUNT': -1,
        'LLGL_INVALID_SLOT': -1,
        'LLGL_WHOLE_SIZE': -1,
        'LLGL_CURRENT_SWAP_INDEX': -1
    }
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
    delegatePrefix = 'LLGL_PFN_'

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
            if typename.startswith('LLGL_PFN_'):
                return StdType.FUNC
            else:
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

    def isStringOfAnyKind(self):
        return self.typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING] or (self.isPointer and self.baseType in [StdType.CHAR, StdType.WCHAR])

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
    deprecated = None
    annotations = []

    def __init__(self, inName, inType = LLGLType()):
        self.name = inName
        self.type = inType
        self.init = None
        self.deprecated = None
        self.annotations = []

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

    def hasVargs(self):
        for param in self.params:
            if param.type.baseType == StdType.VARGS:
                return True
        return False

class LLGLModule:
    name = ''
    enums = [] # Array of LLGLRecord
    flags = [] # Array of LLGLRecord
    structs = [] # Array of LLGLRecord
    funcs = [] # Array of LLGLFunction
    delegates = [] # Array of LLGLFunction
    typeDeps = set() # Set of types used in this header

    def __init__(self):
        self.name = ''
        self.enums = []
        self.flags = []
        self.structs = []
        self.funcs = []
        self.delegates = []
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
        self.delegates.extend(other.delegates)
        self.typeDeps.update(other.typeDeps)

    def findEnumByName(self, name):
        for enum in self.enums:
            if enum.name == name:
                return enum
        return None

    def findFlagsByName(self, name):
        for flag in self.flags:
            if flag.name == name:
                return flag
        return None

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

