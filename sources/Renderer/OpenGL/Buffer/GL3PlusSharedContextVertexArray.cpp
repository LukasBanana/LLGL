/*
 * GL3PlusSharedContextVertexArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GL3PlusSharedContextVertexArray.h"
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


void GL3PlusSharedContextVertexArray::Reset()
{
    inputLayout_.Reset();
}

void GL3PlusSharedContextVertexArray::BuildVertexLayout(const ArrayView<GLVertexAttribute>& attributes)
{
    inputLayout_.Append(attributes);
}

void GL3PlusSharedContextVertexArray::BuildVertexLayout(GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    inputLayout_.Append(bufferID, attributes);
}

void GL3PlusSharedContextVertexArray::Finalize()
{
    inputLayout_.Finalize();
}

void GL3PlusSharedContextVertexArray::Bind(GLStateManager& stateMngr)
{
    stateMngr.BindVertexArray(GetVAOForCurrentContext().GetID());
}

void GL3PlusSharedContextVertexArray::SetDebugName(const char* name)
{
    /* Store debug name */
    debugName_ = StringLiteral{ (name != nullptr ? name : ""), CopyTag{} };

    /* Invalidate debug name for all context dependent VAOs */
    for (GLContextVAO& contextVAO : contextDependentVAOs_)
        contextVAO.isObjectLabelDirty = true;

    /* If this vertex array already has its attributes set, get the current VAO to cause invaldiated labels to be updated */
    if (!inputLayout_.GetAttribs().empty())
        (void)GetVAOForCurrentContext();
}


/*
 * ======= Private: =======
 */

GLVertexArrayObject& GL3PlusSharedContextVertexArray::GetVAOForCurrentContext()
{
    /* Check if there's already an entry for the current context */
    const unsigned contextIndex = GLContext::GetCurrentGlobalIndex();
    LLGL_ASSERT(contextIndex > 0);

    const std::size_t vaoIndex = static_cast<std::size_t>(contextIndex) - 1;
    if (vaoIndex >= contextDependentVAOs_.size())
    {
        /* Resize container and fill new entry */
        contextDependentVAOs_.resize(vaoIndex + 1);
        contextDependentVAOs_[vaoIndex].vao.BuildVertexLayout(inputLayout_);
        if (!debugName_.empty())
            contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
    }
    else if (contextDependentVAOs_[vaoIndex].vao.GetID() == 0)
    {
        /* Fill empty entry in container */
        contextDependentVAOs_[vaoIndex].vao.BuildVertexLayout(inputLayout_);
        if (!debugName_.empty())
            contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
    }
    else
    {
        if (inputLayout_.GetHash() != contextDependentVAOs_[vaoIndex].vao.GetInputLayoutHash())
        {
            /* Update vertex attributes if the input layout has changed (i.e. hashes don't match anymore) */
            contextDependentVAOs_[vaoIndex].vao.BuildVertexLayout(inputLayout_);
        }
        if (contextDependentVAOs_[vaoIndex].isObjectLabelDirty)
        {
            /* Udpate debug label if it has been invalidated */
            if (!debugName_.empty())
                contextDependentVAOs_[vaoIndex].SetObjectLabel(debugName_.c_str());
        }
    }

    /* Return VAO for current context */
    return contextDependentVAOs_[vaoIndex].vao;
}


/*
 * GLContextVAO structure
 */

void GL3PlusSharedContextVertexArray::GLContextVAO::SetObjectLabel(const char* label)
{
    /* Set label for VAO */
    GLSetObjectLabel(GL_VERTEX_ARRAY, vao.GetID(), label);
    isObjectLabelDirty = false;
}


} // /namespace LLGL



// ================================================================================
