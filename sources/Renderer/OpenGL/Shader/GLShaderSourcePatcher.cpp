/*
 * GLShaderSourcePatcher.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLShaderSourcePatcher.h"
#include <LLGL/ShaderFlags.h>
#include <string.h>


namespace LLGL
{


static bool IsWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

static bool ScanToken(const char*& s, char tokenChar)
{
    if (s[0] == tokenChar)
    {
        s += 1;
        return true;
    }
    return false;
}

static bool ScanToken(const char*& s, char tokenChar0, char tokenChar1)
{
    if (s[0] == tokenChar0 && s[1] == tokenChar1)
    {
        s += 2;
        return true;
    }
    return false;
}

static bool ScanToken(const char*& s, const char* token)
{
    const auto tokenLen = ::strlen(token);
    if (::strncmp(s, token, tokenLen) == 0)
    {
        s += tokenLen;
        return true;
    }
    return false;
}

static void SkipUntilToken(const char*& s, char tokenChar)
{
    while (*s != '\0' && !ScanToken(s, tokenChar))
        ++s;
}

static void SkipUntilToken(const char*& s, char tokenChar0, char tokenChar1)
{
    while (*s != '\0' && !ScanToken(s, tokenChar0, tokenChar1))
        ++s;
}

static void SkipWhitespaces(const char*& s)
{
    while (IsWhitespace(*s))
        ++s;
}

static bool SkipComment(const char*& s)
{
    if (ScanToken(s, "//"))
    {
        /* Ignore single line comment */
        SkipUntilToken(s, '\n');
        return true;
    }
    else if (ScanToken(s, "/*"))
    {
        /* Ignore multi line comment */
        SkipUntilToken(s, '*', '/');
        return true;
    }
    return false;
}

// Returns the source position that starts with the '#' character beginning the search from 'off' backwards.
static std::size_t FindStartOfDirective(const char* source, std::size_t off)
{
    do
    {
        if (source[off] == '#')
            return off;
    }
    while (off-- > 0);
    return std::string::npos;
}

// Returns the source position after the '#version'-directive. This is always at the beginning of a new line.
static std::size_t FindEndOfVersionDirective(const char* source)
{
    const char* s = source;
    while (*s != '\0')
    {
        if (SkipComment(s))
        {
            /* Ignore comments */
            continue;
        }
        else if (ScanToken(s, "#"))
        {
            /* Ignore whitespaces after '#' token */
            SkipWhitespaces(s);

            /* Scan for "version" token */
            if (ScanToken(s, "version"))
            {
                /* There has to be at least one more whitespace */
                if (IsWhitespace(*s))
                {
                    /* Jump to end of line */
                    for (++s; *s != '\0'; ++s)
                    {
                        if (ScanToken(s, '\n'))
                            return static_cast<std::size_t>(s - source);
                    }
                }
            }
        }
        else
        {
            /* Move to next character */
            ++s;
        }
    }
    return std::string::npos;
}

GLShaderSourcePatcher::GLShaderSourcePatcher(const char* source) :
    source_ { source }
{
    const auto posAfterVersion = FindEndOfVersionDirective(source);
    if (posAfterVersion != std::string::npos)
        statementInsertPos_ = posAfterVersion;
}

static std::string GenerateGlslVersionDirective(const char* version)
{
    std::string s = "#version ";
    s += version;
    s += '\n';
    return s;
}

void GLShaderSourcePatcher::OverrideVersion(const char* version)
{
    /* First remove current '#version'-directive (in case new one must be at first source line) */
    const std::size_t startOfVersionDirective = FindStartOfDirective(source_.c_str(), statementInsertPos_);
    if (startOfVersionDirective != std::string::npos)
        source_.erase(startOfVersionDirective, statementInsertPos_ - startOfVersionDirective);

    /* Generate new '#version'-directive */
    const std::string newVersionDirective = GenerateGlslVersionDirective(version);
    source_.insert(0, newVersionDirective);

    /* Replace old insertion point for statements after new directive */
    ResetInsertionPoints(newVersionDirective.size());
}

void GLShaderSourcePatcher::AddDefines(const ShaderMacro* defines)
{
    if (defines != nullptr)
    {
        /* Generate macro definition code */
        std::string defineCode;

        for (; defines->name != nullptr; ++defines)
        {
            defineCode += "#define ";
            defineCode += defines->name;
            if (defines->definition != nullptr)
            {
                defineCode += ' ';
                defineCode += defines->definition;
            }
            defineCode += '\n';
        }

        /* Insert macro definitions into source */
        InsertAfterVersionDirective(defineCode);
    }
}

void GLShaderSourcePatcher::AddPragmaDirective(const char* statement)
{
    if (statement != nullptr && *statement != '\0')
    {
        /* Generate '#pragma'-directive code and insert into source */
        std::string pragmaCode = "#pragma ";
        pragmaCode += statement;
        pragmaCode += '\n';
        InsertAfterVersionDirective(pragmaCode);
    }
}

struct SourceScanState
{
    std::string patched;
    std::size_t codeBlockDepth          = 0;
    const char* head                    = nullptr;
    const char* lastPatched             = nullptr;
    const char* lastToken               = nullptr;
    const char* lastIndentRange[2]      = {};
    const char* currentIndentRange[2]   = {};

    SourceScanState(const char* source, std::size_t entryPointStartPos) :
        head               { source + entryPointStartPos },
        lastPatched        { source                      },
        currentIndentRange { head, nullptr               }
    {
    }

    void Append(const char* to)
    {
        patched.append(lastPatched, to);
        lastPatched = to;
    }

    void AppendStatement(const char* statement, bool currentIndent = false)
    {
        /* Append input source from last patched position to end of previous line (before current indentation start) */
        Append(currentIndentRange[0]);

        /* If last token was not the end of the current indentation range, also add everything to that last token */
        if (lastToken != currentIndentRange[1])
            Append(lastToken);

        /* Append indentation from previous or current line */
        if (currentIndent)
            patched.append(currentIndentRange[0], currentIndentRange[1] - currentIndentRange[0]);
        else
            patched.append(lastIndentRange[0], lastIndentRange[1] - lastIndentRange[0]);

        /* Append statement with newline */
        patched += statement;
        patched += '\n';
    }

    void AppendRemainder()
    {
        /* Append remaining source until current head position */
        patched.append(lastPatched, head);
        lastPatched = head;
    }
};

void GLShaderSourcePatcher::AddFinalVertexTransformStatements(const char* statement)
{
    if (statement != nullptr && *statement != '\0')
    {
        CacheEntryPointSourceLocation();

        /* Scan source from entry point start location */
        SourceScanState scan{ GetSource(), entryPointStartPos_ };

        for (auto& s = scan.head; *s != '\0'; scan.lastToken = scan.head)
        {
            if (SkipComment(s))
            {
                /* Ignore comments */
                continue;
            }
            else if (scan.currentIndentRange[0] > scan.currentIndentRange[1])
            {
                /* Record end of current indentation */
                if (IsWhitespace(*s))
                {
                    SkipWhitespaces(s);
                    scan.currentIndentRange[1] = scan.head;
                }
                else
                    scan.currentIndentRange[1] = scan.currentIndentRange[0];
            }
            else if (ScanToken(s, '\n'))
            {
                /* Record previous indentation range */
                scan.lastIndentRange[0] = scan.currentIndentRange[0];
                scan.lastIndentRange[1] = scan.currentIndentRange[1];

                /* Record new indentation range */
                scan.currentIndentRange[0] = scan.head;
            }
            else if (ScanToken(s, '{'))
            {
                /* Record stepping into a code block */
                scan.codeBlockDepth++;
            }
            else if (ScanToken(s, '}'))
            {
                /* Record stepping out of a code block and write last statement as we left the main entry point */
                scan.codeBlockDepth--;
                if (scan.codeBlockDepth == 0)
                {
                    scan.AppendStatement(statement);
                    break;
                }
            }
            else if (ScanToken(s, "return"))
            {
                /* Append vertex transform statement before return statement */
                scan.AppendStatement(statement, true);
            }
            else
            {
                /* Move to next character */
                ++s;
            }
        }

        /* Replace source with finalized patched source */
        scan.AppendRemainder();
        source_ = std::move(scan.patched);
    }
}


/*
 * ======= Private: =======
 */

void GLShaderSourcePatcher::InsertAfterVersionDirective(const std::string& statement)
{
    /* Insert statement into source after version */
    source_.insert(statementInsertPos_, statement);

    /* Move source location for next insertion at the end of newly added code */
    ResetInsertionPoints(statementInsertPos_ + statement.size());
}

static std::size_t FindEntryPointSourceLocation(const char* source, std::size_t start)
{
    const char* s = source + start;
    while (*s != '\0')
    {
        if (ScanToken(s, "//"))
        {
            /* Ignore single line comment */
            SkipUntilToken(s, '\n');
        }
        else if (ScanToken(s, "/*"))
        {
            /* Ignore multi line comment */
            SkipUntilToken(s, '*', '/');
        }
        else if (ScanToken(s, "void"))
        {
            /* Store position at start of current token */
            const auto startPos = static_cast<std::size_t>(s - source - 4);

            /* Ignore whitespaces after 'void' token */
            SkipWhitespaces(s);

            /* Scan for "main" identifier */
            if (ScanToken(s, "main"))
            {
                SkipWhitespaces(s);
                if (ScanToken(s, '('))
                {
                    SkipWhitespaces(s);
                    if (ScanToken(s, ')'))
                    {
                        /* Entry point found => return start position of main function */
                        return startPos;
                    }
                }
            }
        }
        else
        {
            /* Move to next character */
            ++s;
        }
    }
    return std::string::npos;
}

void GLShaderSourcePatcher::CacheEntryPointSourceLocation()
{
    if (entryPointStartPos_ == std::string::npos)
        entryPointStartPos_ = FindEntryPointSourceLocation(GetSource(), statementInsertPos_);
}

void GLShaderSourcePatcher::ResetInsertionPoints(std::size_t newStatementInsertPos)
{
    if (newStatementInsertPos > statementInsertPos_)
    {
        if (entryPointStartPos_ != std::string::npos)
            entryPointStartPos_ += (newStatementInsertPos - statementInsertPos_);
        statementInsertPos_ = newStatementInsertPos;
    }
    else if (newStatementInsertPos < statementInsertPos_)
    {
        if (entryPointStartPos_ != std::string::npos)
            entryPointStartPos_ -= (statementInsertPos_ - newStatementInsertPos);
        statementInsertPos_ = newStatementInsertPos;
    }
}


} // /namespace LLGL



// ================================================================================
