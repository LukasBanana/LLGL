/*
 * GLBufferArray.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLBufferArray.h"
#include "GLBuffer.h"
#include "../../CheckedCast.h"
#include "../../BufferUtils.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static std::vector<GLBufferView> GetAsGLBufferViews(ArrayView<VertexBufferView> bufferViews)
{
    std::vector<GLBufferView> outBufferViews;
    outBufferViews.resize(bufferViews.size());
    for_range(i, bufferViews.size())
    {
        outBufferViews[i].buffer = LLGL_CAST(GLBuffer*, bufferViews[i].buffer);
        outBufferViews[i].offset = static_cast<GLintptr>(bufferViews[i].offset);
    }
    return outBufferViews;
}

GLBufferArray::GLBufferArray(ArrayView<VertexBufferView> bufferViews) :
    BufferArray        { GetCombinedBindFlags(bufferViews) },
    bufferInputLayout_ { GetAsGLBufferViews(bufferViews)   }
{
}


} // /namespace LLGL



// ================================================================================
