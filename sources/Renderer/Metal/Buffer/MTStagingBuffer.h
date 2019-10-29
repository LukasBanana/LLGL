/*
 * MTStagingBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_STAGING_BUFFER_H
#define LLGL_MT_STAGING_BUFFER_H


#import <Metal/Metal.h>

#include <cstdint>


namespace LLGL
{


class MTStagingBuffer
{

    public:

        MTStagingBuffer(id<MTLDevice> device, NSUInteger size);
        ~MTStagingBuffer();

        MTStagingBuffer(MTStagingBuffer&& rhs);
        MTStagingBuffer& operator = (MTStagingBuffer&& rhs);

        MTStagingBuffer(const MTStagingBuffer&) = delete;
        MTStagingBuffer& operator = (const MTStagingBuffer&) = delete;

        // Resets the writing offset.
        void Reset();

        // Returns true if the remaining buffer size can fit the specified data size.
        bool Capacity(NSUInteger dataSize) const;

        // Writes the specified data to the native Metal buffer.
        void Write(const void* data, NSUInteger dataSize);

        // Returns the native MTLBuffer object.
        inline id<MTLBuffer> GetNative() const
        {
            return native_;
        }

        // Returns the size of the native Metal buffer.
        inline NSUInteger GetSize() const
        {
            return size_;
        }

        // Returns the current writing offset.
        inline NSUInteger GetOffset() const
        {
            return offset_;
        }

    private:

        id<MTLBuffer>   native_ = nil;
        NSUInteger      size_   = 0;
        NSUInteger      offset_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
