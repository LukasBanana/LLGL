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
    std::size_t seed = 0;
    HashCombine(seed, attrib.buffer); // for the hash, buffer is interpreted as binding slot index, not the GL buffer object
    HashCombine(seed, attrib.index);
    HashCombine(seed, attrib.size);
    HashCombine(seed, attrib.type);
    HashCombine(seed, attrib.normalized);
    HashCombine(seed, attrib.stride);
    HashCombine(seed, attrib.offsetPtrSized);
    HashCombine(seed, attrib.divisor);
    HashCombine(seed, attrib.isInteger);
    return seed;
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
