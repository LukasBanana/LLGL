/*
 * GLVertexArrayCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLVertexArrayCache.h"
#include "GLBuffer.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


GLVertexArrayCache& GLVertexArrayCache::Get()
{
    static GLVertexArrayCache instance;
    return instance;
}

void GLVertexArrayCache::Clear()
{
    std::lock_guard<std::mutex> guard{ mutex_ };
    vertexBindings_.clear();
}

GLSharedContextVertexArray* GLVertexArrayCache::FindOrMakeVertexArray(const GLVertexInputLayout& vertexInputLayout, const GLBufferInputLayout& bufferInputLayout)
{
    std::lock_guard<std::mutex> guard{ mutex_ };

    /* Always return a VAO, even when there are no input attributes since GL always needs a bound VAO for drawing */
    const ArrayView<GLVertexAttribute>  attribs     = vertexInputLayout.GetAttribs();
    const ArrayView<GLBufferView>       bufferViews = bufferInputLayout.GetBufferViews();

    /* Try to find existing vertex buffer binding for input combination */
    std::size_t insertPosition = 0;
    VertexBufferBinding* vertexBinding = FindInSortedArray<VertexBufferBinding>(
        vertexBindings_.data(),
        vertexBindings_.size(),
        [&vertexInputLayout, &bufferInputLayout](const VertexBufferBinding& entry) -> int
        {
            int cmpVertexInputLayout = GLVertexInputLayout::CompareSWO(entry.vertexInputLayout, vertexInputLayout);
            if (cmpVertexInputLayout != 0)
                return cmpVertexInputLayout;

            int cmpBufferInputLayout = GLBufferInputLayout::CompareSWO(entry.bufferInputLayout, bufferInputLayout);
            if (cmpBufferInputLayout != 0)
                return cmpBufferInputLayout;

            return 0;
        },
        &insertPosition
    );

    if (vertexBinding != nullptr)
        return vertexBinding->vertexArray.get();

    /* No combination found -> create a new vertex array */
    GLSharedContextVertexArrayPtr newVertexArray = MakeUnique<GLSharedContextVertexArray>();

    /* Resolve buffer ID for all vertex attributes */
    std::vector<GLVertexAttribute> vertexAttribs(attribs.begin(), attribs.end());
    for_range(i, vertexAttribs.size())
    {
        const std::uint32_t bufferSlot = vertexAttribs[i].buffer;
        LLGL_ASSERT(
            bufferSlot < bufferViews.size(),
            "GLVertexAttribute::buffer=%u exceeded upper bound of %zu buffers",
            i, bufferViews.size()
        );
        vertexAttribs[i].buffer         = bufferViews[bufferSlot].buffer->GetID();
        vertexAttribs[i].offsetPtrSized += bufferViews[bufferSlot].offset;
    }

    /* Build vertex layout and finalize immediately as it only references a single buffer */
    newVertexArray->BuildVertexLayout(vertexAttribs);
    newVertexArray->Finalize();

    /* Cache new vertex array in sorted list */
    auto itNewEntry = vertexBindings_.insert(
        vertexBindings_.begin() + insertPosition,
        VertexBufferBinding{ vertexInputLayout, bufferInputLayout, std::move(newVertexArray) }
    );
    return itNewEntry->vertexArray.get();
}

//TODO: this could use some speedup as it is currently running with O(n) complexity
void GLVertexArrayCache::NotifyBufferRelease(const GLBuffer& buffer)
{
    /* Run through all vertex buffer bindings to see which one must be destroyed */
    std::lock_guard<std::mutex> guard{ mutex_ };
    const GLBuffer* bufferPtr = &buffer;
    RemoveAllFromListIf(
        vertexBindings_,
        [bufferPtr](const VertexBufferBinding& entry) -> bool
        {
            ArrayView<GLBufferView> bufferViews = entry.bufferInputLayout.GetBufferViews();
            auto it = std::find_if(
                bufferViews.begin(),
                bufferViews.end(),
                [bufferPtr](const GLBufferView& bufferViewEntry) -> bool
                {
                    return (bufferViewEntry.buffer == bufferPtr);
                }
            );
            return (it != bufferViews.end());
        }
    );
}


} // /namespace LLGL



// ================================================================================
