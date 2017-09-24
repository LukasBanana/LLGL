/*
 * VKBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_BUFFER_H
#define LLGL_VK_BUFFER_H


#include <LLGL/Buffer.h>
#include "VKDeviceMemory.h"
#include "../Vulkan.h"
#include "../VKPtr.h"
#include <memory>


namespace LLGL
{


class VKBuffer : public Buffer
{

    public:

        VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        void BindToMemory(VkDevice device, const std::shared_ptr<VKDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset);

        void* Map(VkDevice device);
        void Unmap(VkDevice device);

        // Returns the hardware buffer object.
        inline VkBuffer Get() const
        {
            return buffer_.Get();
        }

        // Returns the memory requirements of the hardware buffer.
        inline const VkMemoryRequirements& GetRequirements() const
        {
            return requirements_;
        }

    private:

        VKPtr<VkBuffer>                 buffer_;
        VkDeviceSize                    size_           = 0;
        VkMemoryRequirements            requirements_;
        std::shared_ptr<VKDeviceMemory> deviceMemory_;

};


} // /namespace LLGL


#endif



// ================================================================================
