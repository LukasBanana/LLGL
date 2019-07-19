/*
 * GLObjectUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLObjectUtils.h"
#include "Ext/GLExtensions.h"
#include "RenderState/GLStateManager.h"
#include "../GLCommon/GLExtensionRegistry.h"
#include <string>
#include <cstring> // std::strlen


namespace LLGL
{


void GLSetObjectLabel(GLenum identifier, GLuint name, const char* label)
{
    #ifdef GL_KHR_debug
    if (HasExtension(GLExt::KHR_debug))
    {
        if (label != nullptr)
        {
            /* Set new label */
            const GLint         maxLength       = GLStateManager::GetCommonLimits().maxLabelLength;
            const std::size_t   actualLength    = std::strlen(label);
            const std::size_t   croppedLength   = std::min(actualLength, static_cast<std::size_t>(maxLength));
            glObjectLabel(identifier, name, croppedLength, label);
        }
        else
        {
            /* Reset label */
            glObjectLabel(identifier, name, 0, nullptr);
        }
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


} // /namespace LLGL



// ================================================================================
