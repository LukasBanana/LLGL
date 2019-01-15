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


static MTLResourceOptions GetMTLResourceOptions(const BufferDescriptor& desc)
{
    if ((desc.miscFlags & MiscFlags::DynamicUsage) != 0)
        return MTLResourceStorageModeShared;
    //else if ((desc.bindFlags & BindFlags::RWStorageBuffer) != 0)
    //    return MTLResourceStorageModePrivate;
    else
        return MTLResourceStorageModeManaged;
}

MTBuffer::MTBuffer(id<MTLDevice> device, const BufferDescriptor& desc, const void* initialData) :
    Buffer           { desc.bindFlags                               },
    indexType16Bits_ { (desc.indexBuffer.format == Format::R16UInt) }
{
    auto opt = GetMTLResourceOptions(desc);
    
    isManaged_ = ((opt & MTLResourceStorageModeManaged) != 0);
    
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
    if (isManaged_)
        [native_ didModifyRange:range];
}

void* MTBuffer::Map(CPUAccess access)
{
    mappedWriteAccess_ = (access == CPUAccess::WriteOnly);
    return [native_ contents];
}

void MTBuffer::Unmap()
{
    if (isManaged_ && mappedWriteAccess_)
    {
        NSRange range;
        range.location  = 0;
        range.length    = [native_ length];
        [native_ didModifyRange:range];
    }
    mappedWriteAccess_ = false;
}


} // /namespace LLGL



// ================================================================================
