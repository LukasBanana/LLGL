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
    desc           { CopyBufferDescWithNewVertexAttribs(desc, vertexAttribs_) }
{
}

void DbgBuffer::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

BufferDescriptor DbgBuffer::GetDesc() const
{
    return instance.GetDesc();
}


} // /namespace LLGL



// ================================================================================
