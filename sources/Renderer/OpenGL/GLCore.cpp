/*
 * GLCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCore.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"
#include <sstream>


namespace LLGL
{


static const char* GLErrorToStr(const GLenum status)
{
    switch (status)
    {
        LLGL_CASE_TO_STR( GL_NO_ERROR );
        LLGL_CASE_TO_STR( GL_INVALID_ENUM );
        LLGL_CASE_TO_STR( GL_INVALID_VALUE );
        LLGL_CASE_TO_STR( GL_INVALID_OPERATION );
        LLGL_CASE_TO_STR( GL_INVALID_FRAMEBUFFER_OPERATION );
        LLGL_CASE_TO_STR( GL_OUT_OF_MEMORY );
        #if defined LLGL_OPENGL && !defined __APPLE__
        LLGL_CASE_TO_STR( GL_STACK_OVERFLOW );
        LLGL_CASE_TO_STR( GL_STACK_UNDERFLOW );
        #endif
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_UNDEFINED );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT );
        #ifdef LLGL_OPENGL
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );
        #endif
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_UNSUPPORTED );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE );
        #ifdef LLGL_OPENGL
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS );
        #endif
    }
    return nullptr;
}

void GLThrowIfFailed(const GLenum status, const GLenum statusRequired, const char* info)
{
    if (status != statusRequired)
    {
        std::string s;

        if (info)
        {
            s += info;
            s += " (error code = ";
        }
        else
            s += "OpenGL operation failed (error code = ";

        if (auto err = GLErrorToStr(status))
            s += err;
        else
        {
            s += "0x";
            s += ToHex(status);
        }

        s += ")";

        throw std::runtime_error(s);
    }
}

const char* GLDebugSourceToStr(const GLenum source)
{
    #if defined LLGL_OPENGL && !defined __APPLE__
    switch (source)
    {
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_API );
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_WINDOW_SYSTEM );
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_SHADER_COMPILER );
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_THIRD_PARTY );
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_APPLICATION );
        LLGL_CASE_TO_STR( GL_DEBUG_SOURCE_OTHER );
    }
    #endif
    return "";
}

const char* GLDebugTypeToStr(const GLenum type)
{
    #if defined LLGL_OPENGL && !defined __APPLE__
    switch (type)
    {
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_ERROR );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_PORTABILITY );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_PERFORMANCE );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_MARKER );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_PUSH_GROUP );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_POP_GROUP );
        LLGL_CASE_TO_STR( GL_DEBUG_TYPE_OTHER );
    }
    #endif
    return "";
}

const char* GLDebugSeverityToStr(const GLenum severity)
{
    #if defined LLGL_OPENGL && !defined __APPLE__
    switch (severity)
    {
        LLGL_CASE_TO_STR( GL_DEBUG_SEVERITY_HIGH );
        LLGL_CASE_TO_STR( GL_DEBUG_SEVERITY_MEDIUM );
        LLGL_CASE_TO_STR( GL_DEBUG_SEVERITY_LOW );
        LLGL_CASE_TO_STR( GL_DEBUG_SEVERITY_NOTIFICATION );
    }
    #endif
    return "";
}

GLboolean GLBoolean(bool value)
{
    return (value ? GL_TRUE : GL_FALSE);
}

// Parse single integer from 'glGetString(GL_VERSION)'.
static bool GLParseInt(const GLubyte*& s, GLint& n)
{
    if (*s >= '0' && *s <= '9')
    {
        for (n = 0; *s >= '0' && *s <= '9'; ++s)
        {
            n *= 10;
            n += (*s - '0');
        }
        return true;
    }
    return false;
}

bool GLParseVersionString(const GLubyte* s, GLint& major, GLint& minor)
{
    if (s != nullptr)
    {
        /* Try to parse string with temporary integers */
        GLint n[2];
        if (!GLParseInt(s, n[0]))
            return false;
        if (*s++ != '.')
            return false;
        if (!GLParseInt(s, n[1]))
            return false;

        /* Return result */
        major = n[0];
        minor = n[1];

        return true;
    }
    return false;
}

[[noreturn]]
void ErrUnsupportedGLProc(const char* name)
{
    #ifdef LLGL_OPENGLES3
    throw std::runtime_error("illegal use of unsupported OpenGLES procedure: " + std::string(name));
    #else
    throw std::runtime_error("illegal use of unsupported OpenGL procedure: " + std::string(name));
    #endif
}

#undef LLGL_CASE_TO_STR


} // /namespace LLGL



// ================================================================================
