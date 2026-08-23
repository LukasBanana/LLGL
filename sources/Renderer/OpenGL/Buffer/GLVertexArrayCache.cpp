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
#if 0//TODO: this needs to call `GLVertexArrayObject::Release()`, but they are GL context dependent, so this needs a deferred deletion mechanism
    for (VertexBufferBinding& vertexBinding : vertexBindings_)
    {
        vertexBinding.vertexArray->Reset();
    }
#endif
    vertexBindings_.clear();
}

GLSharedContextVertexArray* GLVertexArrayCache::GetVertexArray(const GLVertexInputLayout& vertexInputLayout, const GLBufferInputLayout& bufferInputLayout)
{
    /* Always return a VAO, even when there are no input attributes since GL always needs a bound VAO for drawing */
    const ArrayView<GLVertexAttribute>  attribs = vertexInputLayout.GetAttribs();
    const ArrayView<GLBuffer*>          buffers = bufferInputLayout.GetBuffers();

    /* Get combination of vertex input and buffer input hashes */
    std::size_t combinedHash = 0;
    HashCombine(combinedHash, vertexInputLayout.GetHash());
    HashCombine(combinedHash, bufferInputLayout.GetHash());

    /* Try to find existing vertex buffer binding for input combination */
    std::size_t insertPosition = 0;
    VertexBufferBinding* vertexBinding = FindInSortedArray<VertexBufferBinding>(
        vertexBindings_.data(),
        vertexBindings_.size(),
        [combinedHash](const VertexBufferBinding& entry) -> int
        {
            LLGL_COMPARE_SEPARATE_MEMBERS_SWO(combinedHash, entry.combinedInputHash);
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
            bufferSlot < buffers.size(),
            "GLVertexAttribute::buffer=%u exceeded upper bound of %zu buffers",
            i, buffers.size()
        );
        vertexAttribs[i].buffer = buffers[bufferSlot]->GetID();
    }

    /* Build vertex layout and finalize immediately as it only references a single buffer */
    newVertexArray->BuildVertexLayout(vertexAttribs);
    newVertexArray->Finalize();

    /* Cache new vertex array in sorted list */
    auto itNewEntry = vertexBindings_.insert(
        vertexBindings_.begin() + insertPosition,
        VertexBufferBinding{ combinedHash, SmallVector<GLBuffer*, 1>{ buffers.begin(), buffers.end() }, std::move(newVertexArray) }
    );
    return itNewEntry->vertexArray.get();
}

//TODO: this could use some speedup as it is currently running with O(n) complexity
void GLVertexArrayCache::NotifyBufferRelease(const GLBuffer& buffer)
{
    /* Run through all vertex buffer bindings to see which one must be destroyed */
    RemoveAllFromListIf(
        vertexBindings_,
        [bufferPtr = &buffer](const VertexBufferBinding& entry) -> bool
        {
            return (std::find(entry.buffers.begin(), entry.buffers.end(), bufferPtr) != entry.buffers.end());
        }
    );
}


} // /namespace LLGL



// ================================================================================
