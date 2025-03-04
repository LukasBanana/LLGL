/*
 * VKDeviceBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DEVICE_BUFFER_H
#define LLGL_VK_DEVICE_BUFFER_H


#include "../Vulkan.h"
#include "../Memory/VKDeviceMemoryRegion.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKDeviceMemoryManager;

class VKDeviceBuffer
{

    public:

        /* ----- Common ----- */

        VKDeviceBuffer(VkDevice device);

        VKDeviceBuffer(VkDevice device, const VkBufferCreateInfo& createInfo);

        VKDeviceBuffer(
            VkDevice                    device,
            const VkBufferCreateInfo&   createInfo,
            VKDeviceMemoryManager&      deviceMemoryMngr,
            VkMemoryPropertyFlags       memoryProperties
        );

        VKDeviceBuffer(const VKDeviceBuffer&) = delete;
        VKDeviceBuffer& operator = (const VKDeviceBuffer&) = delete;

        VKDeviceBuffer(VKDeviceBuffer&& rhs) noexcept;
        VKDeviceBuffer& operator = (VKDeviceBuffer&& rhs) noexcept;

        /* ----- Native buffer ----- */

        void CreateVkBuffer(VkDevice device, const VkBufferCreateInfo& createInfo);

        void CreateVkBufferAndMemoryRegion(
            VkDevice                    device,
            const VkBufferCreateInfo&   createInfo,
            VKDeviceMemoryManager&      deviceMemoryMngr,
            VkMemoryPropertyFlags       memoryProperties
        );

        void ReleaseVkBuffer();

        //TODO: remove this and bind the buffer to device memory in "CreateVkBuffer".
        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void ReleaseMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr);

        void* Map(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void Unmap(VkDevice device);

        /* ----- Getter ----- */

        // Returns the native VkBuffer handle.
        inline VkBuffer GetVkBuffer() const
        {
            return buffer_.Get();
        }

        // Returns the memory requirements of the native VkBuffer.
        inline const VkMemoryRequirements& GetRequirements() const
        {
            return requirements_;
        }

        // Returns the region of the device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return memoryRegion_;
        }

    private:

        VKPtr<VkBuffer>         buffer_;
        VkMemoryRequirements    requirements_   = {};
        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
