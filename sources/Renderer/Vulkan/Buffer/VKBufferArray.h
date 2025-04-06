/*
 * VKBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_BUFFER_ARRAY_H
#define LLGL_VK_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include "../Vulkan.h"
#include <LLGL/Container/Vector.h>
#include <cstdint>


namespace LLGL
{


class Buffer;

class VKBufferArray final : public BufferArray
{

    public:

        VKBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray);

        // Returns the array of buffer objects.
        inline const vector<VkBuffer>& GetBuffers() const
        {
            return buffers_;
        }

        // Returns the array of offsets.
        inline const vector<VkDeviceSize>& GetOffsets() const
        {
            return offsets_;
        }

    private:

        vector<VkBuffer>       buffers_;
        vector<VkDeviceSize>   offsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
