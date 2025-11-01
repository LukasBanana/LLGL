/*
 * VKImageUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_IMAGE_UTILS_H
#define LLGL_VK_IMAGE_UTILS_H


#include <cstdint>
#include "../Vulkan.h"
#include <LLGL/TextureFlags.h>


namespace LLGL
{

namespace VKImageUtils
{


// Initializes VkImageResolve or VkImageCopy.
template <typename TDst>
void InitVkImageRegion(
    TDst&                   outRegion,
    const TextureRegion&    textureRegion,
    const Offset3D&         srcOffset,
    VkImageAspectFlags      srcAspectFlags,
    VkImageAspectFlags      dstAspectFlags)
{
    outRegion.srcSubresource.aspectMask     = srcAspectFlags;
    outRegion.srcSubresource.mipLevel       = textureRegion.subresource.baseMipLevel;
    outRegion.srcSubresource.baseArrayLayer = textureRegion.subresource.baseArrayLayer;
    outRegion.srcSubresource.layerCount     = textureRegion.subresource.numArrayLayers;
    outRegion.srcOffset.x                   = srcOffset.x;
    outRegion.srcOffset.y                   = srcOffset.y;
    outRegion.srcOffset.z                   = srcOffset.z;
    outRegion.dstSubresource.aspectMask     = dstAspectFlags;
    outRegion.dstSubresource.mipLevel       = textureRegion.subresource.baseMipLevel;
    outRegion.dstSubresource.baseArrayLayer = textureRegion.subresource.baseArrayLayer;
    outRegion.dstSubresource.layerCount     = textureRegion.subresource.numArrayLayers;
    outRegion.dstOffset.x                   = textureRegion.offset.x;
    outRegion.dstOffset.y                   = textureRegion.offset.y;
    outRegion.dstOffset.z                   = textureRegion.offset.z;
    outRegion.extent.width                  = textureRegion.extent.width;
    outRegion.extent.height                 = textureRegion.extent.height;
    outRegion.extent.depth                  = textureRegion.extent.depth;
}

// Returns the image aspect for the specified Vulkan format
VkImageAspectFlags GetInclusiveVkImageAspect(VkFormat format);

// Returns the image aspect for the specified Vulkan format
VkImageAspectFlags GetExclusiveVkImageAspect(VkFormat format, bool preferStencilComponent = false);


} // /namespace VKImageUtils

} // /namespace LLGL


#endif



// ================================================================================
