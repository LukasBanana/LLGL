/*
 * VKCore.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCore.h"
#include "../../Core/Helper.h"
#include "../../Core/HelperMacros.h"


namespace LLGL
{


/* ----- Basic Functions ----- */

std::string VKErrorToStr(const VkResult errorCode)
{
    // see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkResult.html
    switch (errorCode)
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
        LLGL_CASE_TO_STR( VK_ERROR_SURFACE_LOST_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_NATIVE_WINDOW_IN_USE_KHR );
        LLGL_CASE_TO_STR( VK_SUBOPTIMAL_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_OUT_OF_DATE_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_INCOMPATIBLE_DISPLAY_KHR );
        LLGL_CASE_TO_STR( VK_ERROR_VALIDATION_FAILED_EXT );
        LLGL_CASE_TO_STR( VK_ERROR_INVALID_SHADER_NV );
        LLGL_CASE_TO_STR( VK_RESULT_RANGE_SIZE );
    }
    return ToHex(errorCode);
}

void VKThrowIfFailed(const VkResult errorCode, const std::string& info)
{
    if (errorCode != VK_SUCCESS)
        throw std::runtime_error(info + " (error code = " + VKErrorToStr(errorCode) + ")");
}

// see https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#fundamentals-versionnum
std::string VKApiVersionToString(uint32_t version)
{
    union VKVersion
    {
        uint32_t ver;
        struct
        {
            uint32_t patch : 12;
            uint32_t minor : 10;
            uint32_t major : 10;
        };
    }
    versionBits;

    versionBits.ver = version;

    return std::to_string(versionBits.major) + "." + std::to_string(versionBits.minor) + "." + std::to_string(versionBits.patch);
}

VkBool32 VKBoolean(bool value)
{
    return (value ? VK_TRUE : VK_FALSE);
}


/* ----- Query Functions ----- */


std::vector<VkLayerProperties> VKQueryInstanceLayerProperties()
{
    uint32_t propertyCount = 0;
    auto result = vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance layer properties");

    std::vector<VkLayerProperties> properties(propertyCount);
    result = vkEnumerateInstanceLayerProperties(&propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance layer properties");

    return properties;
}

std::vector<VkExtensionProperties> VKQueryInstanceExtensionProperties()
{
    uint32_t propertyCount = 0;
    auto result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan instance extension properties");

    std::vector<VkExtensionProperties> properties(propertyCount);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan instance extension properties");

    return properties;
}

std::vector<VkPhysicalDevice> VKQueryPhysicalDevices(VkInstance instance)
{
    uint32_t deviceCount = 0;
    auto result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan physical devices");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    VKThrowIfFailed(result, "failed to query Vulkan physical devices");

    return devices;
}

std::vector<VkExtensionProperties> VKQueryDeviceExtensionProperties(VkPhysicalDevice device)
{
    uint32_t propertyCount = 0;
    auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan device extension properties");

    std::vector<VkExtensionProperties> properties(propertyCount);
    result = vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, properties.data());
    VKThrowIfFailed(result, "failed to query Vulkan device extension properties");

    return properties;
}

std::vector<VkQueueFamilyProperties> VKQueryQueueFamilyProperties(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

SwapChainSupportDetails VKQuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    /* Query surface capabilities */
    auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.caps);
    VKThrowIfFailed(result, "failed to query Vulkan surface capabilities");

    /* Query surface formats */
    uint32_t formatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    VKThrowIfFailed(result, "failed to query number of Vulkan surface formats");

    if (formatCount > 0)
    {
        details.formats.resize(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        VKThrowIfFailed(result, "failed to query Vulkan surface formats");
    }

    /* Query surface present modes */
    uint32_t presentModeCount;
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

QueueFamilyIndices VKFindQueueFamilies(VkPhysicalDevice device, const VkQueueFlags flags)
{
    QueueFamilyIndices indices;

    auto queueFamilies = VKQueryQueueFamilyProperties(device);

    uint32_t i = 0;
    for (const auto& family : queueFamilies)
    {
        /* Get graphics family index */
        if (family.queueCount > 0 && (family.queueFlags & flags) != 0)
            indices.graphicsFamily = i;

        #if 0
        /* Get present family index */
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupport);

        if (family.queueCount > 0 && presentSupport)
            indices.presentFamily = i;
        #else
        if (family.queueCount > 0)
            indices.presentFamily = i;
        #endif

        /* Check if queue family is complete */
        if (indices.Complete())
            break;

        ++i;
    }

    return indices;
}


} // /namespace LLGL



// ================================================================================
