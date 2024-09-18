/*
 * GLSharedContextVertexArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLSharedContextVertexArray.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../GLObjectUtils.h"
#include "../../../Core/Assertion.h"
#include "../Platform/GLContext.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


void GLSharedContextVertexArray::BuildVertexLayout(GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeVAO())
    {
        /* Bind vertex array for OpenGL 2.x */
        BuildVertexLayoutForGL2X(bufferID, attributes);
    }
    else
    #endif
    {
        #if LLGL_GL3PLUS_SUPPORTED
        BuildVertexLayoutForGL3Plus(bufferID, attributes);
        #endif
    }
}

void GLSharedContextVertexArray::Finalize()
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeVAO())
    {
        /* Finalize vertex array for OpenGL 2.x */
        FinalizeForGL2X();
    }
    #endif // /LLGL_GL_ENABLE_OPENGL2X
}

void GLSharedContextVertexArray::Bind(GLStateManager& stateMngr)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (!HasNativeVAO())
    {
        /* Bind vertex array for OpenGL 2.x */
        BindForGL2X(stateMngr);
    }
    else
    #endif
    {
        #if LLGL_GL3PLUS_SUPPORTED
        /* Bind vertex array for OpenGL 3+ using VAO */
        BindForGL3Plus(stateMngr);
        #endif
    }
}

void GLSharedContextVertexArray::SetDebugName(const char* name)
{
    #if LLGL_GL3PLUS_SUPPORTED

    /* Store debug name */
    debugName_ = (name != nullptr ? name : "");

    /* Invalidate debug name for all context dependent VAOs */
    for (GLContextVAO& contextVAO : contextDependentVAOs_)
        contextVAO.isObjectLabelDirty = true;

    /* If this vertex array already has its attributes set, get the current VAO to cause invaldiated labels to be updated */
    if (!attribs_.empty())
        (void)GetVAOForCurrentContext();

    #endif // /LLGL_GL3PLUS_SUPPORTED
}

#if LLGL_GL3PLUS_SUPPORTED

void GLSharedContextVertexArray::GLContextVAO::SetObjectLabel(const char* label)
{
    #ifdef LLGL_GL_ENABLE_OPENGL2X
    if (HasNativeVAO())
    #endif
    {
        /* Set label for VAO */
        GLSetObjectLabel(GL_VERTEX_ARRAY, vao.GetID(), label);
        isObjectLabelDirty = false;
    }
}

#endif // /LLGL_GL3PLUS_SUPPORTED


/*
 * ======= Private: =======
 */

#if LLGL_GL3PLUS_SUPPORTED

GLVertexArrayObject& GLSharedContextVertexArray::GetVAOForCurrentContext()
{
    /* Check if there's already an entry for the current context */
    const unsigned contextIndex = GLContext::GetCurrentGlobalIndex();
    LLGL_ASSERT(contextIndex > 0);

    const std::size_t vaoIndex = static_cast<std::size_t>(contextIndex) - 1;
    if (vaoIndex >= contextDependentVAOs_.size())
    {
        /* Resize container and fill new entry */
        contextDependentVAOs_.resize(vaoIndex + 1);
        contextDependentVAOs_[vaoIndex].vao.BuildVertexLayout(attribs_);
        if (!debugName_.empty())
            contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
    }
    else if (contextDependentVAOs_[vaoIndex].vao.GetID() == 0)
    {
        /* Fill empty entry in container */
        contextDependentVAOs_[vaoIndex].vao.BuildVertexLayout(attribs_);
        if (!debugName_.empty())
            contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
    }
    else if (contextDependentVAOs_[vaoIndex].isObjectLabelDirty)
    {
        /* Udpate debug label if it has been invalidated */
        if (!debugName_.empty())
            contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
    }

    /* Return VAO for current context */
    return contextDependentVAOs_[vaoIndex].vao;
}

void GLSharedContextVertexArray::BuildVertexLayoutForGL3Plus(GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    const std::size_t startOffset = attribs_.size();
    attribs_.resize(startOffset + attributes.size());

    for_range(i, attributes.size())
    {
        /* Convert to GLVertexAttribute */
        GLConvertVertexAttrib(attribs_[startOffset + i], attributes[i], bufferID);
    }
}

void GLSharedContextVertexArray::BindForGL3Plus(GLStateManager& stateMngr)
{
    stateMngr.BindVertexArray(GetVAOForCurrentContext().GetID());
}

#endif // /LLGL_GL3PLUS_SUPPORTED

#ifdef LLGL_GL_ENABLE_OPENGL2X

void GLSharedContextVertexArray::BuildVertexLayoutForGL2X(GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    const std::size_t startOffset = attribs_.size();
    attribs_.resize(startOffset + attributes.size());

    for_range(i, attributes.size())
    {
        const VertexAttribute& inAttrib = attributes[i];

        /* Check if instance divisor is used */
        if (inAttrib.instanceDivisor > 0)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("per-instance vertex attributes");

        /* Check if integral vertex attribute is used */
        bool isNormalizedFormat = IsNormalizedFormat(inAttrib.format);
        bool isFloatFormat      = IsFloatFormat(inAttrib.format);

        if (!isNormalizedFormat && !isFloatFormat)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("integral vertex attributes");

        /* Get data type and components of vector type */
        const FormatAttributes& formatAttribs = GetFormatAttribs(inAttrib.format);
        if ((formatAttribs.flags & FormatFlags::SupportsVertex) == 0)
            LLGL_TRAP_FEATURE_NOT_SUPPORTED("specified vertex attribute");

        /* Convert to GLVertexAttribute */
        GLConvertVertexAttrib(attribs_[startOffset + i], inAttrib, bufferID);
    }

    FinalizeForGL2X();
}

void GLSharedContextVertexArray::BindForGL2X(GLStateManager& stateMngr)
{
    /* Enable required vertex arrays */
    for (const GLVertexAttribute& attr : attribs_)
    {
        stateMngr.BindBuffer(GLBufferTarget::ArrayBuffer, attr.buffer);
        glVertexAttribPointer(attr.index, attr.size, attr.type, attr.normalized, attr.stride, reinterpret_cast<const GLvoid*>(attr.offsetPtrSized));
        glEnableVertexAttribArray(attr.index);
    }

    //TODO: add case for disabling attrib arrays inbetween, e.g. when only index 0 and 2 is used (rare case probably)
    /* Disable remaining vertex arrays */
    stateMngr.DisableVertexAttribArrays(attribIndexEnd_);
}

void GLSharedContextVertexArray::FinalizeForGL2X()
{
    if (attribs_.empty())
        return;

    /* Validate attribute indices are unique and fill the entire range [0, N) */
    std::vector<bool> locationsTaken;
    locationsTaken.resize(attribs_.size(), false);

    for (const GLVertexAttribute& attr : attribs_)
    {
        LLGL_ASSERT(
            attr.index < locationsTaken.size() && !locationsTaken[attr.index],
            "vertex attribute locations must fill the entire half-open range [0, N) for OpenGL 2.X"
        );
        locationsTaken[attr.index] = true;
    }

    /* Store upper bound for attribute indices */
    attribIndexEnd_ = attribs_.back().index + 1;

    /* Sort attributes by buffer binding and index in ascending order */
    std::sort(
        attribs_.begin(),
        attribs_.end(),
        [](const GLVertexAttribute& lhs, const GLVertexAttribute& rhs)
        {
            if (lhs.buffer < rhs.buffer)
                return true;
            if (lhs.buffer > rhs.buffer)
                return false;
            return (lhs.index < rhs.index);
        }
    );
}

#endif // /LLGL_GL_ENABLE_OPENGL2X


} // /namespace LLGL



// ================================================================================
