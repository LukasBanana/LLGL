/*
 * VKInitializers.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKInitializers.h"


namespace LLGL
{


void BuildVkBufferCreateInfo(VkBufferCreateInfo& createInfo, VkDeviceSize size, VkBufferUsageFlags usage)
{
    createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.size                     = size;
    createInfo.usage                    = usage;
    createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount    = 0;
    createInfo.pQueueFamilyIndices      = nullptr;
}


} // /namespace LLGL



// ================================================================================
