/*
 * VKDeviceMemory.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_MEMORY_H
#define LLGL_VK_DEVICE_MEMORY_H


#include "../Vulkan.h"
#include "../VKPtr.h"


namespace LLGL
{


class VKDeviceMemory
{

    public:

        VKDeviceMemory(const VKPtr<VkDevice>& device, VkDeviceSize size, std::uint32_t memoryTypeIndex);

        void* Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size);
        void Unmap(VkDevice device);

        // Returns the hardware buffer object.
        inline VkDeviceMemory Get() const
        {
            return deviceMemory_.Get();
        }

    private:

        VKPtr<VkDeviceMemory> deviceMemory_;

};


} // /namespace LLGL


#endif



// ================================================================================
