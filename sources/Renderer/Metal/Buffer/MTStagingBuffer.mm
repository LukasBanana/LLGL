/*
 * MTStagingBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTStagingBuffer.h"
#include <string.h>


namespace LLGL
{


MTStagingBuffer::MTStagingBuffer(id<MTLDevice> device, NSUInteger size) :
    size_ { size }
{
    native_ = [device newBufferWithLength:size options:MTLResourceStorageModeShared];
}

MTStagingBuffer::~MTStagingBuffer()
{
    if (native_ != nil)
        [native_ release];
}

MTStagingBuffer::MTStagingBuffer(MTStagingBuffer&& rhs) :
    native_ { rhs.native_ },
    size_   { rhs.size_   },
    offset_ { rhs.offset_ }
{
    rhs.native_ = nil;
}

MTStagingBuffer& MTStagingBuffer::operator = (MTStagingBuffer&& rhs)
{
    if (this != &rhs)
    {
        /* Release previous native buffer */
        if (native_ != nil)
            [native_ release];

        /* Copy data */
        native_ = rhs.native_;
        size_   = rhs.size_;
        offset_ = rhs.offset_;

        /* Drop reference from argument buffer */
        rhs.native_ = nil;
    }
    return *this;
}

void MTStagingBuffer::Reset()
{
    offset_ = 0;
}

bool MTStagingBuffer::Capacity(NSUInteger dataSize) const
{
    return (offset_ + dataSize <= size_);
}

void MTStagingBuffer::Write(const void* data, NSUInteger dataSize)
{
    /* Copy data to CPU buffer region and increase offset for next data */
    auto byteAlignedBuffer = reinterpret_cast<std::int8_t*>([native_ contents]);
    ::memcpy(byteAlignedBuffer + offset_, data, dataSize);
    offset_ += dataSize;
}


} // /namespace LLGL



// ================================================================================
