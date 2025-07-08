/*
 * GLVertexInputLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexInputLayout.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


void GLVertexInputLayout::Reset()
{
    attribs_.clear();
    attribsHash_.Reset();
}

void GLVertexInputLayout::Append(const ArrayView<GLVertexAttribute>& attributes)
{
    attribs_.insert(attribs_.end(), attributes.begin(), attributes.end());
}

void GLVertexInputLayout::Append(const GLuint bufferID, const ArrayView<VertexAttribute>& attributes)
{
    /* Convert to GLVertexAttribute */
    const std::size_t startOffset = attribs_.size();
    attribs_.resize(startOffset + attributes.size());

    for_range(i, attributes.size())
        GLConvertVertexAttrib(attribs_[startOffset + i], attributes[i], bufferID);
}

void GLVertexInputLayout::Finalize()
{
    /* Update vertex attributes hash */
    attribsHash_.Update(attribs_);
}


} // /namespace LLGL



// ================================================================================
