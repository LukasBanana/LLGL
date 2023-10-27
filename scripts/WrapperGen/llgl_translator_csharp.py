#
# llgl_translator_csharp.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_translator import *

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
            'Offset2D',
            'Offset3D',
            'Extent2D',
            'Extent3D',
            'Viewport',
            'Scissor',
            'BindingSlot',
            'DrawIndirectArguments',
            'DrawIndexedIndirectArguments',
            'DrawPatchIndirectArguments',
        ]
        trivialClasses = [
            'RenderingFeatures',
        ]

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
            type = ''
            ident = ''

            def __init__(self, ident):
                self.marshal = None
                self.type = ''
                self.ident = ident

        def translateDecl(declType, ident = None, isInsideStruct = False):
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
                    elif declType.baseType == StdType.BOOL:
                        decl.marshal = 'MarshalAs(UnmanagedType.I1)'
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

            return decl

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

        # Write records that are trivial to map between unmanaged and managed code
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        def writeStruct(struct, modifier = None):
            self.statement(f'public {modifier + " " if modifier != None else ""}struct {struct.name}')
            self.openScope()

            # Write struct field declarations
            declList = Translator.DeclarationList()
            for field in struct.fields:
                if not field.type.externalCond:
                    # Write two fields for dynamic arrays
                    if field.type.arraySize == LLGLType.DYNAMIC_ARRAY:
                        declList.append(Translator.Declaration('UIntPtr', 'num{}{}'.format(field.name[0].upper(), field.name[1:])))
                    fieldDecl = translateDecl(field.type, field.name, isInsideStruct = True)
                    if fieldDecl.marshal and fieldDecl.marshal == '<unroll>':
                        for i in range(0, field.type.arraySize):
                            declList.append(Translator.Declaration(fieldDecl.type, f'{fieldDecl.ident}{i}', field.init))
                    else:
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
            self.statement()

        # Write all trivial structures
        if len(commonStructs) > 0:
            self.statement('/* ----- Structures ----- */')
            self.statement()
            for struct in commonStructs:
                if struct.name in saveStructs:
                    writeStruct(struct)
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
            self.closeScope()
            self.statement()

        self.statement()

        # Write all non-trivial native structures
        if len(commonStructs) > 0:
            self.statement('/* ----- Native structures ----- */')
            self.statement()
            for struct in commonStructs:
                if not struct.name in saveStructs:
                    writeStruct(struct, 'unsafe')
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
                if returnType.type == 'bool':
                    self.statement(f'[return: MarshalAs(UnmanagedType.I1)]')
                elif returnType.marshal:
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
                self.statement(f'[DllImport(DllName, EntryPoint="{func.name}", CallingConvention=CallingConvention.Cdecl)]');

                returnType = translateDecl(func.returnType)
                if returnType.type == 'bool':
                    self.statement(f'[return: MarshalAs(UnmanagedType.I1)]')
                elif returnType.marshal:
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
