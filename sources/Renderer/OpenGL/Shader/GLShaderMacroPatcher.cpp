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

// Returns the source position after the '#version'-directive. This is always at the beginning of a new line.
static std::size_t FindEndOfVersionDirective(const char* source)
{
    const char* s = source;
    while (*s != '\0')
    {
        if (s[0] == '/' && s[1] == '/')
        {
            /* Ignore single line comment */
            s += 2;
            while (*s != '\n')
                ++s;
        }
        else if (s[0] == '/' && s[1] == '*')
        {
            /* Ignore multi line comment */
            s += 2;
            while (s[0] == '*' && s[1] == '/')
                ++s;
        }
        else if (s[0] == '#')
        {
            /* Parse '#version'-directive */
            ++s;

            /* Ignore whitespaces after '#' token */
            while (IsWhitespace(*s))
                ++s;

            /* Scan for "version" token */
            if (::strncmp(s, "version", 7) == 0)
            {
                s += 7;

                /* There has to be at least one more whitespace */
                if (IsWhitespace(*s))
                {
                    /* Jump to end of line */
                    for (++s; *s != '\0'; ++s)
                    {
                        if (*s == '\n')
                            return static_cast<std::size_t>(s - source) + 1;
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
    source_          { source                            },
    posAfterVersion_ { FindEndOfVersionDirective(source) }
{
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
        if (posAfterVersion_ != std::string::npos)
            source_.insert(posAfterVersion_, defineCode);
        else
            source_ = defineCode + source_;
    }
}


} // /namespace LLGL



// ================================================================================
