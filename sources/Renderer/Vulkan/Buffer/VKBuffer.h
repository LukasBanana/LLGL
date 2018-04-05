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


struct VKBufferObject
{
    VKBufferObject(const VKPtr<VkDevice>& device);
    VKBufferObject(VKBufferObject&& rhs);
    VKBufferObject& operator = (VKBufferObject&& rhs);

    void Create(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);
    void Release();

    VKPtr<VkBuffer>         buffer;
    VkMemoryRequirements    requirements;
};


class VKBuffer : public Buffer
{

    public:

        VKBuffer(const BufferType type, const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        void BindToMemory(VkDevice device, const std::shared_ptr<VKDeviceMemory>& deviceMemory, VkDeviceSize memoryOffset);
        void TakeStagingBuffer(VKBufferObject&& buffer, std::shared_ptr<VKDeviceMemory>&& deviceMemory);

        // Returns the hardware buffer object.
        inline VkBuffer GetVkBuffer() const
        {
            return bufferObj_.buffer.Get();
        }

        // Returns the memory requirements of the hardware buffer.
        inline const VkMemoryRequirements& GetRequirements() const
        {
            return bufferObj_.requirements;
        }

        // Returns the size originally specified in the descriptor.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

    private:

        VKBufferObject                  bufferObj_;
        std::shared_ptr<VKDeviceMemory> deviceMemory_;

        VKBufferObject                  bufferObjStaging_;
        std::shared_ptr<VKDeviceMemory> deviceMemoryStaging_;

        VkDeviceSize                    size_                   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
