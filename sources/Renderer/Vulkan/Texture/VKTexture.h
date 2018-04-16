/*
 * VKTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

        Gs::Vector3ui QueryMipLevelSize(std::uint32_t mipLevel) const override;
        TextureDescriptor QueryDesc() const override;

        void BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion);

        void CreateImageView(VkDevice device, const TextureDescriptor& desc);

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return image_;
        }

        // Returns the Vulkan image view object.
        inline VkImageView GetVkImageView() const
        {
            return imageView_;
        }

        // Returns the number of MIP level with whereby the VkImage object was created.
        inline std::uint32_t GetNumMipLevels() const
        {
            return numMipLevels_;
        }

        // Returns the VkExtent3D with whereby the VkImage object was created.
        inline const VkExtent3D& GetExtent() const
        {
            return extent_;
        }

    private:

        void CreateImage(VkDevice device, const TextureDescriptor& desc);

        VKPtr<VkImage>          image_;
        VKPtr<VkImageView>      imageView_;

        VKDeviceMemoryRegion*   memoryRegion_   = nullptr;

        std::uint32_t           numMipLevels_   = 0;
        VkExtent3D              extent_;

};


} // /namespace LLGL


#endif



// ================================================================================
