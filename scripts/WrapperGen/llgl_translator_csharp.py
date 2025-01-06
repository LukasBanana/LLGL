#
# llgl_translator_csharp.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_translator import *

class CsharpProperties:
    setter = False
    getter = False
    fullCtor = False

    def __init__(self, setter = False, getter = False, fullCtor = False):
        self.setter = setter
        self.getter = getter
        self.fullCtor = fullCtor

class CsharpTranslator(Translator):
    class WrapperClasses:
        STRING = 'AnsiString'

    def __init__(self):
        self.indentDirectives = True

    def translateModule(self, doc):
        builtinTypenames = {
            StdType.VOID: 'void',
            StdType.BOOL: 'bool',
            StdType.CHAR: 'byte',
            StdType.WCHAR: 'char',
            StdType.INT8: 'byte', # sbyte is not CLS compliant
            StdType.INT16: 'short',
            StdType.INT32: 'int',
            StdType.INT64: 'long',
            StdType.UINT8: 'byte',
            StdType.UINT16: 'short', # ushort is not CLS compliant
            StdType.UINT32: 'int', # uint is not CLS compliant
            StdType.UINT64: 'long', # ulong is not CLS compliant
            StdType.LONG: 'int', # uint is not CLS compliant
            StdType.SIZE_T: 'IntPtr', # UIntPtr is not CLS compliant
            StdType.FLOAT: 'float',
            StdType.FUNC: 'IntPtr',
        }
        nativeBaseTypes = [
            'Resource',
            'Surface'
        ]
        saveStructs = {
            'BindingSlot': CsharpProperties(fullCtor = True),
            'DrawIndexedIndirectArguments': None,
            'DrawIndirectArguments': None,
            'DrawPatchIndirectArguments': None,
            'Extent2D': CsharpProperties(fullCtor = True),
            'Extent3D': CsharpProperties(fullCtor = True),
            'FormatAttributes': None,
            'Offset2D': CsharpProperties(fullCtor = True),
            'Offset3D': CsharpProperties(fullCtor = True),
            'QueryPipelineStatistics': None,
            'Scissor': CsharpProperties(fullCtor = True),
            'SubresourceFootprint': None,
            'TextureLocation': None,
            'TextureRegion': None,
            'TextureSubresource': None,
            'TextureSwizzleRGBA': None,
            'Viewport': CsharpProperties(fullCtor = True),
        }
        trivialClasses = {
            'AttachmentClear': CsharpProperties(getter = True),
            'AttachmentDescriptor': CsharpProperties(getter = True),
            'AttachmentFormatDescriptor': CsharpProperties(getter = True),
            'BlendTargetDescriptor': CsharpProperties(getter = True),
            'BindingDescriptor': CsharpProperties(getter = True, setter = True, fullCtor = True),
            'BufferDescriptor': CsharpProperties(getter = True, setter = True),
            'BufferViewDescriptor': CsharpProperties(getter = True),
            'ColorCodes': CsharpProperties(getter = True, setter = True),
            'CombinedTextureSamplerDescriptor': CsharpProperties(getter = True, setter = True),
            'CommandBufferDescriptor': CsharpProperties(getter = True),
            'ComputePipelineDescriptor': CsharpProperties(getter = True),
            'ComputeShaderAttributes': CsharpProperties(getter = True, setter = True, fullCtor = True),
            'DepthBiasDescriptor': CsharpProperties(getter = True),
            'DepthDescriptor': CsharpProperties(getter = True),
            'DisplayMode': CsharpProperties(getter = True, setter = True),
            'FragmentAttribute': CsharpProperties(getter = True, setter = True, fullCtor = True),
            'FragmentShaderAttributes': CsharpProperties(getter = True, setter = True, fullCtor = True),
            'FrameProfile': CsharpProperties(setter = True),
            'GraphicsPipelineDescriptor': CsharpProperties(getter = True),
            'PipelineLayoutDescriptor': CsharpProperties(getter = True),
            'ProfileCommandBufferRecord': CsharpProperties(setter = True),
            'ProfileCommandQueueRecord': CsharpProperties(setter = True),
            'ProfileTimeRecord': CsharpProperties(getter = True, setter = True),
            'QueryHeapDescriptor': CsharpProperties(getter = True),
            'RasterizerDescriptor': CsharpProperties(getter = True),
            'RenderingFeatures': CsharpProperties(setter = True),
            'RenderingLimits': CsharpProperties(setter = True),
            'RenderingCapabilities': CsharpProperties(setter = True),
            'ResourceHeapDescriptor': CsharpProperties(getter = True),
            'ResourceViewDescriptor': CsharpProperties(getter = True),
            'SamplerDescriptor': CsharpProperties(getter = True, setter = True),
            'ShaderMacro': CsharpProperties(getter = True, fullCtor = True),
            'ShaderReflection': CsharpProperties(setter = True),
            'ShaderResourceReflection': CsharpProperties(getter = True, setter = True),
            'StaticSamplerDescriptor': CsharpProperties(getter = True, setter = True),
            'StencilDescriptor': CsharpProperties(getter = True),
            'StencilFaceDescriptor': CsharpProperties(getter = True),
            'SwapChainDescriptor': CsharpProperties(getter = True, fullCtor = True),
            'TessellationDescriptor': CsharpProperties(getter = True),
            'TextureDescriptor': CsharpProperties(getter = True, setter = True),
            'TextureViewDescriptor': CsharpProperties(getter = True),
            'UniformDescriptor': CsharpProperties(getter = True, setter = True),
            'VertexAttribute': CsharpProperties(getter = True, setter = True, fullCtor = True),
            'VertexShaderAttributes': CsharpProperties(getter = True, setter = True, fullCtor = True),
        }

        self.statement('/*')
        self.statement(' * {}.cs'.format(doc.name))
        self.statement(' *')
        for line in LLGLMeta.copyright:
            self.statement(' * ' + line)
        self.statement(' */')
        self.statement()
        for line in LLGLMeta.info:
            self.statement('/* {} */'.format(line))
        self.statement()
        self.statement('using System;')
        self.statement('using System.Text;')
        self.statement('using System.Runtime.InteropServices;')
        self.statement()
        self.statement('namespace LLGL')
        self.openScope()

        class CsharpDeclaration:
            marshal = None
            deprecated = None
            type = ''
            ident = ''
            arraySize = 0

            def __init__(self, ident, arraySize = 0):
                self.marshal = None
                self.deprecated = None
                self.type = ''
                self.ident = ident
                self.arraySize = arraySize

        def translateField(field, isInsideStruct = False, noFixedScope = False, noRefTypes = False):
            nonlocal builtinTypenames

            fieldType = field.type
            decl = CsharpDeclaration(field.name, arraySize = fieldType.arraySize)

            def sanitizeTypename(typename):
                nonlocal isInsideStruct
                if typename.startswith(LLGLMeta.typePrefix):
                    return typename[len(LLGLMeta.typePrefix):]
                elif typename in LLGLMeta.stringClasses:
                    return 'byte*' if isInsideStruct else 'string'
                else:
                    return typename

            if fieldType.baseType == StdType.STRUCT and fieldType.typename in LLGLMeta.interfaces:
                decl.type = sanitizeTypename(fieldType.typename)
            elif fieldType.baseType == StdType.STRUCT and fieldType.typename in LLGLMeta.handles:
                decl.type = 'IntPtr' # Translate any handle to generic pointer type
            else:
                builtin = builtinTypenames.get(fieldType.baseType)
                if isInsideStruct:
                    if not noFixedScope and fieldType.arraySize > 0 and builtin:
                        decl.type += 'fixed '
                    decl.type += builtin if builtin else sanitizeTypename(fieldType.typename)
                    if fieldType.isPointer or fieldType.arraySize == LLGLType.DYNAMIC_ARRAY:
                        decl.type += '*'
                    elif fieldType.arraySize > 0:
                        if noFixedScope:
                            decl.type += '[]'
                        elif builtin:
                            decl.ident += f'[{fieldType.arraySize}]'
                        else:
                            decl.marshal = '<unroll>'
                else:
                    decl.type += builtin if builtin else sanitizeTypename(fieldType.typename)
                    if fieldType.isPointer or fieldType.arraySize > 0:
                        if LLGLAnnotation.NULLABLE in field.annotations or LLGLAnnotation.ARRAY in field.annotations:
                            decl.type += '*'
                        elif fieldType.baseType == StdType.STRUCT:
                            if noRefTypes:
                                decl.type += '*'
                            else:
                                decl.marshal = 'ref'
                        elif fieldType.baseType == StdType.CHAR:
                            decl.type = 'string'
                            decl.marshal = 'MarshalAs(UnmanagedType.LPStr)'
                        elif fieldType.baseType == StdType.WCHAR:
                            decl.type = 'string'
                            decl.marshal = 'MarshalAs(UnmanagedType.LPWStr)'
                        else:
                            decl.type += '*'

                if fieldType.baseType == StdType.BOOL and not (fieldType.isPointer or fieldType.arraySize > 0):
                    decl.marshal = 'MarshalAs(UnmanagedType.I1)'

            return decl

        def translateReturnType(type):
            return translateField(LLGLField(inName = None, inType = type), noRefTypes = True)

        def translateDeprecationMessage(msg):
            if msg is not None:
                msg = msg.replace('::', '.')
                return f'Obsolete({msg})'
            return None

        def identToPropertyIdent(ident):
            return Translator.convertIdentToPascalCase(ident)

        def classNameToFlagsName(className):
            return f'{className[:-len("Descriptor")] if className.endswith("Descriptor") else className}Flags'

        def translateInitializer(init, type, isParamList = False):
            nonlocal doc
            nonlocal saveStructs

            # Parameters in C# can only have compile-time default arguments but null is not allowed for structure types
            if isParamList and type in saveStructs:
                return None

            if init:
                # Replace common C-to-C# syntax
                for substitute in [['::', '.'], ['nullptr', 'null'], ['|', ' | '], [',', ', '], ['{', '{ '], ['}', ' }'], ['~0u', '-1'], ['0u', '0']]:
                    init = init.replace(substitute[0], substitute[1])

                if init.startswith('{') and init.endswith('}'):
                    # Parameters in C# can only have compile-time default arguments
                    if isParamList:
                        return 'null'

                    # Extract sub expressions from initializer list
                    fieldExprs = init[1:-1].split(',')

                    # Write initializer list as structure field initializer
                    struct = doc.findStructByName(type)
                    if struct:
                        if len(fieldExprs) != len(struct.fields):
                            fatal(f"error: mismatch between initializer expressions ({len(fieldExprs)}) and record fields ({len(struct.fields)}) for '{struct.name}'")
                        initExpr = ''
                        for fieldIndex in range(0, len(struct.fields)):
                            if len(initExpr) > 0:
                                initExpr += ', '
                            initExpr += f'{identToPropertyIdent(struct.fields[fieldIndex].name)} = {fieldExprs[fieldIndex]}'
                        return f'new {type}() ' + '{ ' + initExpr + ' }'
                else:
                    # Map known constant name to its value
                    constant = LLGLMeta.constants.get(init)
                    if constant:
                        if constant < 0:
                            if type == 'byte':
                                constant = StdTypeLimits.MAX_UINT8 + constant + 1
                            elif type == 'ushort':
                                constant = StdTypeLimits.MAX_UINT16 + constant + 1
                            elif type == 'uint':
                                constant = StdTypeLimits.MAX_UINT32 + constant + 1
                            elif type == 'ulong':
                                constant = StdTypeLimits.MAX_UINT64 + constant + 1
                            else:
                                return constant
                        return f'({type})0x{constant:X}'
                    
                return init
            else:
                # Parameters in C# can only have compile-time default arguments
                if isParamList:
                    return 'null'

                struct = doc.findStructByName(type)
                if struct:
                    return f'new {struct.name}()'

            return None

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement()

            def translateConstInit(struct, init):
                structBaseIdent = struct.name + '::'
                if init.startswith(structBaseIdent):
                    return init[len(structBaseIdent):]
                else:
                    return init

            for struct in constStructs:
                self.statement('public enum {} : int'.format(struct.name))
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    declList.append(Translator.Declaration('', field.name, field.init))

                for decl in declList.decls:
                    self.statement(f'{decl.name}{declList.spaces(1, decl.name)} = {translateConstInit(struct, decl.init)},')

                self.closeScope()
                self.statement()

        # Write all enumerations
        if len(doc.enums) > 0:
            self.statement('/* ----- Enumerations ----- */')
            self.statement()

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
                self.statement()

        # Write all flags
        if len(doc.flags) > 0:
            def translateFlagInitializer(init):
                s = init
                s = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', s)
                return s

            self.statement('/* ----- Flags ----- */')
            self.statement()

            for flag in doc.flags:
                self.statement('[Flags]')
                self.statement('public enum {} : int'.format(flag.name))
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
                self.statement()

        def typeToPropertyType(type, propName, className):
            nonlocal doc

            if type.endswith('*'):
                if type == 'byte*':
                    return 'string'
                else:
                    return f'{type[:-1]}[]'

            # Use 'int' for sized-types
            if type == 'IntPtr':
                return 'int'

            # Try to find flags type that matches the class name, e.g. 'CommandBufferFlags'
            if propName == 'Flags':
                flags = doc.findFlagsByName(classNameToFlagsName(className))
                if flags:
                    return flags.name

            if propName.endswith('Flags') or propName in LLGLMeta.structFlagProperties:
                # Try to find flags type that matches the property name, e.g. 'BindFlags'
                flags = doc.findFlagsByName(propName)
                if flags:
                    return flags.name

                # Try to map class name to known flags, e.g. 'BlendTargetDescriptor' to 'ColorMaskFlags'
                knownFlags = LLGLMeta.structFlags.get(className)
                if knownFlags:
                    return knownFlags

            return type


        # Write records that are trivial to map between unmanaged and managed code
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        def findRecordProperties(name):
            properties = saveStructs.get(name)
            return properties if properties is not None else trivialClasses.get(name)

        def writeStruct(struct, modifier = None, managedTypeProperties = None, fieldsAsProperties = False):
            nonlocal doc
            nonlocal saveStructs

            def isSafeType(typename):
                nonlocal doc
                nonlocal saveStructs
                return typename in saveStructs or doc.findEnumByName(typename) is not None or doc.findFlagsByName(typename) is not None

            self.statement(f'public {modifier + " " if modifier is not None else ""}{"class" if managedTypeProperties is not None else "struct"} {struct.name}')
            self.openScope()

            # Write struct field declarations
            declList = Translator.DeclarationList()
            for field in struct.fields:
                if not field.type.externalCond:
                    # Write two fields for dynamic arrays
                    if field.type.arraySize == LLGLType.DYNAMIC_ARRAY and not managedTypeProperties:
                        declList.append(Translator.Declaration('IntPtr', 'num{}{}'.format(field.name[0].upper(), field.name[1:])))

                    if field.deprecated:
                        declList.append(Translator.Declaration(None, translateDeprecationMessage(field.deprecated)))

                    fieldDecl = translateField(field, isInsideStruct = True, noFixedScope = (managedTypeProperties is not None))
                    declName = identToPropertyIdent(fieldDecl.ident) if fieldsAsProperties else fieldDecl.ident

                    if managedTypeProperties:
                        declType = typeToPropertyType(fieldDecl.type, declName, struct.name)
                        declList.append(Translator.Declaration(
                            CsharpTranslator.WrapperClasses.STRING if declType == 'string' else declType,
                            declName,
                            field.init,
                            inDeprecated = field.deprecated,
                            inOriginalType = fieldDecl.type,
                            inOriginalName = fieldDecl.ident,
                            inFixedArray = fieldDecl.arraySize))
                    elif fieldDecl.marshal and fieldDecl.marshal == '<unroll>':
                        for i in range(0, field.type.arraySize):
                            declList.append(Translator.Declaration(
                                fieldDecl.type,
                                f'{declName}{i}',
                                field.init if field.deprecated is None else None,
                                inOriginalName = fieldDecl.ident))
                    else:
                        if fieldDecl.marshal:
                            declList.append(Translator.Declaration(None, fieldDecl.marshal))

                        declList.append(Translator.Declaration(
                            fieldDecl.type,
                            declName,
                            field.init if field.deprecated is None else None,
                            inOriginalName = fieldDecl.ident))

            # Write optionally constructor that initializes all fields
            hasDefaultCtor = False

            if fieldsAsProperties:
                structProperties = findRecordProperties(struct.name)
                if structProperties and structProperties.fullCtor:
                    paramList = ''
                    hasParamsWithoutDefualtArg = False
                    defaultArgsStarted = False
                    for decl in declList.decls:
                        declType = 'string' if decl.type == CsharpTranslator.WrapperClasses.STRING else decl.type
                        if len(paramList) > 0:
                            paramList += ', '
                        paramList += f'{declType} {decl.originalName}'
                        declInit = translateInitializer(decl.init, declType, isParamList = True)
                        if declInit:
                            defaultArgsStarted = True
                            paramList += f' = {declInit}'
                        else:
                            hasParamsWithoutDefualtArg = True
                            if defaultArgsStarted:
                                paramList += f' = new {declType}()'
                                #fatal(f"error: no initializer defined for parameter '{decl.originalName}' in constructor '{struct.name}', but default argument list has already started")

                    if hasParamsWithoutDefualtArg and len(paramList) > 0:
                        self.statement(f'public {struct.name}() ' + '{ }')
                        self.statement()
                        hasDefaultCtor = True

                    self.statement(f'public {struct.name}({paramList})')
                    self.openScope()

                    for decl in declList.decls:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= {decl.originalName};')

                    self.closeScope()
                    self.statement()

            # Write all fields as variables or properties
            hasUnsafeContext = False

            for decl in declList.decls:
                if not decl.type:
                    self.statement(f'[{decl.name}]')
                else:
                    if decl.fixedArray > 0:
                        hasUnsafeContext = True
                        
                    if managedTypeProperties and decl.originalType.endswith('*') and decl.type != CsharpTranslator.WrapperClasses.STRING:
                        hasUnsafeContext = True

                        # Translate array type with internal native array
                        subType = decl.type[:-2]

                        if isSafeType(subType):
                            self.statement(f'public {decl.type}{declList.spaces(0, decl.type)}{decl.name} ' + '{ get; set; }')
                        else:
                            originalSubType = f'NativeLLGL.{decl.originalType[:-1]}'

                            self.statement(f'private {decl.type} {decl.originalName};')
                            self.statement(f'private {originalSubType + "[]"} {decl.originalName}Native;')
                            self.statement(f'public {decl.type} {decl.name}')
                            self.openScope()

                            self.statement('get')
                            self.openScope()
                            self.statement(f'return {decl.originalName};')
                            self.closeScope()

                            self.statement('set')
                            self.openScope()
                            self.statement('if (value != null)')
                            self.openScope()
                            self.statement(f'{decl.originalName} = value;')
                            self.statement(f'{decl.originalName}Native = new {originalSubType}[{decl.originalName}.Length];')
                            self.statement(f'for (int {decl.originalName}Index = 0; {decl.originalName}Index < {decl.originalName}.Length; ++{decl.originalName}Index)')
                            self.openScope()
                            self.statement(f'if ({decl.originalName}[{decl.originalName}Index] != null)')
                            self.openScope()
                            self.statement(f'{decl.originalName}Native[{decl.originalName}Index] = {decl.originalName}[{decl.originalName}Index].Native;')
                            self.closeScope()
                            self.closeScope()
                            self.closeScope()
                            self.statement('else')
                            self.openScope()
                            self.statement(f'{decl.originalName} = null;')
                            self.statement(f'{decl.originalName}Native = null;')
                            self.closeScope()
                            self.closeScope()

                            self.closeScope()

                    else:
                        if decl.type == CsharpTranslator.WrapperClasses.STRING:
                            hasUnsafeContext = True

                        fieldStmt = f'public {decl.type}{declList.spaces(0, decl.type)}{decl.name}'
                        if fieldsAsProperties:
                            fieldStmt += ' { get; set; }'
                            declInit = translateInitializer(decl.init, decl.type)
                            if declInit:
                                fieldStmt += declList.spaces(1, decl.name)
                                if managedTypeProperties:
                                    if decl.type.endswith('[]'):
                                        subType = decl.type[:-2]
                                        fieldStmt += f'= new {subType}[]{declInit};'
                                    else:
                                        fieldStmt += f'= {declInit};'
                                else:
                                    fieldStmt += f'/* = {declInit} */'
                        else:
                            fieldStmt += ';'
                            if decl.init:
                                fieldStmt += f'{declList.spaces(1, decl.name)}/* = {translateInitializer(decl.init, decl.type)} */'
                        self.statement(fieldStmt)

            def getNativeTypeAttrib(type):
                if (type in LLGLMeta.interfaces or doc.findStructByName(type)) and type not in saveStructs:
                    return 'NativeBase' if type in nativeBaseTypes else 'Native'
                return None

            # Write optional conversion to native type
            if managedTypeProperties:
                if managedTypeProperties.setter:
                    # Write constructors for implicit conversion
                    self.statement()
                    if not hasDefaultCtor:
                        self.statement(f'public {struct.name}() ' + '{ }')
                        self.statement()
                    self.statement(f'internal {struct.name}(NativeLLGL.{struct.name} native)')
                    self.openScope()
                    self.statement('Native = native;')
                    self.closeScope()

                self.statement()
                self.statement(f'internal NativeLLGL.{struct.name} Native')
                self.openScope()

                declList.appendSubscriptSpaces()

                if managedTypeProperties.getter:
                    self.statement('get')
                    self.openScope()
                    self.statement(f'var native = new NativeLLGL.{struct.name}();')

                    if hasUnsafeContext:
                        self.statement('unsafe')
                        self.openScope()

                    def writeGetterAssignment(decl, subscript = ''):
                        if decl.originalType.endswith('*'):
                            if subscript != '':
                                fatal(f"cannot generate fixed array for 'byte* {decl.name}'")
                            if decl.originalType == 'byte*':
                                self.statement(f'fixed (byte* {decl.originalName}Ptr = {decl.name}.Ascii)')
                                self.openScope()
                                self.statement(f'native.{decl.originalName} = {decl.originalName}Ptr;')
                                self.closeScope()
                            else:
                                subType = decl.type[:-2]
                                if subType in saveStructs:
                                    self.statement(f'if ({decl.name} != null)')
                                    self.openScope()
                                    self.statement(f'native.num{decl.name} = (IntPtr){decl.name}.Length;')
                                    self.statement(f'fixed ({decl.originalType} {decl.originalName}Ptr = {decl.name})')
                                    self.openScope()
                                    self.statement(f'native.{decl.originalName} = {decl.originalName}Ptr;')
                                    self.closeScope()
                                    self.closeScope()
                                else:
                                    self.statement(f'if ({decl.originalName} != null)')
                                    self.openScope()
                                    self.statement(f'native.num{decl.name} = (IntPtr){decl.originalName}.Length;')
                                    self.statement(f'fixed (NativeLLGL.{decl.originalType} {decl.originalName}Ptr = {decl.originalName}Native)')
                                    self.openScope()
                                    self.statement(f'native.{decl.originalName} = {decl.originalName}Ptr;')
                                    self.closeScope()
                                    self.closeScope()
                        else:
                            nativeAttrib = getNativeTypeAttrib(decl.type)
                            if nativeAttrib is not None:
                                self.statement(f'if ({decl.name} != null)')
                                self.openScope()
                                assignStmt = f'native.{decl.originalName}{subscript} = '
                                if decl.type != decl.originalType:
                                    assignStmt += f'({decl.originalType})'
                                assignStmt += f'{decl.name}{subscript}.{nativeAttrib}'
                                self.statement(assignStmt + ';')
                                self.closeScope()
                            else:
                                assignStmt = f'{decl.originalName}{subscript}'
                                assignStmt = f'native.{assignStmt}{declList.spaces(1, assignStmt)}= '
                                if decl.type != decl.originalType:
                                    assignStmt += f'({decl.originalType})'
                                assignStmt += f'{decl.name}{subscript}'
                                self.statement(assignStmt + ';')

                    for decl in declList.decls:
                        if decl.type and not decl.deprecated:
                            if decl.fixedArray > 0:
                                for i in range(decl.fixedArray):
                                    writeGetterAssignment(decl, subscript = f'[{i}]')
                            else:
                                writeGetterAssignment(decl)

                    if hasUnsafeContext:
                        self.closeScope()

                    self.statement('return native;')
                    self.closeScope()

                if managedTypeProperties.setter:
                    self.statement('set')
                    self.openScope()

                    if hasUnsafeContext:
                        self.statement('unsafe')
                        self.openScope()

                    def writeSetterAssignment(decl, subscript = ''):
                        if decl.originalType.endswith('*'):
                            if subscript != '':
                                fatal(f"cannot generate fixed array for 'byte* {decl.name}'")
                            if decl.originalType == 'byte*':
                                self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= Marshal.PtrToStringAnsi((IntPtr)value.{decl.originalName});')
                            else:
                                subType = decl.type[:-2]
                                self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= new {subType}[(int)value.num{decl.name}];')
                                self.statement(f'for (int i = 0; i < {decl.name}.Length; ++i)')
                                self.openScope()
                                nativeAttribs = getNativeTypeAttrib(subType)
                                if nativeAttribs is not None:
                                    self.statement(f'{decl.name}[i] = new {subType}(value.{decl.originalName}[i]);')
                                else:
                                    self.statement(f'{decl.name}[i] = value.{decl.originalName}[i];')
                                self.closeScope()
                        else:
                            assignStmt = f'{decl.name}{subscript}'
                            nativeAttribs = getNativeTypeAttrib(decl.type)
                            if nativeAttribs is not None:
                                assignStmt += f'.{nativeAttribs}'
                            assignStmt += f'{declList.spaces(1, assignStmt)}= '
                            if decl.type != decl.originalType:
                                assignStmt += f'({decl.type})'
                            assignStmt += f'value.{decl.originalName}{subscript}'
                            self.statement(assignStmt + ';')

                    for decl in declList.decls:
                        if decl.type and not decl.deprecated:
                            if decl.fixedArray > 0:
                                for i in range(decl.fixedArray):
                                    writeSetterAssignment(decl, subscript = f'[{i}]')
                            else:
                                writeSetterAssignment(decl)

                    if hasUnsafeContext:
                        self.closeScope()

                    self.closeScope()

                self.closeScope()

            self.closeScope()
            self.statement()

        if len(commonStructs) > 0:
            # Write all trivial structures
            self.statement('/* ----- Structures ----- */')
            self.statement()
            for struct in commonStructs:
                if struct.name in saveStructs:
                    writeStruct(struct, fieldsAsProperties = True)

            # Write all trivial classes (with conversion to native struct)
            self.statement('/* ----- Classes ----- */')
            self.statement()
            for struct in commonStructs:
                property = trivialClasses.get(struct.name)
                if property:
                    writeStruct(struct, managedTypeProperties = trivialClasses.get(struct.name), fieldsAsProperties = True)

        # Write native LLGL interface
        self.statement('#region NativeLLGL - native interface to LLGL using P/Invoke')
        self.statement()
        self.statement('internal static class NativeLLGL')
        self.openScope()

        # Write DLL name
        self.statement('#if DEBUG')
        self.statement('const string DllName = "LLGLD";')
        self.statement('#else')
        self.statement('const string DllName = "LLGL";')
        self.statement('#endif')
        self.statement()
        self.statement('#pragma warning disable 0649 // Disable warning about unused fields')
        self.statement()

        # Write all interface handles
        self.statement('/* ----- Handles ----- */')
        self.statement()

        def writeInterfaceCtor(self, interface, parent):
            self.statement(f'public {interface}({parent} instance)')
            self.openScope()
            self.statement('ptr = instance.ptr;')
            self.closeScope()

        def writeInterfaceInterpret(self, interface):
            self.statement(f'public {interface} As{interface}()')
            self.openScope()
            self.statement(f'return new {interface}(this);')
            self.closeScope()

        def writeInterfaceRelation(self, interface, parent, children):
            if interface in children:
                writeInterfaceCtor(self, interface, parent)
                writeInterfaceInterpret(self, parent)
            elif interface == parent:
                for child in children:
                    writeInterfaceCtor(self, parent, child)
                    writeInterfaceInterpret(self, child)

        for interface in LLGLMeta.interfaces:
            self.statement(f'public unsafe struct {interface}')
            self.openScope()
            self.statement('internal unsafe void* ptr;')
            writeInterfaceRelation(self, interface, 'Surface', ['Window', 'Canvas'])
            writeInterfaceRelation(self, interface, 'RenderTarget', ['SwapChain'])
            writeInterfaceRelation(self, interface, 'Resource', ['Buffer', 'Texture', 'Sampler'])
            self.closeScope()
            self.statement()

        # Write all non-trivial native structures
        if len(commonStructs) > 0:
            self.statement('/* ----- Native structures ----- */')
            self.statement()
            for struct in commonStructs:
                if not struct.name in saveStructs:
                    writeStruct(struct, modifier = 'unsafe')

        def translateParamList(func):
            paramListStr = ''

            for param in func.params:
                if len(paramListStr) > 0:
                    paramListStr += ', '
                paramDecl = translateField(param)
                if paramDecl.marshal:
                    if paramDecl.marshal == 'ref':
                        paramListStr += f'{paramDecl.marshal} '
                    else:
                        paramListStr += f'[{paramDecl.marshal}] '
                paramListStr += f'{paramDecl.type} {paramDecl.ident}'

            return paramListStr

        # Write all native delegates
        if len(doc.delegates) > 0:
            self.statement('/* ----- Native delegates ----- */')
            self.statement()

            for delegate in doc.delegates:
                self.statement(f'[UnmanagedFunctionPointer(CallingConvention.Cdecl)]');

                returnType = translateReturnType(delegate.returnType)
                if returnType.marshal and returnType.marshal != 'ref':
                    self.statement(f'[return: {returnType.marshal}]')

                delegateName = delegate.name[len(LLGLMeta.delegatePrefix):]
                self.statement(f'public unsafe delegate {returnType.type} {delegateName}Delegate({translateParamList(delegate)});');
                self.statement()

            self.statement()

        # Write all native functions
        if len(doc.funcs) > 0:
            self.statement('/* ----- Native functions ----- */')
            self.statement()

            errFuncNames = []

            for func in doc.funcs:
                # Ignore functions with variadic arguments for now
                if func.hasVargs():
                    continue

                # All exported function are expected to start with 'llgl' prefix
                if func.name[:len(LLGLMeta.funcPrefix)] != LLGLMeta.funcPrefix:
                    errFuncNames.append(func.name)
                    continue

                self.statement(f'[DllImport(DllName, EntryPoint="{func.name}", CallingConvention=CallingConvention.Cdecl)]');

                returnType = translateReturnType(func.returnType)
                if returnType.marshal and returnType.marshal != 'ref':
                    self.statement(f'[return: {returnType.marshal}]')

                funcName = func.name[len(LLGLMeta.funcPrefix):]
                self.statement(f'public static extern unsafe {returnType.type} {funcName}({translateParamList(func)});');
                self.statement()

            if len(errFuncNames) > 0:
                errFuncNameList = ''
                for name in errFuncNames:
                    if len(errFuncNameList) > 0:
                        errFuncNameList += ', '
                    errFuncNameList += name
                fatal(f"error: the following functions were exported but do not match the prefix '{LLGLMeta.funcPrefix}': {errFuncNameList}")

        self.statement('#pragma warning restore 0649 // Restore warning about unused fields')
        self.closeScope()

        self.statement()
        self.statement('#endregion')
        self.closeScope()

        self.statement()
        self.statement()
        self.statement()
        self.statement()
        self.statement('// ================================================================================')
