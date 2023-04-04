/*
 * VKDeviceImage.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_IMAGE_WRAPPER_H
#define LLGL_VK_IMAGE_WRAPPER_H


#include <LLGL/Texture.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


class VKDeviceMemoryRegion;
class VKDeviceMemoryManager;

// Wrapper class for VkImage handle.
class VKDeviceImage
{

    public:

        VKDeviceImage(VkDevice device);
        virtual ~VKDeviceImage() = default;

        // Explicit default move constructors required for GCC (to be used in VKSwapChain c'tor)
        VKDeviceImage(VKDeviceImage&&) = default;
        VKDeviceImage& operator = (VKDeviceImage&&) = default;

        void AllocateMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr);
        void ReleaseMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr);

        void BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void CreateVkImage(
            VkDevice                device,
            VkImageType             imageType,
            VkFormat                format,
            const VkExtent3D&       extent,
            std::uint32_t           numMipLevels,
            std::uint32_t           numArrayLayers,
            VkImageCreateFlags      createFlags,
            VkSampleCountFlagBits   sampleCountBits,
            VkImageUsageFlags       usageFlags
        );

        void ReleaseVkImage();

        void CreateVkImageView(
            VkDevice                        device,
            VkImageViewType                 viewType,
            VkFormat                        format,
            const VkImageSubresourceRange&  subresourceRange,
            VKPtr<VkImageView>&             outImageView,
            const VkComponentMapping*       components          = nullptr
        );

        // Returns the native VkImage handle.
        inline VkImage GetVkImage() const
        {
            return image_;
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return memoryRegion_;
        }

    private:

        VKPtr<VkImage>          image_;
        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
