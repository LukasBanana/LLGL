/*
 * VKCore.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCore.h"
#include "../../Core/StringUtils.h"
#include "../../Core/MacroUtils.h"
#include "../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


/* ----- Basic Functions ----- */

static const char* VKResultToStr(const VkResult result)
{
    // see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkResult.html
    switch (result)
    {
        LLGL_CASE_TO_STR( VK_SUCCESS );
        LLGL_CASE_TO_STR( VK_NOT_READY );
        LLGL_CASE_TO_STR( VK_TIMEOUT );
        LLGL_CASE_TO_STR( VK_EVENT_SET );
        LLGL_CASE_TO_STR( VK_EVENT_RESET );
        LLGL_CASE_TO_STR( VK_INCOMPLETE );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_HOST_MEMORY );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_DEVICE_MEMORY );
        LLGL_CASE_TO_STR( VK_ERROR_INITIALIZATION_FAILED );
        LLGL_CASE_TO_STR( VK_ERROR_DEVICE_LOST );
        LLGL_CASE_TO_STR( VK_ERROR_MEMORY_MAP_FAILED );
        LLGL_CASE_TO_STR( VK_ERROR_LAYER_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_EXTENSION_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_FEATURE_NOT_PRESENT );
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_DRIVER );
        LLGL_CASE_TO_STR( VK_ERROR_TOO_MANY_OBJECTS );
        LLGL_CASE_TO_STR( VK_ERROR_FORMAT_NOT_SUPPORTED );
        LLGL_CASE_TO_STR( VK_ERROR_FRAGMENTED_POOL );
        #if VK_HEADER_VERSION >= 131
        LLGL_CASE_TO_STR( VK_ERROR_UNKNOWN );
        #endif
        #ifdef VK_VERSION_1_1
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_POOL_MEMORY );
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_EXTERNAL_HANDLE );
        #endif // VK_VERSION_1_1
        #ifdef VK_VERSION_1_2
        LLGL_CASE_TO_STR( VK_ERROR_FRAGMENTATION );
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS );
        #endif // VK_VERSION_1_2
        #ifdef VK_KHR_surface
        LLGL_CASE_TO_STR( VK_ERROR_SURFACE_LOST_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_NATIVE_WINDOW_IN_USE_KHR );
        #endif // VK_KHR_surface
        #ifdef VK_KHR_swapchain
        LLGL_CASE_TO_STR( VK_SUBOPTIMAL_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_DATE_KHR );
        #endif // VK_KHR_swapchain
        #ifdef VK_KHR_display_swapchain
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_DISPLAY_KHR );
        #endif // VK_KHR_display_swapchain
        #ifdef VK_EXT_debug_report
        LLGL_CASE_TO_STR( VK_ERROR_VALIDATION_FAILED_EXT );
        #endif // VK_EXT_debug_report
        #ifdef VK_NV_glsl_shader
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_SHADER_NV );
        #endif // VK_NV_glsl_shader
        #ifdef VK_KHR_ray_tracing
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_VERSION_KHR );
        #endif // VK_KHR_ray_tracing
        #ifdef VK_EXT_image_drm_format_modifier
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT );
        #endif // VK_EXT_image_drm_format_modifier
        #ifdef VK_EXT_global_priority
        LLGL_CASE_TO_STR( VK_ERROR_NOT_PERMITTED_EXT );
        #endif // VK_EXT_global_priority
        #ifdef VK_EXT_full_screen_exclusive
        LLGL_CASE_TO_STR( VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT );
        #endif // VK_EXT_full_screen_exclusive
        #ifdef VK_KHR_deferred_host_operations
        LLGL_CASE_TO_STR( VK_THREAD_IDLE_KHR );
        LLGL_CASE_TO_STR( VK_THREAD_DONE_KHR );
        LLGL_CASE_TO_STR( VK_OPERATION_DEFERRED_KHR );
        LLGL_CASE_TO_STR( VK_OPERATION_NOT_DEFERRED_KHR );
        #endif // VK_KHR_deferred_host_operations
        #ifdef VK_EXT_pipeline_creation_cache_control
        LLGL_CASE_TO_STR( VK_PIPELINE_COMPILE_REQUIRED_EXT );
        #endif // VK_EXT_pipeline_creation_cache_control
        default: break;
    }
    return nullptr;
}

static const char* VKResultToStrOrHex(const VkResult result)
{
    if (const char* err = VKResultToStr(result))
        return err;
    else
        return IntToHex(static_cast<int>(result));
}

void VKThrowIfFailed(const VkResult result, const char* details)
{
    if (LLGL_VK_FAILED(result))
    {
        const char* resultStr = VKResultToStrOrHex(result);
        if (details != nullptr && *details != '\0')
            LLGL_TRAP("%s (error code = %s)", details, resultStr);
        else
            LLGL_TRAP("Vulkan operation failed (error code = %s)", resultStr);
    }
}

void VKThrowIfCreateFailed(const VkResult result, const char* interfaceName, const char* contextInfo)
{
    if (LLGL_VK_FAILED(result))
    {
        std::string s;
        {
            s = "failed to create instance of <";
            s += interfaceName;
            s += '>';
            if (contextInfo != nullptr)
            {
                s += ' ';
                s += contextInfo;
            }
        }
        VKThrowIfFailed(result, s.c_str());
    }
}

// see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#fundamentals-versionnum
std::string VKApiVersionToString(std::uint32_t version)
{
    std::string s;

    s += std::to_string(VK_VERSION_MAJOR(version));
    s += '.';
    s += std::to_string(VK_VERSION_MINOR(version));
    s += ".";
    s += std::to_string(VK_VERSION_PATCH(version));

    return s;
}

VkBool32 VKBoolean(bool value)
{
    return (value ? VK_TRUE : VK_FALSE);
}


/* ----- Query Functions ----- */


std::vector<VkLayerProperties> VKQueryInstanceLayerProperties()
{
    std::uint32_t propertyCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance layer properties");

    std::vector<VkLayerProperties> properties(propertyCount);
    result = vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance layer properties");

    return properties;
}

std::vector<VkExtensionProperties> VKQueryInstanceExtensionProperties(const char* layerName)
{
    std::uint32_t propertyCount = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance extension properties");

    std::vector<VkExtensionProperties> properties(propertyCount);
    result = vkEnumerateInstanceExtensionProperties(layerName, &propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance extension properties");

    return properties;
}

std::vector<VkPhysicalDevice> VKQueryPhysicalDevices(VkInstance instance)
{
    std::uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan physical devices");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    VKThrowIfFailed(result, "failed to query Vulkan physical devices");

    return devices;
}

std::vector<VkExtensionProperties> VKQueryDeviceExtensionProperties(VkPhysicalDevice device)
{
    std::uint32_t propertyCount = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan device extension properties");

    std::vector<VkExtensionProperties> properties(propertyCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan device extension properties");

    return properties;
}

std::vector<VkQueueFamilyProperties> VKQueryQueueFamilyProperties(VkPhysicalDevice device)
{
    std::uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

VKSurfaceSupportDetails VKQuerySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VKSurfaceSupportDetails details;

    /* Query surface capabilities */
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &(details.caps));
    VKThrowIfFailed(result, "failed to query Vulkan surface capabilities");

    /* Query surface formats */
    std::uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan surface formats");

    if (formatCount > 0)
    {
        details.formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        VKThrowIfFailed(result, "failed to query Vulkan surface formats");
    }

    /* Query surface present modes */
    std::uint32_t presentModeCount;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan surface present modes");

    if (presentModeCount > 0)
    {
        details.presentModes.resize(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        VKThrowIfFailed(result, "failed to query Vulkan surface present modes");
    }

    return details;
}

VKQueueFamilyIndices VKFindQueueFamilies(VkPhysicalDevice device, const VkQueueFlags flags, VkSurfaceKHR* surface)
{
    VKQueueFamilyIndices indices;

    const std::vector<VkQueueFamilyProperties> queueFamilies = VKQueryQueueFamilyProperties(device);

    std::uint32_t i = 0;
    for (const VkQueueFamilyProperties& family : queueFamilies)
    {
        /* Get graphics family index */
        if (family.queueCount > 0 && (family.queueFlags & flags) != 0)
            indices.graphicsFamily = i;

        if (surface != nullptr)
        {
            /* Get present family index */
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupport);

            if (family.queueCount > 0 && presentSupport)
                indices.presentFamily = i;
        }
        else
        {
            if (family.queueCount > 0)
                indices.presentFamily = i;
        }

        /* Check if queue family is complete */
        if (indices.Complete())
            break;

        ++i;
    }

    return indices;
}

VkFormat VKFindSupportedImageFormat(VkPhysicalDevice device, const VkFormat* candidates, std::size_t numCandidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for_range(i, numCandidates)
    {
        VkFormat format = candidates[i];

        /* Query physics device properties of current image format */
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device, format, &properties);

        /* Check if features are supported */
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            return format;
    }
    LLGL_TRAP("failed to find suitable image format");
}

std::uint32_t VKFindMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
    for_range(i, memoryProperties.memoryTypeCount)
    {
        if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    LLGL_TRAP("failed to find suitable Vulkan memory type");
}


} // /namespace LLGL



// ================================================================================
