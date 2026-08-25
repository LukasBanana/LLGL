/*
 * GLBufferInputLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferInputLayout.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLBufferInputLayout::GLBufferInputLayout(GLBuffer* buffer)
{
    SetBuffers({ buffer });
}

GLBufferInputLayout::GLBufferInputLayout(ArrayView<GLBuffer*> buffers)
{
    SetBuffers(buffers);
}

void GLBufferInputLayout::Reset()
{
    buffers_.clear();
}

void GLBufferInputLayout::SetBuffers(ArrayView<GLBuffer*> buffers)
{
    buffers_.insert(buffers_.end(), buffers.begin(), buffers.end());
}

int GLBufferInputLayout::CompareSWO(const GLBufferInputLayout& lhs, const GLBufferInputLayout& rhs)
{
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO( lhs.buffers_.size(), rhs.buffers_.size() );
    for_range(i, lhs.buffers_.size())
    {
        LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.buffers_[i], rhs.buffers_[i]);
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
