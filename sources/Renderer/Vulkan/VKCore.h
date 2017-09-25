/*
 * VKCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_CORE_H
#define LLGL_VK_CORE_H


#include "Vulkan.h"
#include <string>
#include <vector>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

struct QueueFamilyIndices
{
    static const std::uint32_t invalidIndex = 0xffffffff;

    QueueFamilyIndices() :
        graphicsFamily { invalidIndex },
        presentFamily  { invalidIndex }
    {
    }

    union
    {
        std::uint32_t indices[2];
        struct
        {
            std::uint32_t graphicsFamily;
            std::uint32_t presentFamily;
            //std::uint32_t transferFamily;
        };
    };

    inline bool Complete() const
    {
        return (graphicsFamily != invalidIndex && presentFamily != invalidIndex);
    }
};

struct SurfaceSupportDetails
{
    VkSurfaceCapabilitiesKHR        caps;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


/* ----- Basic Functions ----- */

// Converts the DX error code into a string.
std::string VKErrorToStr(const VkResult errorCode);

// Throws an std::runtime_error exception if 'errorCode' is not VK_SUCCESS.
void VKThrowIfFailed(const VkResult errorCode, const std::string& info);

// Converts the specified Vulkan API version into a string (e.g. "1.0.100").
std::string VKApiVersionToString(std::uint32_t version);

//! Converts the boolean value into a VkBool322 value.
VkBool32 VKBoolean(bool value);



/* ----- Query Functions ----- */

std::vector<VkLayerProperties> VKQueryInstanceLayerProperties();
std::vector<VkExtensionProperties> VKQueryInstanceExtensionProperties();
std::vector<VkPhysicalDevice> VKQueryPhysicalDevices(VkInstance instance);
std::vector<VkExtensionProperties> VKQueryDeviceExtensionProperties(VkPhysicalDevice device);
std::vector<VkQueueFamilyProperties> VKQueryQueueFamilyProperties(VkPhysicalDevice device);

SurfaceSupportDetails VKQuerySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
QueueFamilyIndices VKFindQueueFamilies(VkPhysicalDevice device, const VkQueueFlags flags);


} // /namespace LLGL


#endif



// ================================================================================
