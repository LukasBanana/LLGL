/*
 * GLVertexArrayHash.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexArrayHash.h"
#include "GLVertexAttribute.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


static std::size_t HashGLVertexAttribute(const GLVertexAttribute& attrib)
{
    return Hash(
        attrib.buffer, // for the hash, buffer is interpreted as binding slot index, not the GL buffer object
        attrib.index,
        attrib.size,
        attrib.type,
        attrib.normalized,
        attrib.stride,
        attrib.offsetPtrSized,
        attrib.divisor,
        attrib.isInteger
    );
}

GLVertexArrayHash::GLVertexArrayHash(const ArrayView<GLVertexAttribute>& attributes)
{
    Update(attributes);
}

void GLVertexArrayHash::Reset()
{
    hash_ = 0;
}

void GLVertexArrayHash::Update(const ArrayView<GLVertexAttribute>& attributes)
{
    hash_ = 0;
    for (const GLVertexAttribute& attrib : attributes)
        HashCombine(hash_, HashGLVertexAttribute(attrib));
}


} // /namespace LLGL



// ================================================================================
