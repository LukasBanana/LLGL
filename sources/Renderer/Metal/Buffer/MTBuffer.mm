/*
 * MTBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTBuffer.h"
#include <string.h>


namespace LLGL
{


static MTLResourceOptions GetMTLResourceOptions(const BufferDescriptor& desc)
{
    #ifdef LLGL_OS_IOS
    return MTLResourceStorageModeShared;
    #else
    if ((desc.miscFlags & MiscFlags::DynamicUsage) != 0)
        return MTLResourceStorageModeShared;
    //else if ((desc.bindFlags & BindFlags::Storage) != 0)
    //    return MTLResourceStorageModePrivate;
    else
        return MTLResourceStorageModeManaged;
    #endif
}

MTBuffer::MTBuffer(id<MTLDevice> device, const BufferDescriptor& desc, const void* initialData) :
    Buffer           { desc.bindFlags                   },
    indexType16Bits_ { (desc.format == Format::R16UInt) }
{
    auto opt = GetMTLResourceOptions(desc);

    #ifndef LLGL_OS_IOS
    isManaged_ = ((opt & MTLResourceStorageModeManaged) != 0);
    #endif

    if (initialData)
        native_ = [device newBufferWithBytes:initialData length:(NSUInteger)desc.size options:opt];
    else
        native_ = [device newBufferWithLength:(NSUInteger)desc.size options:opt];
}

MTBuffer::~MTBuffer()
{
    [native_ release];
}

BufferDescriptor MTBuffer::GetDesc() const
{
    BufferDescriptor bufferDesc;

    bufferDesc.size             = [native_ length];
    bufferDesc.bindFlags        = GetBindFlags();
    #if 0//TODO
    bufferDesc.cpuAccessFlags   = 0;
    bufferDesc.miscFlags        = 0;
    #endif

    return bufferDesc;
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

    #ifndef LLGL_OS_IOS
    /* Notify Metal API about update */
    if (isManaged_)
        [native_ didModifyRange:range];
    #endif
}

void* MTBuffer::Map(CPUAccess access)
{
    mappedWriteAccess_ = (access == CPUAccess::WriteOnly);
    return [native_ contents];
}

void MTBuffer::Unmap()
{
    #ifndef LLGL_OS_IOS
    if (isManaged_ && mappedWriteAccess_)
    {
        NSRange range;
        range.location  = 0;
        range.length    = [native_ length];
        [native_ didModifyRange:range];
    }
    #endif // /LLGL_OS_IOS
    mappedWriteAccess_ = false;
}


} // /namespace LLGL



// ================================================================================
