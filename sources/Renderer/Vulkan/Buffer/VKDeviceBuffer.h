/*
 * VKDeviceBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_BUFFER_H
#define LLGL_VK_DEVICE_BUFFER_H


#include "../Vulkan.h"
#include "../Memory/VKDeviceMemoryRegion.h"
#include "../VKPtr.h"

#include <stdexcept>

namespace LLGL
{


class VKDeviceMemoryManager;

class VKDeviceBuffer
{

    public:

        /* ----- Common ----- */

        VKDeviceBuffer(const VKPtr<VkDevice>& device);

        VKDeviceBuffer(const VKPtr<VkDevice>& device, const VkBufferCreateInfo& createInfo);

        VKDeviceBuffer(
            const VKPtr<VkDevice>&      device,
            const VkBufferCreateInfo&   createInfo,
            VKDeviceMemoryManager&      deviceMemoryMngr,
            VkMemoryPropertyFlags       memoryProperties
        );

        VKDeviceBuffer(const VKDeviceBuffer&) = delete;
        VKDeviceBuffer& operator = (const VKDeviceBuffer&) = delete;

        VKDeviceBuffer(VKDeviceBuffer&& rhs);
        VKDeviceBuffer& operator = (VKDeviceBuffer&& rhs);

        /* ----- Native buffer ----- */

        void CreateVkBuffer(
            const VKPtr<VkDevice>&      device,
            const VkBufferCreateInfo&   createInfo
        );

        void CreateVkBufferAndMemoryRegion(
            const VKPtr<VkDevice>&      device,
            const VkBufferCreateInfo&   createInfo,
            VKDeviceMemoryManager&      deviceMemoryMngr,
            VkMemoryPropertyFlags       memoryProperties
        );

        void ReleaseVkBuffer();

        //TODO: remove this and bind the buffer to device memory in "CreateVkBuffer".
        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void ReleaseMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr);

        void* Map(VkDevice device);
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
        VkMemoryRequirements    requirements_;
        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
