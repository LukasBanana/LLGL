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
#include <vector>


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

        std::uint32_t FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const;

        // Returns true if the specified Vulkan extension is supported bny this physical device.
        bool SupportsExtension(const char* extension) const;

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

        // Returns the Vulkan specific features of the physical device.
        inline const VkPhysicalDeviceFeatures& GetFeatures() const
        {
            return features_;
        }

        // Returns the Vulkan specific limits of the physical device.
        inline const VkPhysicalDeviceProperties& GetProperties() const
        {
            return properties_;
        }

        // Returns the memory properties of the physical device.
        inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
        {
            return memoryProperties_;
        }

        // Returns the list of supported extensions names.
        inline const std::vector<const char*>& GetSupportedExtensionNames() const
        {
            return supportedExtensionNames_;
        }

    private:

        void QueryDeviceProperties();

    private:

        VkPhysicalDevice                    physicalDevice_             = VK_NULL_HANDLE;
        std::vector<VkExtensionProperties>  supportedExtensions_;
        std::vector<const char*>            supportedExtensionNames_;

        VkPhysicalDeviceFeatures            features_;
        VkPhysicalDeviceProperties          properties_;
        VkPhysicalDeviceMemoryProperties    memoryProperties_;

};


} // /namespace LLGL


#endif



// ================================================================================
