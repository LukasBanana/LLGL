/*
 * VKDevicePhysical.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PHYSICAL_DEVICE_H
#define LLGL_VK_PHYSICAL_DEVICE_H


#include "Vulkan.h"
#include "VKDevice.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


struct VKGraphicsPipelineLimits;

class VKPhysicalDevice
{

    public:

        /* ----- Common ----- */

        bool PickPhysicalDevice(VkInstance instance);

        void QueryDeviceProperties(
            RendererInfo&               info,
            RenderingCapabilities&      caps,
            VKGraphicsPipelineLimits&   pipelineLimits
        );

        VKDevice CreateLogicalDevice();

        /* ----- Handles ----- */

        // Returns the native VkPhysicalDevice handle.
        inline VkPhysicalDevice GetVkPhysicalDevice() const
        {
            return physicalDevice_;
        }

        // Returns the native VkPhysicalDevice handle.
        inline operator VkPhysicalDevice () const
        {
            return physicalDevice_;
        }

        // Returns the memory properties of the physical device.
        inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
        {
            return memoryProperties_;
        }

        // Returns the features of the physical device.
        inline const VkPhysicalDeviceFeatures& GetFeatures() const
        {
            return features_;
        }

    private:

        void QueryDeviceProperties();

        VkPhysicalDevice                    physicalDevice_     = VK_NULL_HANDLE;

        VkPhysicalDeviceMemoryProperties    memoryProperties_;
        VkPhysicalDeviceFeatures            features_;

};


} // /namespace LLGL


#endif



// ================================================================================
