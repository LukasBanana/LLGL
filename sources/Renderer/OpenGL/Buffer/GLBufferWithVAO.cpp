/*
 * GLBufferWithVAO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferWithVAO.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensionRegistry.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLBufferWithVAO::GLBufferWithVAO(const BufferDescriptor& bufferDesc) :
    GLBuffer { bufferDesc }
{
    if (bufferDesc.debugName != nullptr)
    {
        const std::string vaoLabel = bufferDesc.debugName + std::string(".VAO");
        vertexArray_.SetDebugName(vaoLabel.c_str());
    }
}

void GLBufferWithVAO::BuildVertexArray(const ArrayView<GLVertexAttribute>& vertexAttribs)
{
    /* Store vertex format (required if this buffer is used in a buffer array) */
    if (vertexAttribs.empty())
        vertexAttribs_.clear();
    else
        vertexAttribs_ = std::vector<GLVertexAttribute>(vertexAttribs.begin(), vertexAttribs.end());

    /* Override buffer ID in all attributes */
    for (GLVertexAttribute& attrib : vertexAttribs_)
        attrib.buffer = GetID();

    /* Build vertex layout and finalize immediately as it only references a single buffer */
    vertexArray_.Reset();
    vertexArray_.BuildVertexLayout(vertexAttribs_);
    vertexArray_.Finalize();
}

void GLBufferWithVAO::BuildVertexArray(const ArrayView<VertexAttribute>& vertexAttribs)
{
    /* Store vertex format (required if this buffer is used in a buffer array) */
    vertexAttribs_.resize(vertexAttribs.size());

    /* Convert vertex attributes to GL format */
    for_range(i, vertexAttribs.size())
        GLConvertVertexAttrib(vertexAttribs_[i], vertexAttribs[i], GetID());

    /* Build vertex layout and finalize immediately as it only references a single buffer */
    vertexArray_.Reset();
    vertexArray_.BuildVertexLayout(vertexAttribs_);
    vertexArray_.Finalize();
}


} // /namespace LLGL



// ================================================================================
