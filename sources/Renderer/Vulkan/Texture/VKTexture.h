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


namespace LLGL
{


class VKTexture : public Texture
{

    public:

        VKTexture(const VKPtr<VkDevice>& device, const TextureDescriptor& desc);

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return image_;
        }

    private:

        VKPtr<VkImage> image_;

};


} // /namespace LLGL


#endif



// ================================================================================
