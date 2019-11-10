/*
 * MTBufferArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
    
        MTBufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray);

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
