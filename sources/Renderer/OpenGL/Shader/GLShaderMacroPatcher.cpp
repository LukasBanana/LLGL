/*
 * GLShaderMacroPatcher.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderMacroPatcher.h"
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

// Returns the source position after the '#version'-directive. This is always at the beginning of a new line.
static std::size_t FindEndOfVersionDirective(const char* source)
{
    const char* s = source;
    while (*s != '\0')
    {
        if (ScanToken(s, "//"))
        {
            /* Ignore single line comment */
            while (!ScanToken(s, '\n'))
                ++s;
        }
        else if (ScanToken(s, "/*"))
        {
            /* Ignore multi line comment */
            while (!ScanToken(s, '*', '/'))
                ++s;
        }
        else if (ScanToken(s, "#"))
        {
            /* Ignore whitespaces after '#' token */
            while (IsWhitespace(*s))
                ++s;

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

GLShaderMacroPatcher::GLShaderMacroPatcher(const char* source) :
    source_ { source }
{
    const auto posAfterVersion = FindEndOfVersionDirective(source);
    if (posAfterVersion != std::string::npos)
        macroInsertPos_ = posAfterVersion;
}

void GLShaderMacroPatcher::AddDefines(const ShaderMacro* defines)
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
        source_.insert(macroInsertPos_, defineCode);
        macroInsertPos_ += defineCode.size();
    }
}


} // /namespace LLGL



// ================================================================================
