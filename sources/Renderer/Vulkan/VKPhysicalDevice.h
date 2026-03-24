/*
 * VKDevicePhysical.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PHYSICAL_DEVICE_H
#define LLGL_VK_PHYSICAL_DEVICE_H


#include "Vulkan.h"
#include "VKDevice.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <vector>
#include <set>
#include <cstring>


namespace LLGL
{


struct VKGraphicsPipelineLimits;

struct VKPhysicalDevicePropertiesExt : VkPhysicalDeviceProperties
{
    /* Extension based physical device properties */
    #if VK_EXT_conservative_rasterization
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT  conservRaster;
    #endif
    #if VK_EXT_nested_command_buffer
    VkPhysicalDeviceNestedCommandBufferPropertiesEXT        nestedCmdBuffer;
    #endif
    #if VK_EXT_transform_feedback
    VkPhysicalDeviceTransformFeedbackPropertiesEXT          transformFeedback;
    #endif
    #if VK_KHR_multiview
    VkPhysicalDeviceMultiviewPropertiesKHR                  multiview;
    #endif
    #if VK_KHR_fragment_shading_rate
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR        fragmentShadingRate;
    #endif
    #if VK_EXT_mesh_shader
    VkPhysicalDeviceMeshShaderPropertiesEXT                 meshShader;
    #endif
};

struct VKPhysicalDeviceFeaturesExt : VkPhysicalDeviceFeatures
{
    /* Extension based physical device features */
    #if VK_EXT_nested_command_buffer
    VkPhysicalDeviceNestedCommandBufferFeaturesEXT          nestedCmdBuffer;
    #endif
    #if VK_EXT_transform_feedback
    VkPhysicalDeviceTransformFeedbackFeaturesEXT            transformFeedback;
    #endif
    #if VK_KHR_multiview
    VkPhysicalDeviceMultiviewFeaturesKHR                    multiview;
    #endif
    #if VK_KHR_fragment_shading_rate
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR          fragmentShadingRate;
    #endif
    #if VK_EXT_mesh_shader
    VkPhysicalDeviceMeshShaderFeaturesEXT                   meshShader;
    #endif
    #if VK_KHR_imageless_framebuffer
    VkPhysicalDeviceImagelessFramebufferFeaturesKHR         imagelessFramebuffer;
    #endif
};

class VKPhysicalDevice
{

    public:

        /* ----- Common ----- */

        // Picks the physical Vulkan device by enumerating the available devices from the specified Vulkan instance.
        bool PickPhysicalDevice(VkInstance instance, const ArrayView<const char*>& supportedInstanceExtensions, long preferredDeviceFlags = 0);

        // Loads the physical Vulkan device from a custom native handle.
        void LoadPhysicalDeviceWeakRef(VkPhysicalDevice physicalDevice);

        void QueryRendererInfo(RendererInfo& outInfo);
        void QueryRenderingCaps(RenderingCapabilities& outCaps);
        void QueryPipelineLimits(VKGraphicsPipelineLimits& outPipelineLimits);

        VKDevice CreateLogicalDevice(VkDevice customLogicalDevice = VK_NULL_HANDLE);

        std::uint32_t FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const;

        // Returns true if the specified Vulkan extension is supported by this physical device.
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
        inline const VKPhysicalDeviceFeaturesExt& GetFeatures() const
        {
            return features_;
        }

        // Returns the Vulkan specific limits of the physical device.
        inline const VKPhysicalDevicePropertiesExt& GetProperties() const
        {
            return properties_;
        }

        // Returns the memory properties of the physical device.
        inline const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const
        {
            return memoryProperties_;
        }

        // Returns the list of names of all supported and enabled extensions.
        inline const std::vector<const char*>& GetExtensionNames() const
        {
            return enabledExtensionNames_;
        }

    private:

        // Helper struct to compare ANSI-C strings in a strict-weak-order (SWO)
        struct CStringSWO
        {
            inline bool operator ()(const char* lhs, const char* rhs) const
            {
                return (std::strcmp(lhs, rhs) < 0);
            }
        };

    private:

        bool EnableExtensions(const char** extensions, bool required = false);

        void QueryDeviceInfo();
        void QueryDeviceFeatures();
        void QueryDeviceProperties();
        void QueryDeviceMemoryProperties();

    private:

        // Main device objects
        VkPhysicalDevice                    physicalDevice_             = VK_NULL_HANDLE;
        std::vector<VkExtensionProperties>  supportedExtensions_;
        std::set<const char*, CStringSWO>   supportedExtensionNames_;
        std::vector<const char*>            enabledExtensionNames_;

        // Common device properties and features
        VkPhysicalDeviceFeatures2           features2_                  = {}; // Use for chaining pNext entries only
        VKPhysicalDeviceFeaturesExt         features_                   = {};
        VKPhysicalDevicePropertiesExt       properties_                 = {};
        VkPhysicalDeviceMemoryProperties    memoryProperties_           = {};

};


} // /namespace LLGL


#endif



// ================================================================================
