/*
 * VKCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_CORE_H
#define LLGL_VK_CORE_H


#include "Vulkan.h"
#include <string>
#include <vector>
#include <cstdint>


namespace LLGL
{


/* ----- Macros ----- */

#define LLGL_VK_FAILED(RESULT)      ((RESULT) < 0)
#define LLGL_VK_SUCCEEDED(RESULT)   ((RESULT) >= 0)


/* ----- Structures ----- */

struct alignas(alignof(std::uint32_t)) VKQueueFamilyIndices
{
    static constexpr std::uint32_t invalidIndex = ~0u;

    std::uint32_t graphicsFamily    = invalidIndex;
    std::uint32_t presentFamily     = invalidIndex;
//  std::uint32_t transferFamily    = invalidIndex;

    // Returns a pointer to the number of indices
    inline const std::uint32_t* Ptr() const
    {
        static_assert(
            offsetof(VKQueueFamilyIndices, graphicsFamily) == 0,
            "LLGL::VKQueueFamilyIndices::graphicsFamily is expected to have memory offset 0"
        );
        return (&graphicsFamily);
    }

    // Returns the number of indices this structure has.
    inline std::uint32_t Count() const
    {
        return sizeof(VKQueueFamilyIndices)/sizeof(std::uint32_t);
    }

    // Returns true if all indices have been set to a valid index.
    inline bool Complete() const
    {
        return (graphicsFamily != invalidIndex && presentFamily != invalidIndex);
    }
};

struct VKSurfaceSupportDetails
{
    VkSurfaceCapabilitiesKHR        caps            = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


/* ----- Basic Functions ----- */

// Traps program execution if 'result' is not VK_SUCCESS.
void VKThrowIfFailed(const VkResult result, const char* details);

// Traps program execution if 'result' is not VK_SUCCESS, with an info about the failed interface creation.
void VKThrowIfCreateFailed(const VkResult result, const char* interfaceName, const char* contextInfo = nullptr);

// Converts the specified Vulkan API version into a string (e.g. "1.0.100").
std::string VKApiVersionToString(std::uint32_t version);

// Converts the boolean value into a VkBool322 value.
VkBool32 VKBoolean(bool value);



/* ----- Query Functions ----- */

std::vector<VkLayerProperties> VKQueryInstanceLayerProperties();
std::vector<VkExtensionProperties> VKQueryInstanceExtensionProperties(const char* layerName = nullptr);
std::vector<VkPhysicalDevice> VKQueryPhysicalDevices(VkInstance instance);
std::vector<VkExtensionProperties> VKQueryDeviceExtensionProperties(VkPhysicalDevice device);
std::vector<VkQueueFamilyProperties> VKQueryQueueFamilyProperties(VkPhysicalDevice device);

VKSurfaceSupportDetails VKQuerySurfaceSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VKQueueFamilyIndices VKFindQueueFamilies(VkPhysicalDevice device, const VkQueueFlags flags, VkSurfaceKHR* surface = nullptr);
VkFormat VKFindSupportedImageFormat(VkPhysicalDevice device, const VkFormat* candidates, std::size_t numCandidates, VkImageTiling tiling, VkFormatFeatureFlags features);

// Returns the memory type index that supports the specified type bits and properties, or traps program execution on failure.
std::uint32_t VKFindMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);


} // /namespace LLGL


#endif



// ================================================================================
