/*
 * VKDevice.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_DEVICE_H
#define LLGL_VK_DEVICE_H


#include <LLGL/TextureFlags.h>
#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"
#include "Buffer/VKDeviceBuffer.h"


namespace LLGL
{


class VKBuffer;
class VKTexture;

class VKDevice
{

    public:

        /* ----- Common ----- */

        VKDevice();
        VKDevice(VKDevice&& device);

        VKDevice& operator = (VKDevice&& device);

        void CreateLogicalDevice(
            VkPhysicalDevice                    physicalDevice,
            const VkPhysicalDeviceFeatures2*    features,
            const char* const*                  extensions,
            std::uint32_t                       numExtensions
        );

        void LoadLogicalDeviceWeakRef(VkPhysicalDevice physicalDevice, VkDevice device);

        // Blocks until the VkDevice becomes idle.
        void WaitIdle();

        /* ----- Allocation ----- */

        VKPtr<VkCommandPool> CreateCommandPool();

        /* ----- Queue ----- */

        VkCommandBuffer AllocCommandBuffer(bool begin = true);
        void FlushCommandBuffer(VkCommandBuffer cmdBuffer, bool release = true);

        /* ----- Buffer/Image operatons ----- */

        void CopyBuffer(
            VkBuffer        srcBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    size,
            VkDeviceSize    srcOffset = 0,
            VkDeviceSize    dstOffset = 0
        );

        void WriteBuffer(VKDeviceBuffer& buffer, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
        void ReadBuffer(VKDeviceBuffer& buffer, void* data, VkDeviceSize size, VkDeviceSize offset = 0);
        void FlushMappedBuffer(VKDeviceBuffer& buffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        /* ----- Handles ----- */

        // Returns the native VkDevice handle.
        inline const VKPtr<VkDevice>& GetVkDevice() const
        {
            return device_;
        }

        // Returns the native VkDevice handle.
        inline operator const VKPtr<VkDevice>& () const
        {
            return device_;
        }

        // Returns the native VkDevice handle.
        inline operator VkDevice () const
        {
            return device_;
        }

        // Returns the indices for the graphics and compute queues.
        inline const VKQueueFamilyIndices& GetQueueFamilyIndices() const
        {
            return queueFamilyIndices_;
        }

        // Returns the native VkQueue handle.
        inline VkQueue GetVkQueue() const
        {
            return graphicsQueue_;
        }

        // Returns the native VkCommandPool handle.
        inline const VKPtr<VkCommandPool>& GetVkCommandPool() const
        {
            return commandPool_;
        }

    private:

        VKPtr<VkDevice>         device_;
        VKQueueFamilyIndices    queueFamilyIndices_;
        VkQueue                 graphicsQueue_      = VK_NULL_HANDLE;
        VKPtr<VkCommandPool>    commandPool_;

};


} // /namespace LLGL


#endif



// ================================================================================
