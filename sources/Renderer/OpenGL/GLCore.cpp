/*
 * GLCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCore.h"
#include "../../Core/HelperMacros.h"
#include "Platform/MacOS/MacOSGLExt.h"
#include <sstream>


namespace LLGL
{


std::string GLErrorToStr(const GLenum errorCode)
{
    switch (errorCode)
    {
        LLGL_CASE_TO_STR( GL_NO_ERROR );
        LLGL_CASE_TO_STR( GL_INVALID_ENUM );
        LLGL_CASE_TO_STR( GL_INVALID_VALUE );
        LLGL_CASE_TO_STR( GL_INVALID_OPERATION );
        LLGL_CASE_TO_STR( GL_INVALID_FRAMEBUFFER_OPERATION );
        LLGL_CASE_TO_STR( GL_OUT_OF_MEMORY );
        LLGL_CASE_TO_STR( GL_STACK_OVERFLOW );
        LLGL_CASE_TO_STR( GL_STACK_UNDERFLOW );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_UNDEFINED );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_UNSUPPORTED );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE );
        LLGL_CASE_TO_STR( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS );
    }
    return "";
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

#undef LLGL_CASE_TO_STR


} // /namespace LLGL



// ================================================================================
