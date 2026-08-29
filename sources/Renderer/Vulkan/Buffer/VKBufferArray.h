/*
 * VKBufferArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_BUFFER_ARRAY_H
#define LLGL_VK_BUFFER_ARRAY_H


#include <LLGL/BufferArray.h>
#include <LLGL/Container/ArrayView.h>
#include "../Vulkan.h"
#include <vector>


namespace LLGL
{


class Buffer;

class VKBufferArray final : public BufferArray
{

    public:

        VKBufferArray(ArrayView<VertexBufferView> bufferViews);

        // Returns the array of buffer objects.
        inline const std::vector<VkBuffer>& GetBuffers() const
        {
            return buffers_;
        }

        // Returns the array of offsets.
        inline const std::vector<VkDeviceSize>& GetOffsets() const
        {
            return offsets_;
        }

    private:

        std::vector<VkBuffer>       buffers_;
        std::vector<VkDeviceSize>   offsets_;

};


} // /namespace LLGL


#endif



// ================================================================================
