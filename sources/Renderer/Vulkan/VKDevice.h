/*
 * VKDevice.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_H
#define LLGL_VK_DEVICE_H


#include "Vulkan.h"
#include "VKPtr.h"
#include "VKCore.h"


namespace LLGL
{


class VKDevice
{

    public:

        /* ----- Common ----- */

        VKDevice();

        void CreateLogicalDevice(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures* features);

        void WaitIdle();

        /* ----- Allocation ----- */

        VKPtr<VkCommandPool> CreateCommandPool();

        /* ----- Queue ----- */

        VkCommandBuffer AllocCommandBuffer(bool begin = true);
        void FlushCommandBuffer(VkCommandBuffer cmdBuffer, bool release = true);

        /* ----- Buffer/Image operatons ----- */

        void TransitionImageLayout(
            VkImage         image,
            VkFormat        format,
            VkImageLayout   oldLayout,
            VkImageLayout   newLayout,
            std::uint32_t   numMipLevels,
            std::uint32_t   numArrayLayers
        );

        void CopyBuffer(
            VkBuffer        srcBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    size,
            VkDeviceSize    srcOffset = 0,
            VkDeviceSize    dstOffset = 0
        );

        void CopyBufferToImage(
            VkBuffer            srcBuffer,
            VkImage             dstImage,
            const VkExtent3D&   extent,
            std::uint32_t       numLayers
        );

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
        inline const QueueFamilyIndices& GetQueueFamilyIndices() const
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
        QueueFamilyIndices      queueFamilyIndices_;
        VkQueue                 graphicsQueue_      = VK_NULL_HANDLE;
        VKPtr<VkCommandPool>    commandPool_;

};


} // /namespace LLGL


#endif



// ================================================================================
