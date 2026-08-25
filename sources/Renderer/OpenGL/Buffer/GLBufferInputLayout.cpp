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


GLBufferInputLayout::GLBufferInputLayout(GLBuffer* buffer, GLintptr offset)
{
    SetBufferViews({ GLBufferView{ buffer, offset } });
}

GLBufferInputLayout::GLBufferInputLayout(ArrayView<GLBuffer*> buffers)
{
    bufferViews_.resize(buffers.size());
    for_range(i, buffers.size())
        bufferViews_[i] = GLBufferView{ buffers[i], 0 };
}

GLBufferInputLayout::GLBufferInputLayout(ArrayView<GLBufferView> buffers)
{
    SetBufferViews(buffers);
}

void GLBufferInputLayout::Reset()
{
    bufferViews_.clear();
}

void GLBufferInputLayout::SetBufferViews(ArrayView<GLBufferView> bufferViews)
{
    bufferViews_.insert(bufferViews_.end(), bufferViews.begin(), bufferViews.end());
}

int GLBufferInputLayout::CompareSWO(const GLBufferInputLayout& lhs, const GLBufferInputLayout& rhs)
{
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO( lhs.bufferViews_.size(), rhs.bufferViews_.size() );
    for_range(i, lhs.bufferViews_.size())
    {
        LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.bufferViews_[i].buffer, rhs.bufferViews_[i].buffer);
        LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.bufferViews_[i].offset, rhs.bufferViews_[i].offset);
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
