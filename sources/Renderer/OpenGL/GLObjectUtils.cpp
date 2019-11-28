/*
 * GLObjectUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLObjectUtils.h"
#include "Ext/GLExtensions.h"
#include "Ext/GLExtensionRegistry.h"
#include "RenderState/GLStateManager.h"
#include <string>
#include <cstring> // std::strlen


namespace LLGL
{


#ifdef GL_KHR_debug

// Returns the length of the specified label with a maximum length determined by GL_MAX_LABEL_LENGTH
static GLsizei GetCroppedLength(const char* label)
{
    const GLint         maxLength       = GLStateManager::GetCommonLimits().maxLabelLength;
    const std::size_t   actualLength    = std::strlen(label);
    const std::size_t   croppedLength   = std::min(actualLength, static_cast<std::size_t>(maxLength));
    return static_cast<GLsizei>(croppedLength);
}

#endif // /GL_KHR_debug

void GLSetObjectLabel(GLenum identifier, GLuint name, const char* label)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        if (label != nullptr)
            glObjectLabel(identifier, name, GetCroppedLength(label), label);
        else
            glObjectLabel(identifier, name, 0, nullptr);
    }
    #endif // /GL_KHR_debug
}

void GLSetObjectLabelSubscript(GLenum identifier, GLuint name, const char* label, const char* subscript)
{
    if (label != nullptr)
    {
        /* Append subscript to label */
        std::string labelWithSubscript = label;

        labelWithSubscript += '[';
        labelWithSubscript += subscript;
        labelWithSubscript += ']';

        GLSetObjectLabel(identifier, name, labelWithSubscript.c_str());
    }
    else
        GLSetObjectLabel(identifier, name, nullptr);
}

void GLSetObjectLabelIndexed(GLenum identifier, GLuint name, const char* label, std::uint32_t index)
{
    if (label != nullptr)
    {
        /* Append subscript to label */
        std::string subscript = std::to_string(index);
        GLSetObjectLabelSubscript(identifier, name, label, subscript.c_str());
    }
    else
        GLSetObjectLabel(identifier, name, nullptr);
}

void GLSetObjectPtrLabel(void* ptr, const char* label)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        if (label != nullptr)
            glObjectPtrLabel(ptr, GetCroppedLength(label), label);
        else
            glObjectPtrLabel(ptr, 0, nullptr);
    }
    #endif // /GL_KHR_debug
}


} // /namespace LLGL



// ================================================================================
