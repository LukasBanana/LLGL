/*
 * VKTexture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_TEXTURE_H
#define LLGL_VK_TEXTURE_H


#include <LLGL/Texture.h>
#include "VKDeviceImage.h"
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


class VKDeviceMemoryRegion;
class VKDeviceMemoryManager;

class VKTexture final : public Texture
{

    public:

        VKTexture(
            VkDevice                    device,
            VKDeviceMemoryManager&      deviceMemoryMngr,
            const TextureDescriptor&    desc
        );

        Extent3D GetMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor GetDesc() const override;

        Format GetFormat() const override;

        SubresourceFootprint GetSubresourceFootprint(std::uint32_t mipLevel) const override;

    public:

        // Creates an additional texture view of the specified texture range and uses the same format as this texture object.
        void CreateImageView(
            VkDevice                    device,
            const TextureSubresource&   subresource,
            Format                      format,
            VKPtr<VkImageView>&         outImageView
        );

        // Creates an additional texture view with the specififed view descriptor.
        void CreateImageView(
            VkDevice                        device,
            const TextureViewDescriptor&    textureViewDesc,
            VKPtr<VkImageView>&             outImageView
        );

        // Creates the standard image view that is stored within this texture object.
        void CreateInternalImageView(VkDevice device);

        // Returns the image ascpect flags for the VkFormat of this texture.
        VkImageAspectFlags GetAspectFlags() const;

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return image_.GetVkImage();
        }

        // Returns the internal Vulkan image view object (created with 'CreateInternalImageView').
        inline VkImageView GetVkImageView() const
        {
            return imageView_;
        }

        // Returns the VkFormat with whereby the VkImage object was created.
        inline VkFormat GetVkFormat() const
        {
            return format_;
        }

        // Returns the VkExtent3D with whereby the VkImage object was created (does not include the array layer count).
        inline const VkExtent3D& GetVkExtent() const
        {
            return extent_;
        }

        // Returns the number of MIP level with whereby the VkImage object was created.
        inline std::uint32_t GetNumMipLevels() const
        {
            return numMipLevels_;
        }

        // Returns the number of array layers.
        inline std::uint32_t GetNumArrayLayers() const
        {
            return numArrayLayers_;
        }

        // Returns the sample count as Vulkan bit mask.
        inline VkSampleCountFlagBits GetSampleCountBits() const
        {
            return sampleCountBits_;
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return image_.GetMemoryRegion();
        }

    private:

        void CreateImage(VkDevice device, const TextureDescriptor& desc);

    private:

        VKDeviceImage           image_;
        VKPtr<VkImageView>      imageView_;

        VkFormat                format_             = VK_FORMAT_UNDEFINED;
        VkExtent3D              extent_;
        std::uint32_t           numMipLevels_       = 0;
        std::uint32_t           numArrayLayers_     = 0;
        VkSampleCountFlagBits   sampleCountBits_    = VK_SAMPLE_COUNT_1_BIT;

};


} // /namespace LLGL


#endif



// ================================================================================
