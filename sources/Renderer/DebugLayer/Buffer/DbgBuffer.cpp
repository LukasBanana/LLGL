/*
 * DbgBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgBuffer.h"
#include "../DbgCore.h"


namespace LLGL
{


static BufferDescriptor CopyBufferDescWithNewVertexAttribs(const BufferDescriptor& inDesc, const ArrayView<VertexAttribute>& inVertexAttribs)
{
    auto outDesc = inDesc;
    outDesc.vertexAttribs = inVertexAttribs;
    return outDesc;
}

DbgBuffer::DbgBuffer(Buffer& instance, const BufferDescriptor& desc) :
    Buffer         { desc.bindFlags                                           },
    vertexAttribs_ { desc.vertexAttribs.begin(), desc.vertexAttribs.end()     },
    instance       { instance                                                 },
    desc           { CopyBufferDescWithNewVertexAttribs(desc, vertexAttribs_) },
    label          { LLGL_DBG_LABEL(desc)                                     }
{
}

void DbgBuffer::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

BufferDescriptor DbgBuffer::GetDesc() const
{
    return instance.GetDesc();
}

void DbgBuffer::OnMap(const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    mappedAccess_   = access;
    mappedRange_[0] = std::min(offset, desc.size);
    mappedRange_[1] = std::min(offset + length, desc.size);
}

void DbgBuffer::OnUnmap()
{
    if (IsMappedForCPUAccess())
    {
        if (mappedAccess_ == CPUAccess::WriteDiscard || mappedAccess_ == CPUAccess::WriteOnly)
        {
            /* If the buffer was mapped for writing, we assume the buffer has been initialized */
            initialized = true;
        }
        mappedRange_[0] = 0;
        mappedRange_[1] = 0;
    }
}

bool DbgBuffer::IsMappedForCPUAccess() const
{
    return (mappedRange_[0] < mappedRange_[1]);
}


} // /namespace LLGL



// ================================================================================
