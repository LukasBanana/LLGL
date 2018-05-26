/*
 * GLCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
        #ifndef __APPLE__
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

std::string GLDebugSourceToStr(const GLenum source)
{
    #ifndef __APPLE__
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

std::string GLDebugTypeToStr(const GLenum type)
{
    #ifndef __APPLE__
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

std::string GLDebugSeverityToStr(const GLenum severity)
{
    #ifndef __APPLE__
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

[[noreturn]]
void ErrUnsupportedGLProc(const char* name)
{
    throw std::runtime_error("illegal use of unsupported OpenGL procedure: " + std::string(name));
}

#undef LLGL_CASE_TO_STR


} // /namespace LLGL



// ================================================================================
