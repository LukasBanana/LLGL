/*
 * VKInitializers.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_INITIALIZERS_H
#define LLGL_VK_INITIALIZERS_H


#include <vulkan/vulkan.h>


namespace LLGL
{


void BuildVkBufferCreateInfo(VkBufferCreateInfo& createInfo, VkDeviceSize size, VkBufferUsageFlags usage);


} // /namespace LLGL


#endif



// ================================================================================
