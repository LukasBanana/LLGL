/*
 * GLCore.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLCore.h"
#include "../../Core/Exception.h"
#include "../../Core/StringUtils.h"
#include "../../Core/MacroUtils.h"
#include <stdexcept>


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

static const char* GLErrorToStrOrHex(const GLenum status)
{
    if (const char* err = GLErrorToStr(status))
        return err;
    else
        return IntToHex(status);
}

void GLThrowIfFailed(const GLenum status, const GLenum statusRequired, const char* info)
{
    if (status != statusRequired)
    {
        const char* err = GLErrorToStrOrHex(status);
        LLGL_TRAP("%s (error code = %s)", (info != nullptr ? info : "OpenGL operation failed"), err);
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

// Helper structure to query major/minor versions only once
struct GLVersion
{
    GLVersion()
    {
        #if defined(GL_MAJOR_VERSION) && defined(GL_MINOR_VERSION)
        GLint major = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        GLint minor = 0;
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        this->no = major * 100 + minor * 10;
        #else
        this->no = 200;
        #endif
    }
    int no;
};

int GLGetVersion()
{
    static GLVersion version;
    return version.no;
}

[[noreturn]]
void ErrUnsupportedGLProc(const char* name)
{
    #if defined(LLGL_OPENGL)
    LLGL_TRAP("illegal use of unsupported OpenGL procedure: %s", name);
    #elif defined(LLGL_OS_WASM)
    LLGL_TRAP("illegal use of unsupported OpenGLES procedure: %s", name);
    #else
    LLGL_TRAP("illegal use of unsupported WebGL procedure: %s", name);
    #endif
}

#undef LLGL_CASE_TO_STR


} // /namespace LLGL



// ================================================================================
