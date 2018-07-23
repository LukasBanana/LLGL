/*
 * MTBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTBuffer.h"
#include <string.h>


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

void MTBuffer::Write(NSUInteger dstOffset, const void* data, NSUInteger dataSize)
{
    /* Set buffer region to update */
    NSRange range;
    range.location  = dstOffset;
    range.length    = dataSize;
    
    /* Copy data to CPU buffer region */
    auto byteAlignedBuffer = reinterpret_cast<std::int8_t*>([native_ contents]);
    ::memcpy(byteAlignedBuffer + dstOffset, data, dataSize);
    
    /* Notify Metal API about update */
    [native_ didModifyRange:range];
}


} // /namespace LLGL



// ================================================================================
