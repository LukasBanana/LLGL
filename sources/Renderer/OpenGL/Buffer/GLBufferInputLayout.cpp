/*
 * GLBufferInputLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferInputLayout.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


GLBufferInputLayout::GLBufferInputLayout(GLBuffer* buffer)
{
    Append({ buffer });
    Finalize();
}

GLBufferInputLayout::GLBufferInputLayout(ArrayView<GLBuffer*> buffers)
{
    Append(buffers);
    Finalize();
}

void GLBufferInputLayout::Reset()
{
    buffers_.clear();
    hash_ = 0;
}

void GLBufferInputLayout::Append(ArrayView<GLBuffer*> buffers)
{
    buffers_.insert(buffers_.end(), buffers.begin(), buffers.end());
}

void GLBufferInputLayout::Finalize()
{
    hash_ = 0;
    for (GLBuffer* buffer : buffers_)
        HashCombine(hash_, buffer);
}


} // /namespace LLGL



// ================================================================================
