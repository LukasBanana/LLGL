#
# llgl_translator.py
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

from llgl_parser import *

class Translator:
    indent = 0
    tabSize = 4

    class Declaration:
        type = ''
        name = ''
        init = None
        directive = None
        comment = None

        def __init__(self, inType = '', inName = '', inInit = None, inDirective = None, inComment = None):
            self.type = inType
            self.name = inName
            self.init = inInit
            self.directive = inDirective
            self.comment = inComment

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

    def statement(self, line = ''):
        if len(line) == 0:
            print('')
        elif len(line) > 0 and line[0] == '#':
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

