#
# llgl_parser.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

import os
import sys
import re
from llgl_module import *

def fatal(msg):
    print(sys.argv[0] + ': ' + msg)
    sys.exit(1)

class DebugContext:
    record = ''
    func = ''
    isOper = False
    isCtor = False

    def __init__(self):
        self.record = ''
        self.func = ''
        self.isOper = False
        self.isCtor = False

    def __str__(self):
        info = ''
        def appendInfo(newInfo):
            nonlocal info
            if len(info) > 0:
                info += ', '
            info += newInfo

        if self.record != '':
            appendInfo(f"record = '{self.record}'")
        if self.func != '':
            if self.isOper:
                appendInfo(f"operator '{self.func}'")
            elif self.isCtor:
                appendInfo(f"constructor = '{self.func}'")
            else:
                appendInfo(f"function = '{self.func}'")

        return info

    def reset(self, record = None, func = None, oper = None, ctor = None):
        if record != None:
            self.record = record
        if func != None:
            self.func = func
            self.isOper = False
            self.isCtor = False
        elif oper != None:
            self.func = oper
            self.isOper = True
            self.isCtor = False
        elif ctor != None:
            self.func = ctor
            self.isOper = False
            self.isCtor = True

class Scanner:
    filename = ''
    tokens = []
    readPos = 0
    context = DebugContext()

    def __init__(self):
        self.filename = ''
        self.tokens = []
        self.readPos = 0
        self.context = DebugContext()

    def good(self):
        return self.readPos < len(self.tokens)

    @staticmethod
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
                return re.findall(r'//[^\n]*|""|"[^"]+"|[a-zA-Z_]\w*|\d+\.\d+[fF]|\d+[uU]|\d+|[{}\[\]]|::|:|\.\.\.|<<|>>|->|\+=|-=|[+-=,;<>\|~]|[*]|[(]|[)]', text)
        except UnicodeDecodeError:
            fatal('UnicodeDecodeError exception while reading file: ' + filename)
        return None

    @staticmethod
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

    def scan(self, filename):
        self.filename = filename
        self.tokens = Scanner.reduceTokens(Scanner.scanTokens(filename))

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

    def acceptIfAny(self, search):
        for tok in search:
            if self.acceptIf(tok):
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
            contextInfo = str(self.context)
            contextInfo = f' [{contextInfo}]' if contextInfo != '' else ''
            fatal(f"{self.filename}: error: expected token '{search}', but got '{self.tok()}'; predecessors: {self.tokens[self.readPos - 5:self.readPos]}{contextInfo}")

    def ignoreUntil(self, search):
        while self.good():
            if self.acceptIf(search):
                break
            self.accept()

    def acceptBlockOrFail(self, openTok = '{', closeTok = '}'):
        self.acceptOrFail(openTok)
        while self.good():
            if self.match(openTok):
                self.acceptBlockOrFail(openTok, closeTok)
            elif self.match(closeTok):
                break
            else:
                self.accept()
        self.acceptOrFail(closeTok)

class Parser:
    scanner = None

    def __init__(self):
        self.scanner = Scanner()

    def tryParseDeprecated(self):
        while self.scanner.acceptIfAny(['LLGL_DEPRECATED_IGNORE_PUSH', 'LLGL_DEPRECATED_IGNORE_POP']):
            self.scanner.acceptOrFail('(')
            self.scanner.ignoreUntil(')')
        if self.scanner.acceptIf('LLGL_DEPRECATED'):
            self.scanner.acceptOrFail('(')
            msg = self.scanner.accept()
            self.scanner.ignoreUntil(')')
            return msg
        return None

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
        elif self.scanner.acceptIf('...'):
            return LLGLType('...');
        else:
            isConst = self.scanner.acceptIf('const')
            typename = self.scanner.accept()
            isConst = self.scanner.acceptIf('const') or isConst
            if typename in LLGLMeta.containers and self.scanner.acceptIf('<'):
                isConst = self.scanner.acceptIf('const') or isConst
                typename = self.scanner.accept()
                isPointer = self.scanner.acceptIfAny(['*', '&'])
                self.scanner.acceptOrFail('>')
                outType = LLGLType(typename, isConst, isPointer)
                outType.setArraySize(LLGLType.DYNAMIC_ARRAY)
                return outType
            else:
                isPointer = self.scanner.acceptIfAny(['*', '&'])
                return LLGLType(typename, isConst, isPointer)

    def parseStructMembers(self, structName):
        members = []

        self.scanner.context.reset(record = structName)

        while self.scanner.tok() != '}':
            deprecated = self.tryParseDeprecated()

            # Ignore unions inside structs
            if self.scanner.acceptIf('union'):
                self.scanner.acceptBlockOrFail()
                self.scanner.acceptOrFail(';')
                continue

            fieldType = self.parseType()
            isCtor = fieldType.typename == structName
            isOper = self.scanner.tok() == 'operator'
            isFunc = self.scanner.tok(1) == '('

            self.scanner.context.reset(
                ctor = structName if isCtor else None,
                oper = self.scanner.tok(1) if isOper else None,
                func = self.scanner.tok() if isFunc else None)

            if isCtor or isOper or isFunc:
                # Ignore operators
                if isOper:
                    self.scanner.accept(2)
                elif isFunc:
                    self.scanner.context.currentFunc = self.scanner.accept()

                # Ingore constructs
                self.scanner.acceptOrFail('(')
                self.scanner.ignoreUntil(')')
                self.scanner.acceptIf('noexcept')
                if self.scanner.acceptIf(':'):
                    # Ignore initializer list
                    while self.scanner.good():
                        self.scanner.accept() # Member
                        self.scanner.acceptBlockOrFail()
                        if not self.scanner.acceptIf(','):
                            break

                    # Ignore c'tor body
                    self.scanner.acceptBlockOrFail()
                elif self.scanner.match('{'):
                    # Ignore function body
                    self.scanner.acceptBlockOrFail()
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
                member.deprecated = deprecated
                members.append(member)
                self.scanner.acceptOrFail(';')

        return members

    def parseAnnotationArgument(self):
        if self.scanner.acceptIf('NULL'):
            return LLGLAnnotation.NULLABLE
        elif self.scanner.acceptIf('['):
            self.scanner.accept()
            self.scanner.acceptOrFail(']')
            return LLGLAnnotation.ARRAY
        else:
            fatal(f"error: unknwon annotation argument '{self.scanner.tok()}'")
        return LLGLAnnotation.UNDEFINED

    def parseParameter(self):
        # Only parse return type name parameter name as C does not support default arguments
        paramType = self.parseType()

        paramName = self.scanner.accept() if paramType.baseType != StdType.VARGS else ''

        param = LLGLField(paramName, paramType)

        # Parse optional fixed size array
        if self.scanner.acceptIf('['):
            param.type.setArraySize(self.scanner.accept())
            self.scanner.acceptOrFail(']')

        # Parse optional annotations
        if self.scanner.acceptIf('LLGL_ANNOTATE'):
            self.scanner.acceptOrFail('(')
            while self.scanner.good():
                param.annotations.append(self.parseAnnotationArgument())
                if not self.scanner.acceptIf(','):
                    break
            self.scanner.acceptOrFail(')')

        return param

    def parseParameterList(self):
        params = []

        self.scanner.acceptOrFail('(')
        if not self.scanner.match(')'):
            if self.scanner.match(['void', ')']):
                # Ignore explicit empty parameter list
                self.scanner.accept()
            else:
                # Parse parameters until no more ',' is scanned
                while True:
                    params.append(self.parseParameter())
                    if not self.scanner.acceptIf(','):
                        break
        self.scanner.acceptOrFail(')')

        return params

    def parseFunctionDecl(self):
        # Parse return type
        returnType = self.parseType()

        # Parse function name
        name = self.scanner.accept()

        # Parse parameter list
        func = LLGLFunction(name, returnType)

        func.params = self.parseParameterList()
        self.scanner.acceptOrFail(';')

        return func

    def parseDelegateDecl(self, returnType):
        # Parse delegate name
        self.scanner.acceptOrFail(['(', '*'])
        name = self.scanner.accept()
        self.scanner.acceptOrFail(')')

        delegate = LLGLFunction(name, returnType)

        # Parse parameter list
        delegate.params = self.parseParameterList()
        self.scanner.acceptOrFail(';')

        return delegate

    # Parses input file by filename and returns LLGLModule
    def parseHeader(self, filename, processFunctions = False):
        mod = LLGLModule()
        mod.name = os.path.splitext(os.path.basename(filename))[0]

        self.scanner.scan(filename)

        while self.scanner.good():
            if processFunctions and self.scanner.acceptIf('LLGL_C_EXPORT'):
                # Parse function declaration
                mod.funcs.append(self.parseFunctionDecl())
            elif self.scanner.acceptIf('typedef') and not self.scanner.match('struct'):
                # Parse type alias
                typeDecl = self.parseType()

                if self.scanner.match('('):
                    # Parse delegate delcaration
                    mod.delegates.append(self.parseDelegateDecl(typeDecl))
                else:
                    # Ignore type definition
                    self.scanner.ignoreUntil(';')
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
                ignoreRecord = self.tryParseDeprecated() is not None
                
                # Parse record name and trim 'LLGL' prefix (occurs in custom structs of C99 wrapper such as LLGLWindowEventListener)
                name = self.scanner.accept()
                if name.startswith('LLGL'):
                    name = name[len('LLGL'):]

                # Parse optional inheritance
                inheritedFields = []
                if self.scanner.acceptIf(':'):
                    baseName = self.scanner.accept()
                    baseRecord = mod.findStructByName(baseName)
                    if baseRecord:
                        inheritedFields = baseRecord.fields
                    else:
                        fatal(f'failed to find base record "{baseName}" when parsing struct "{name}"')

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
                    struct.fields = inheritedFields + self.parseStructMembers(name)
                    if not ignoreRecord:
                        mod.structs.append(struct)
                self.scanner.acceptOrFail('}')
            else:
                self.scanner.accept()

        return mod

