/*
 * VKImageUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_IMAGE_UTILS_H
#define LLGL_VK_IMAGE_UTILS_H


#include <cstdint>
#include <vulkan/vulkan.h>


namespace LLGL
{

namespace VKImageUtils
{


// Returns the image aspect for the specified Vulkan format
VkImageAspectFlags GetInclusiveVkImageAspect(VkFormat format);

// Returns the image aspect for the specified Vulkan format
VkImageAspectFlags GetExclusiveVkImageAspect(VkFormat format, bool preferStencilComponent = false);


} // /namespace VKImageUtils

} // /namespace LLGL


#endif



// ================================================================================
