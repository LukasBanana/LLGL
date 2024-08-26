#
# llgl_translator_c99.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_translator import *

class C99Translator(Translator):
    def translateModule(self, doc):
        def translateDependency(inType):
            if inType.baseType in [StdType.BOOL]:
                return '<stdbool.h>', True
            elif inType.baseType in [StdType.INT8, StdType.INT16, StdType.INT32, StdType.INT64, StdType.UINT8, StdType.UINT16, StdType.UINT32, StdType.UINT64]:
                return '<stdint.h>', True
            elif inType.baseType in [StdType.SIZE_T]:
                return '<stddef.h>', True
            elif inType.baseType in [StdType.CHAR, StdType.WCHAR, StdType.LONG, StdType.FLOAT]:
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

        def translateDeprecationMessage(msg):
            if msg is not None:
                if msg.startswith('"') and msg.endswith('"'):
                    msg = msg[1:-1] # Remove quotation marks
                msg = msg.replace('LLGL::', 'LLGL')
                msg = msg.replace('::', '.')
                return msg
            return None

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

        # Write header guard
        headerGuardName = f'LLGL_C99{Translator.convertNameToHeaderGuard(doc.name)}_H'
        self.statement(f'#ifndef {headerGuardName}')
        self.statement(f'#define {headerGuardName}')
        self.statement()
        self.statement()

        # Write all include directives
        includeHeaders = translateIncludes(doc.typeDeps)
        if len(includeHeaders[0]) > 0 or len(includeHeaders[1]) > 0:
            for headers in includeHeaders:
                for inc in headers:
                    self.statement(f'#include {inc}')

            for external in LLGLMeta.externals:
                if external.cond and external.include:
                    self.statement()
                    self.statement(f'#if {external.cond}')
                    self.statement(f'#   include {external.include}')
                    self.statement(f'#endif /* {external.cond} */')

            self.statement()
            self.statement()

        # Write all constants
        constStructs = list(filter(lambda record: record.hasConstFieldsOnly(), doc.structs))

        if len(constStructs) > 0:
            self.statement('/* ----- Constants ----- */')
            self.statement()

            def translateConstFieldToMacroIdent(struct, fieldName):
                return f'LLGL_{struct.name.upper()}_{fieldName.upper()}'

            def translateConstInit(struct, init):
                structBaseIdent = struct.name + '::'
                if init.startswith(structBaseIdent):
                    return translateConstFieldToMacroIdent(struct, init[len(structBaseIdent):])
                else:
                    return init

            for struct in constStructs:
                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    declList.append(Translator.Declaration('', translateConstFieldToMacroIdent(struct, field.name), field.init))

                for decl in declList.decls:
                    self.statement(f'#define {decl.name}{declList.spaces(1, decl.name)} ( {translateConstInit(struct, decl.init)} )')
                self.statement()

            self.statement()

        # Write all enumerations
        sizedTypes = dict()

        if len(doc.enums) > 0:
            self.statement('/* ----- Enumerations ----- */')
            self.statement()

            for enum in doc.enums:
                if enum.base:
                    bitsize = enum.base.getFixedBitsize()
                    if bitsize > 0:
                        sizedTypes[enum.name] = bitsize

                self.statement(f'typedef enum LLGL{enum.name}')
                self.openScope()

                # Write enumeration entry declarations
                declList = Translator.DeclarationList()
                for field in enum.fields:
                    declList.append(Translator.Declaration('', f'LLGL{enum.name}{field.name}', field.init))

                for decl in declList.decls:
                    if decl.init:
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= {decl.init},')
                    else:
                        self.statement(f'{decl.name},')

                self.closeScope()
                self.statement(f'LLGL{enum.name};')
                self.statement()

            self.statement()

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
            self.statement()

            for flag in doc.flags:
                self.statement(f'typedef enum LLGL{flag.name}')
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
                        self.statement(f'{decl.name}{declList.spaces(1, decl.name)}= {decl.init},')
                    else:
                        self.statement(f'{decl.name},')

                self.closeScope()
                self.statement(f'LLGL{flag.name};')
                self.statement()

            self.statement()

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
                    if fieldType.baseType == StdType.STRUCT:
                        typeStr += 'struct ' if fieldType.externalCond else 'LLGL'
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
            self.statement()

            for struct in commonStructs:
                self.statement(f'typedef struct LLGL{struct.name}')
                self.openScope()

                # Write struct field declarations
                declList = Translator.DeclarationList()
                for field in struct.fields:
                    # Write two fields for dynamic arrays
                    externalCond = field.type.externalCond
                    if externalCond:
                        declList.append(Translator.Declaration(inDirective = f'#if {externalCond}'))
                    if field.type.isDynamicArray():
                        declList.append(Translator.Declaration('size_t', f'num{field.name[0].upper()}{field.name[1:]}', '0'))
                    declStr = translateStructField(field.type, field.name)
                    declList.append(
                        Translator.Declaration(
                            declStr[0],
                            declStr[1],
                            translateFieldInitializer(field.type, field.init),
                            inComment = translateDeprecationMessage(field.deprecated)))
                    if externalCond:
                        declList.append(Translator.Declaration(inDirective = f'#endif /* {externalCond} */'))

                for decl in declList.decls:
                    if decl.directive:
                        self.statement(decl.directive)
                    elif decl.comment:
                        self.statement(f'{decl.type}{declList.spaces(0, decl.type)}{decl.name};{declList.spaces(1, decl.name)}/* {decl.comment} */')
                    elif decl.init:
                        self.statement(f'{decl.type}{declList.spaces(0, decl.type)}{decl.name};{declList.spaces(1, decl.name)}/* = {decl.init} */')
                    else:
                        self.statement(f'{decl.type}{declList.spaces(0, decl.type)}{decl.name};')
                self.closeScope()
                self.statement(f'LLGL{struct.name};')
                self.statement()

            self.statement()

        self.statement(f'#endif /* {headerGuardName} */')
        self.statement()
        self.statement()
        self.statement()
        self.statement('/* ================================================================================ */')
        self.statement()

