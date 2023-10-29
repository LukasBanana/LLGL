#
# llgl_translator_csharp.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_translator import *

class CsharpProperty:
    setter = False
    getter = False

    def __init__(self, setter = False, getter = False):
        self.setter = setter
        self.getter = getter

class CsharpTranslator(Translator):
    def translateModule(self, doc):
        builtinTypenames = {
            StdType.VOID: 'void',
            StdType.BOOL: 'bool',
            StdType.CHAR: 'byte',
            StdType.WCHAR: 'char',
            StdType.INT8: 'sbyte',
            StdType.INT16: 'short',
            StdType.INT32: 'int',
            StdType.INT64: 'long',
            StdType.UINT8: 'byte',
            StdType.UINT16: 'ushort',
            StdType.UINT32: 'uint',
            StdType.UINT64: 'ulong',
            StdType.LONG: 'uint',
            StdType.SIZE_T: 'UIntPtr',
            StdType.FLOAT: 'float',
            StdType.FUNC: 'IntPtr',
        }
        saveStructs = [
            'BindingSlot',
            'DisplayModeDescriptor',
            'DrawIndexedIndirectArguments',
            'DrawIndirectArguments',
            'DrawPatchIndirectArguments',
            'Extent2D',
            'Extent3D',
            'FormatAttributes',
            'Offset2D',
            'Offset3D',
            'QueryPipelineStatistics',
            'Scissor',
            'SubresourceFootprint',
            'TextureSubresource',
            'Viewport',
        ]
        trivialClasses = {
            'BlendTargetDescriptor': CsharpProperty(getter = True), 
            'BufferViewDescriptor': CsharpProperty(getter = True),
            'CommandBufferDescriptor': CsharpProperty(getter = True),
            'DepthBiasDescriptor': CsharpProperty(getter = True),
            'DepthDescriptor': CsharpProperty(getter = True),
            'RasterizerDescriptor': CsharpProperty(getter = True),
            'RenderingFeatures': CsharpProperty(setter = True),
            'StencilFaceDescriptor': CsharpProperty(getter = True),
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
        self.statement('using System.Runtime.InteropServices;')
        self.statement()
        self.statement('namespace LLGL')
        self.openScope()

        class CsharpDeclaration:
            marshal = None
            deprecated = None
            type = ''
            ident = ''

            def __init__(self, ident):
                self.marshal = None
                self.deprecated = None
                self.type = ''
                self.ident = ident

        def translateDecl(declType, ident = None, isInsideStruct = False, isReturnType = False):
            decl = CsharpDeclaration(ident)

            def sanitizeTypename(typename):
                nonlocal isInsideStruct
                if typename.startswith(LLGLMeta.typePrefix):
                    return typename[len(LLGLMeta.typePrefix):]
                elif typename in [LLGLMeta.UTF8STRING, LLGLMeta.STRING]:
                    return 'string' if not isInsideStruct else 'byte*'
                else:
                    return typename

            nonlocal builtinTypenames

            if declType.baseType == StdType.STRUCT and declType.typename in LLGLMeta.interfaces:
                decl.type = sanitizeTypename(declType.typename)
            elif declType.baseType == StdType.STRUCT and declType.typename in LLGLMeta.handles:
                decl.type = 'IntPtr' # Translate any handle to generic pointer type
            else:
                builtin = builtinTypenames.get(declType.baseType)
                if isInsideStruct:
                    if declType.arraySize > 0 and builtin:
                        decl.type += 'fixed '
                    decl.type += builtin if builtin else sanitizeTypename(declType.typename)
                    if declType.isPointer or declType.arraySize == LLGLType.DYNAMIC_ARRAY:
                        decl.type += '*'
                    elif declType.arraySize > 0:
                        if builtin:
                            decl.ident += f'[{declType.arraySize}]'
                        else:
                            decl.marshal = '<unroll>'
                else:
                    decl.type += builtin if builtin else sanitizeTypename(declType.typename)
                    if declType.isPointer or declType.arraySize > 0:
                        if declType.baseType == StdType.STRUCT:
                            decl.marshal = 'ref'
                        elif declType.baseType == StdType.CHAR:
                            decl.type = 'string'
                            decl.marshal = 'MarshalAs(UnmanagedType.LPStr)'
                        elif declType.baseType == StdType.WCHAR:
                            decl.type = 'string'
                            decl.marshal = 'MarshalAs(UnmanagedType.LPWStr)'
                        else:
                            decl.type += '*'

                if declType.baseType == StdType.BOOL and not (declType.isPointer or declType.arraySize > 0):
                    decl.marshal = 'MarshalAs(UnmanagedType.I1)'

            return decl

        def translateDeprecationMessage(msg):
            if msg is not None:
                msg = msg.replace('::', '.')
                return f'Obsolete({msg})'
            return None

        def translateInitializer(init, type):
            if init is not None:
                init = init.replace('::', '.')
                init = init.replace('nullptr', 'null')

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
                    return f'({type})0x{constant:X}'
                    
                return init
            return None

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement()

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
                self.statement()

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
                self.statement()

            self.statement()

        def identToPropertyIdent(ident):
            return ident[0].upper() + ident[1:] if len(ident) > 0 else ident;

        def classNameToFlagsName(className):
            return f'{className[:-len("Descriptor")] if className.endswith("Descriptor") else className}Flags'

        def typeToPropertyType(type, propName, className):
            nonlocal doc

            if propName == 'Flags':
                # Try to find flags type that matches the class name, e.g. 'CommandBufferFlags'
                flags = doc.findFlagsByName(classNameToFlagsName(className))
                if flags:
                    return flags.name

            elif propName.endswith('Flags'):
                # Try to find flags type that matches the property name, e.g. 'BindFlags'
                flags = doc.findFlagsByName(propName)
                if flags:
                    return flags.name

            elif propName in LLGLMeta.specialFlags:
                # Try to find flags type that matches the property name with 'Flags' added, e.g. 'ColorMaskFlags'
                flags = doc.findFlagsByName(propName + 'Flags')
                if flags:
                    return flags.name

            return type


        # Write records that are trivial to map between unmanaged and managed code
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        def writeStruct(struct, modifier = None, managedTypeProperties = None, fieldsAsProperties = False):
            nonlocal trivialClasses

            self.statement(f'public {modifier + " " if modifier is not None else ""}{"class" if managedTypeProperties is not None else "struct"} {struct.name}')
            self.openScope()

            # Write struct field declarations
            declList = Translator.DeclarationList()
            for field in struct.fields:
                if not field.type.externalCond:
                    # Write two fields for dynamic arrays
                    if field.type.arraySize == LLGLType.DYNAMIC_ARRAY:
                        declList.append(Translator.Declaration('UIntPtr', 'num{}{}'.format(field.name[0].upper(), field.name[1:])))

                    if field.deprecated:
                        declList.append(Translator.Declaration(None, translateDeprecationMessage(field.deprecated)))

                    fieldDecl = translateDecl(field.type, field.name, isInsideStruct = True)
                    declName = identToPropertyIdent(fieldDecl.ident) if fieldsAsProperties else fieldDecl.ident

                    if managedTypeProperties:
                        declType = typeToPropertyType(fieldDecl.type, declName, struct.name)
                        declList.append(Translator.Declaration(declType, declName, field.init, inDeprecated = field.deprecated, inOriginalType = fieldDecl.type, inOriginalName = fieldDecl.ident))
                    elif fieldDecl.marshal and fieldDecl.marshal == '<unroll>':
                        for i in range(0, field.type.arraySize):
                            declList.append(Translator.Declaration(fieldDecl.type, f'{declName}{i}', field.init if field.deprecated is None else None))
                    else:
                        if fieldDecl.marshal:
                            declList.append(Translator.Declaration(None, fieldDecl.marshal))
                        declList.append(Translator.Declaration(fieldDecl.type, declName, field.init if field.deprecated is None else None))

            # Write all fields as variables or properties
            for decl in declList.decls:
                if not decl.type:
                    self.statement(f'[{decl.name}]')
                else:
                    fieldStmt = f'public {decl.type}{declList.spaces(0, decl.type)}{decl.name}'
                    if fieldsAsProperties:
                        fieldStmt += ' { get; set; }'
                        if decl.init:
                            fieldStmt += declList.spaces(1, decl.name)
                            if managedTypeProperties:
                                fieldStmt += f' = {translateInitializer(decl.init, decl.type)};'
                            else:
                                fieldStmt += f'/* = {translateInitializer(decl.init, decl.type)} */'
                    else:
                        fieldStmt += ';'
                        if decl.init:
                            fieldStmt += f'{declList.spaces(1, decl.name)}/* = {translateInitializer(decl.init, decl.type)} */'
                    self.statement(fieldStmt)

            # Write optional conversion to native type
            if managedTypeProperties:
                self.statement()
                self.statement(f'internal NativeLLGL.{struct.name} Native')
                self.openScope()

                if managedTypeProperties.getter:
                    self.statement('get')
                    self.openScope()
                    self.statement(f'var native = new NativeLLGL.{struct.name}();')

                    for decl in declList.decls:
                        if decl.type and not decl.deprecated:
                            assignStmt = f'native.{decl.originalName}{declList.spaces(1, decl.name)}= '
                            if decl.type != decl.originalType:
                                assignStmt += f'({decl.originalType}){decl.name}'
                            else:
                                assignStmt += decl.name
                            if decl.type in trivialClasses:
                                assignStmt += '.Native'
                            self.statement(assignStmt + ';')

                    self.statement('return native;')
                    self.closeScope()

                if managedTypeProperties.setter:
                    self.statement('set')
                    self.openScope()

                    for decl in declList.decls:
                        if decl.type and not decl.deprecated:
                            assignStmt = decl.name
                            if decl.type in trivialClasses:
                                assignStmt += '.Native'
                            assignStmt += f'{declList.spaces(1, assignStmt)}= '
                            if decl.type != decl.originalType:
                                assignStmt += f'({decl.type})value.{decl.originalName}'
                            else:
                                assignStmt += f'value.{decl.originalName}'
                            self.statement(assignStmt + ';')

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
            self.statement()

            # Write all trivial classes (with conversion to native struct)
            self.statement('/* ----- Classes ----- */')
            self.statement()
            for struct in commonStructs:
                property = trivialClasses.get(struct.name)
                if property:
                    writeStruct(struct, managedTypeProperties = trivialClasses.get(struct.name), fieldsAsProperties = True)
            self.statement()

        # Write native LLGL interface
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

        self.statement()

        # Write all non-trivial native structures
        if len(commonStructs) > 0:
            self.statement('/* ----- Native structures ----- */')
            self.statement()
            for struct in commonStructs:
                if not struct.name in saveStructs:
                    writeStruct(struct, modifier = 'unsafe')
            self.statement()

        def translateParamList(func):
            paramListStr = ''

            for param in func.params:
                if len(paramListStr) > 0:
                    paramListStr += ', '
                paramDecl = translateDecl(param.type, param.name)
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

                returnType = translateDecl(delegate.returnType)
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

            for func in doc.funcs:
                # Ignore functions with variadic arguments for now
                if func.hasVargs():
                    continue

                self.statement(f'[DllImport(DllName, EntryPoint="{func.name}", CallingConvention=CallingConvention.Cdecl)]');

                returnType = translateDecl(func.returnType)
                if returnType.marshal and returnType.marshal != 'ref':
                    self.statement(f'[return: {returnType.marshal}]')

                funcName = func.name[len(LLGLMeta.funcPrefix):]
                self.statement(f'public static extern unsafe {returnType.type} {funcName}({translateParamList(func)});');
                self.statement()

        self.statement('#pragma warning restore 0649 // Restore warning about unused fields')
        self.statement()

        self.closeScope()
        self.closeScope()
        self.statement()
        self.statement()
        self.statement()
        self.statement()
        self.statement('// ================================================================================')
