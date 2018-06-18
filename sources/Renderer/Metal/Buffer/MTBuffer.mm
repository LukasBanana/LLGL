/*
 * MTBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTBuffer.h"


namespace LLGL
{


static MTLResourceOptions GetMTLResourceOptions(long flags)
{
    if ((flags & BufferFlags::DynamicUsage) != 0)
        return MTLResourceStorageModeShared;
    else
        return MTLResourceStorageModeManaged;
}

MTBuffer::MTBuffer(id<MTLDevice> device, const BufferDescriptor& desc, const void* initialData) :
    Buffer { desc.type }
{
    auto opt = GetMTLResourceOptions(desc.flags);
    if (initialData)
        native_ = [device newBufferWithBytes:initialData length:(NSUInteger)desc.size options:opt];
    else
        native_ = [device newBufferWithLength:(NSUInteger)desc.size options:opt];
}

MTBuffer::~MTBuffer()
{
    [native_ release];
}


} // /namespace LLGL



// ================================================================================
