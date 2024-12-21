#
# llgl_translator_golang.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_translator import *

class GolangTranslator(Translator):
    def translateModule(self, doc):
        def translateDeprecationMessage(msg):
            if msg is not None:
                if msg.startswith('"') and msg.endswith('"'):
                    msg = msg[1:-1] # Remove quotation marks
                msg = msg.replace('LLGL::', 'LLGL')
                msg = msg.replace('::', '.')
                return msg
            return None

        # Private names in Go must be in 'camelCase'
        def toPrivateName(ident):
            return Translator.convertIdentToCamelCase(ident)
        
        # Public names in Go must be in 'PascalCase'
        def toPublicName(ident):
            return Translator.convertIdentToPascalCase(ident)

        # Drops the '_t' suffix of standard typenames
        def toTypenameWithoutStdSuffix(typename):
            return typename[:-2] if typename[-2:] == '_t' else typename

        self.statement('/*')
        self.statement(f' * {doc.name}.h')
        self.statement(' *')
        for line in LLGLMeta.copyright:
            self.statement(f' * {line}')
        self.statement(' */')
        self.statement()
        for line in LLGLMeta.info:
            self.statement(f'/* {line} */')
        self.statement()

        # Write C-include boilerplate code
        self.statement('package llgl')
        self.statement()
        self.statement('// #cgo LDFLAGS: libLLGL.dll.a') #TODO: this is for experimentation only
        self.statement('// #cgo CFLAGS: -I ../../include') #TODO: this is for experimentation only
        self.statement('// #include <LLGL-C/LLGL.h>')
        self.statement('import "C"')
        self.statement()
        self.statement('import "unsafe"')
        self.statement()
        self.statement()

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement()
            self.openScope('const (')

            def translateConstInit(struct, init):
                structBaseIdent = struct.name + '::'
                if init.startswith(structBaseIdent):
                    return struct.name + init[len(structBaseIdent):]
                else:
                    return init

            for struct in constStructs:
                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    declList.append(Translator.Declaration('', f'{struct.name}{field.name}', field.init))

                for decl in declList.decls:
                    self.statement(f'{decl.name}{declList.spaces(1, decl.name)} = {translateConstInit(struct, decl.init)}')

            self.closeScope(')')
            self.statement()

        # Write all enumerations
        if len(doc.enums) > 0:
            self.statement('/* ----- Enumerations ----- */')
            self.statement()

            for enum in doc.enums:
                self.statement(f'type {enum.name} int')
                self.openScope('const (')

                # Write enumeration entry declarations
                declList = Translator.DeclarationList()
                for field in enum.fields:
                    declList.append(Translator.Declaration('', f'{enum.name}{field.name}', field.init))

                isFirstDecl = True
                for decl in declList.decls:
                    typeSpecifier = f' {enum.name} = iota' if isFirstDecl else ''
                    #if decl.init:
                    #    self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= {decl.init}')
                    #else:
                    #    self.statement(f'{decl.name}{typeSpecifier}')
                    self.statement(f'{decl.name}{typeSpecifier}')
                    isFirstDecl = False

                self.closeScope(')')
                self.statement()

            self.statement()

        # Write all flags
        if len(doc.flags) > 0:
            def translateFlagInitializer(basename, init):
                s = init
                s = re.sub(r'([a-zA-Z_]\w*)', '{}{}'.format(basename, r'\1'), s)
                s = re.sub(r'(\||<<|>>|\+|\-|\*|\/)', r' \1 ', s)
                return s

            def translateFieldName(name):
                exceptions = [
                    ('CPUAccessReadWrite', None) # Identifier for LLGL::CPUAccessFlags::ReadWrite is already used for LLGL::CPUAccess::ReadWrite
                ]
                for exception in exceptions:
                    if name == exception[0]:
                        return exception[1]
                return name


            self.statement('/* ----- Flags ----- */')
            self.statement()

            for flag in doc.flags:
                self.statement(f'type {flag.name} int')
                basename = flag.name[:-len('Flags')]
                self.openScope('const (')

                # Write flag entry declarations
                declList = Translator.DeclarationList()
                for field in flag.fields:
                    fieldName = translateFieldName(f'{basename}{field.name}')
                    if fieldName:
                        declList.append(Translator.Declaration('', fieldName, translateFlagInitializer(basename, field.init) if field.init else None))

                isFirstDecl = True
                for decl in declList.decls:
                    typeSpecifier = f' {flag.name}' if isFirstDecl else ''
                    if decl.init:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= {decl.init}')
                    else:
                        self.statement(f'{decl.name}{typeSpecifier}')
                    isFirstDecl = False

                self.closeScope(')')
                self.statement()

            self.statement()

        # Write all structures
        commonStructs = list(filter(lambda record: not record.hasConstFieldsOnly(), doc.structs))

        if len(commonStructs) > 0:
            def translateStructField(fieldType, name):
                typeStr = ''
                declStr = ''

                # Write type specifier
                if fieldType.typename in LLGLMeta.stringClasses or (fieldType.baseType == StdType.CHAR and fieldType.isPointer):
                    typeStr = 'string'
                elif fieldType.isPointer and fieldType.baseType == StdType.VOID:
                    typeStr += 'unsafe.Pointer'
                else:
                    if fieldType.arraySize > 0:
                        typeStr += f'[{fieldType.arraySize}]'
                    elif fieldType.isDynamicArray():
                        typeStr += '[]'
                    elif fieldType.isPointer:
                        typeStr += '*'

                    if fieldType.baseType == StdType.CHAR:
                        typeStr += 'byte'
                    elif fieldType.baseType == StdType.LONG:
                        typeStr += 'uint'
                    elif fieldType.baseType == StdType.SIZE_T:
                        typeStr += 'uintptr'
                    elif fieldType.baseType == StdType.FLOAT:
                        typeStr += 'float32'
                    elif fieldType.baseType in [StdType.INT8, StdType.INT16, StdType.INT32, StdType.INT64, StdType.UINT8, StdType.UINT16, StdType.UINT32, StdType.UINT64]:
                        typeStr += toTypenameWithoutStdSuffix(fieldType.typename) # Remove '_t' suffix
                    else:
                        typeStr += fieldType.typename

                # Write field name; 'type' is reserved in Go, so rename it to its typename
                declStr = toPublicName(name)

                return (typeStr, declStr)

            def translateFieldInitializer(fieldType, init):
                if fieldType.isDynamicArray():
                    return 'nil'
                if init:
                    if init == 'nullptr':
                        return '""' if fieldType.isStringOfAnyKind() else 'nil'
                    else:
                        return re.sub(r'(\w+::)', r'\1', init).replace('::', '').replace('|', ' | ').replace('Flags', '').replace('.0f', '.0')
                return None

            self.statement('/* ----- Structures ----- */')
            self.statement()

            for struct in commonStructs:
                self.openScope(f'type {struct.name} struct ' + '{')

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    # Write two fields for dynamic arrays
                    externalCond = field.type.externalCond
                    if not externalCond:
                        declStr = translateStructField(field.type, field.name)
                        declList.append(
                            Translator.Declaration(
                                declStr[0],
                                declStr[1],
                                translateFieldInitializer(field.type, field.init),
                                inComment = translateDeprecationMessage(field.deprecated)))

                for decl in declList.decls:
                    if decl.directive:
                        self.statement(decl.directive)
                    elif decl.comment:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}{decl.type}{declList.spaces(0, decl.type)}/* {decl.comment} */')
                    elif decl.init:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}{decl.type}{declList.spaces(0, decl.type)}/* = {decl.init} */')
                    else:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}{decl.type}')
                self.closeScope()
                self.statement()

            self.statement()

        self.statement()
        self.statement()
        self.statement()
        self.statement('/* ================================================================================ */')
        self.statement()

