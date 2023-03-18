/*
 * VKExtensionRegistry.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_EXTENSION_REGISTRY_H
#define LLGL_VK_EXTENSION_REGISTRY_H


namespace LLGL
{


#define LLGL_ASSERT_VK_EXTENSION(EXT, EXTNAME) \
    AssertExtension(EXT, EXTNAME, __FUNCTION__)

// Vulkan extension enumeration.
enum class VKExt
{
    /* Khronos extensions */
    KHR_maintenance1,
    KHR_get_physical_device_properties2,

    /* Multivendor extensions */
    EXT_debug_marker,
    EXT_conditional_rendering,
    EXT_transform_feedback,
    EXT_conservative_rasterization,

    /* Enumeration entry counter */
    Count,
};

// Vulkan extension support enumeration.
enum class VKExtSupport
{
    Unsupported,    // Vulkan extension is unsupported and will not be loaded.
    Optional,       // Vulkan extension is supported but optional.
    DebugOnly,      // Vulkan extension is supported but only used for debugging.
    Required,       // Vulkan extension is supported and required.
};


// Registers the specified Vulkan extension support.
void RegisterExtension(VKExt extension);

// Returns true if the specified Vulkan extension is supported.
bool HasExtension(const VKExt extension);

// Throws an exception if the specified Vulkan extension is not supported.
void AssertExtension(const VKExt extension, const char* extensionName, const char* funcName);

// Returns the null-terminated list of optional extensions.
const char** GetOptionalExtensions();

// Returns the type of support for the specified Vulkan instance extension.
VKExtSupport GetVulkanInstanceExtensionSupport(const char* extensionName);


} // /namespace LLGL


#endif



// ================================================================================
