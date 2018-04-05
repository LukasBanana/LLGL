/*
 * VKDeviceMemory.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_MEMORY_MANAGER_H
#define LLGL_VK_DEVICE_MEMORY_MANAGER_H


#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKDeviceMemory;

class VKDeviceMemoryManager
{

    public:

        VKDeviceMemoryManager(const VKPtr<VkDevice>& device);

        //VKDeviceMemory* AllocDeviceMemory(const VkMemoryRequirements& requirements);

    private:

        const VKPtr<VkDevice>& device_;

};


} // /namespace LLGL


#endif



// ================================================================================
