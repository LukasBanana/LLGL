/*
 * VKDeviceImage.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        VKDeviceImage(const VKPtr<VkDevice>& device);
        virtual ~VKDeviceImage() = default;

        // Explicit default move constructors required for GCC (to be used in VKRenderContext c'tor)
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
            VkDevice            device,
            VkImageViewType     viewType,
            VkFormat            format,
            VkImageAspectFlags  aspectFlags,
            std::uint32_t       baseMipLevel,
            std::uint32_t       numMipLevels,
            std::uint32_t       baseArrayLayer,
            std::uint32_t       numArrayLayers,
            VkImageView*        imageViewRef
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
