/*
 * MTBufferArray.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_BUFFER_ARRAY_H
#define LLGL_MT_BUFFER_ARRAY_H


#import <Metal/Metal.h>

#include <LLGL/BufferArray.h>
#include <vector>


namespace LLGL
{


class Buffer;

class MTBufferArray final : public BufferArray
{

    public:

        using NativeType = id<MTLBuffer>;

        MTBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of buffer IDs.
        inline const std::vector<NativeType>& GetIDArray() const
        {
            return idArray_;
        }

        // Returns the array of buffer offsets.
        inline const std::vector<NSUInteger>& GetOffsets() const
        {
            return offsets_;
        }

    private:

        std::vector<NativeType> idArray_;
        std::vector<NSUInteger> offsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
