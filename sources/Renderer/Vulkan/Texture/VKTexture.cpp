/*
 * VKTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTexture.h"
#include "../VKTypes.h"
#include "../VKCore.h"


namespace LLGL
{


VKTexture::VKTexture(const VKPtr<VkDevice>& device, const TextureDescriptor& desc) :
    Texture { desc.type              },
    image_  { device, vkDestroyImage }
{
    /* Create sampler state */
    VkImageCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.imageType                = VKTypes::Map(desc.type);
        createInfo.format                   = VKTypes::Map(desc.format);
        createInfo.extent.width             = desc.texture3D.width;
        createInfo.extent.height            = desc.texture3D.height;
        createInfo.extent.depth             = desc.texture3D.depth;
        createInfo.mipLevels                = 1;
        createInfo.arrayLayers              = 1;//!!!
        createInfo.samples                  = VK_SAMPLE_COUNT_1_BIT;//!!!
        createInfo.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage                    = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE; // only used by graphics queue
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    VkResult result = vkCreateImage(device, &createInfo, nullptr, image_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan image");
}


} // /namespace LLGL



// ================================================================================
