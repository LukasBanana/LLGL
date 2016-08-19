/*
 * GLCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLCore.h"
#include <sstream>


namespace LLGL
{


#define CASE_TO_STR(STR) case STR: return #STR

std::string GLErrorToStr(const GLenum errorCode)
{
    switch (errorCode)
    {
        CASE_TO_STR(GL_NO_ERROR);
        CASE_TO_STR(GL_INVALID_ENUM);
        CASE_TO_STR(GL_INVALID_VALUE);
        CASE_TO_STR(GL_INVALID_OPERATION);
        CASE_TO_STR(GL_INVALID_FRAMEBUFFER_OPERATION);
        CASE_TO_STR(GL_OUT_OF_MEMORY);
        #ifndef __APPLE__
        CASE_TO_STR(GL_STACK_OVERFLOW);
        CASE_TO_STR(GL_STACK_UNDERFLOW);
        #endif
        CASE_TO_STR(GL_FRAMEBUFFER_UNDEFINED);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        CASE_TO_STR(GL_FRAMEBUFFER_UNSUPPORTED);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
        CASE_TO_STR(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS);
    }
    return "";
}

std::string GLDebugSourceToStr(const GLenum source)
{
    #ifndef __APPLE__
    switch (source)
    {
        CASE_TO_STR(GL_DEBUG_SOURCE_API);
        CASE_TO_STR(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
        CASE_TO_STR(GL_DEBUG_SOURCE_SHADER_COMPILER);
        CASE_TO_STR(GL_DEBUG_SOURCE_THIRD_PARTY);
        CASE_TO_STR(GL_DEBUG_SOURCE_APPLICATION);
        CASE_TO_STR(GL_DEBUG_SOURCE_OTHER);
    }
    #endif
    return "";
}

std::string GLDebugTypeToStr(const GLenum type)
{
    #ifndef __APPLE__
    switch (type)
    {
        CASE_TO_STR(GL_DEBUG_TYPE_ERROR);
        CASE_TO_STR(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
        CASE_TO_STR(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
        CASE_TO_STR(GL_DEBUG_TYPE_PORTABILITY);
        CASE_TO_STR(GL_DEBUG_TYPE_PERFORMANCE);
        CASE_TO_STR(GL_DEBUG_TYPE_MARKER);
        CASE_TO_STR(GL_DEBUG_TYPE_PUSH_GROUP);
        CASE_TO_STR(GL_DEBUG_TYPE_POP_GROUP);
        CASE_TO_STR(GL_DEBUG_TYPE_OTHER);
    }
    #endif
    return "";
}

std::string GLDebugSeverityToStr(const GLenum severity)
{
    #ifndef __APPLE__
    switch (severity)
    {
        CASE_TO_STR(GL_DEBUG_SEVERITY_HIGH);
        CASE_TO_STR(GL_DEBUG_SEVERITY_MEDIUM);
        CASE_TO_STR(GL_DEBUG_SEVERITY_LOW);
        CASE_TO_STR(GL_DEBUG_SEVERITY_NOTIFICATION);
    }
    #endif
    return "";
}

GLboolean GLBoolean(bool value)
{
    return (value ? GL_TRUE : GL_FALSE);
}

#undef CASE_TO_STR


} // /namespace LLGL



// ================================================================================
