/*
 * MTBuffer.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTBuffer.h"
#include "../../ResourceUtils.h"
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

void MTBuffer::Write(NSUInteger offset, const void* data, NSUInteger dataSize)
{
    if (char* sharedGpuMemory = reinterpret_cast<char*>([native_ contents]))
    {
        ::memcpy(sharedGpuMemory + offset, data, dataSize);

        #ifndef LLGL_OS_IOS
        /* Notify Metal API about update */
        if (isManaged_)
            [native_ didModifyRange:NSMakeRange(offset, dataSize)];
        #endif
    }
}

void MTBuffer::Read(NSUInteger offset, void* data, NSUInteger dataSize)
{
    if (const char* sharedGpuMemory = reinterpret_cast<char*>([native_ contents]))
        ::memcpy(data, sharedGpuMemory + offset, dataSize);
}

void* MTBuffer::Map(CPUAccess access)
{
    if (HasWriteAccess(access))
        mappedWriteRange_ = NSMakeRange(0, [native_ length]);
    return [native_ contents];
}

void* MTBuffer::Map(CPUAccess access, NSUInteger offset, NSUInteger length)
{
    if (HasWriteAccess(access))
        mappedWriteRange_ = NSMakeRange(offset, length);
    char* mappedMemory = reinterpret_cast<char*>([native_ contents]) + offset;
    return reinterpret_cast<void*>(mappedMemory);
}

void MTBuffer::Unmap()
{
    #ifndef LLGL_OS_IOS
    if (isManaged_ && mappedWriteRange_.length > 0)
        [native_ didModifyRange:mappedWriteRange_];
    #endif // /LLGL_OS_IOS
    mappedWriteRange_ = NSMakeRange(0, 0);
}


} // /namespace LLGL



// ================================================================================
