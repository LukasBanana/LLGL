/*
 * VKTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_TEXTURE_H
#define LLGL_VK_TEXTURE_H


#include <LLGL/Texture.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <cstdint>


namespace LLGL
{


class VKDeviceMemoryRegion;

class VKTexture : public Texture
{

    public:

        VKTexture(const VKPtr<VkDevice>& device, const TextureDescriptor& desc);

        Extent3D QueryMipLevelSize(std::uint32_t mipLevel) const override;
        TextureDescriptor QueryDesc() const override;

        void BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void CreateImageView(
            VkDevice device,
            std::uint32_t baseMipLevel, std::uint32_t numMipLevels,
            std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers,
            VkImageView* imageViewRef
        );

        void CreateInternalImageView(VkDevice device);

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return image_;
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

    private:

        void CreateImage(VkDevice device, const TextureDescriptor& desc);

        VKPtr<VkImage>          image_;
        VKPtr<VkImageView>      imageView_;

        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

        VkFormat                format_         = VK_FORMAT_UNDEFINED;
        VkExtent3D              extent_;
        std::uint32_t           numMipLevels_   = 0;
        std::uint32_t           numArrayLayers_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
