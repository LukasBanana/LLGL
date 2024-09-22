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
class VKCommandContext;
struct TextureSubresource;

// Wrapper class for VkImage handle.
class VKDeviceImage
{

    public:

        VKDeviceImage() = default;
        virtual ~VKDeviceImage() = default;

        VKDeviceImage(VkDevice device);

        VKDeviceImage(VKDeviceImage&& rhs);
        VKDeviceImage& operator = (VKDeviceImage&& rhs);

        VKDeviceImage(const VKDeviceImage&) = delete;
        VKDeviceImage& operator = (const VKDeviceImage&) = delete;

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

        VkImageLayout TransitionImageLayout(
            VKCommandContext&           context,
            VkFormat                    format,
            VkImageLayout               newLayout,
            const TextureSubresource&   subresource
        );

        // Returns the native VkImage handle.
        inline VkImage GetVkImage() const
        {
            return image_;
        }

        // Returns the native VkImageLayout state of this image.
        inline VkImageLayout GetVkImageLayout() const
        {
            return layout_;
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return memoryRegion_;
        }

        // Returns the Vulkan memory requirements for this device image.
        inline const VkMemoryRequirements& GetMemoryRequirements() const
        {
            return memoryRequirements_;
        }

    private:

        VKPtr<VkImage>          image_;
        VkImageLayout           layout_             = VK_IMAGE_LAYOUT_UNDEFINED;
        VkMemoryRequirements    memoryRequirements_ = {};
        VKDeviceMemoryRegion*   memoryRegion_       = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
