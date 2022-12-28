/*
 * DbgBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
