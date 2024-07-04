/*
 * GLBufferWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(long bindFlags, const char* debugName) :
    GLBuffer { bindFlags, debugName }
{
    if (debugName != nullptr)
    {
        const std::string vaoLabel = debugName + std::string(".VAO");
        vertexArray_.SetDebugName(vaoLabel.c_str());
    }
}

void GLBufferWithVAO::BuildVertexArray(const ArrayView<VertexAttribute>& vertexAttribs)
{
    /* Store vertex format (required if this buffer is used in a buffer array) */
    if (vertexAttribs.empty())
        vertexAttribs_.clear();
    else
        vertexAttribs_ = std::vector<VertexAttribute>(vertexAttribs.begin(), vertexAttribs.end());

    /* Build vertex layout and finalize immediately as it only references a single buffer */
    vertexArray_.BuildVertexLayout(GetID(), vertexAttribs_);
    vertexArray_.Finalize();
}


} // /namespace LLGL



// ================================================================================
