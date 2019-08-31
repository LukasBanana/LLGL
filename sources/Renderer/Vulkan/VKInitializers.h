/*
 * VKInitializers.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
