/*
 * VKExtensionRegistry.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_EXTENSION_REGISTRY_H
#define LLGL_VK_EXTENSION_REGISTRY_H


#define LLGL_ASSERT_VK_EXT(EXT) \
    if (!LLGL::HasExtension(LLGL::VKExt::EXT)) { LLGL::TrapVKExtensionNotSupported(__FUNCTION__, "VK_" #EXT); }

#ifndef VK_LAYER_KHRONOS_VALIDATION_NAME
#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"
#endif


namespace LLGL
{


// Vulkan extension enumeration.
enum class VKExt
{
    /* Required surface extensions */
    KHR_android_surface,
    KHR_win32_surface,
    KHR_xlib_surface,

    #if LLGL_LINUX_ENABLE_WAYLAND
    KHR_wayland_surface,
    #endif

    /* Khronos extensions */
    KHR_maintenance1,
    KHR_get_physical_device_properties2,
    KHR_imageless_framebuffer,
    KHR_multiview,              // Needed for EXT_mesh_shader
    KHR_fragment_shading_rate,  // Needed for EXT_mesh_shader

    /* Multivendor extensions */
    EXT_conditional_rendering,
    EXT_conservative_rasterization,
    EXT_debug_marker,
    EXT_debug_utils,
    EXT_nested_command_buffer,
    EXT_transform_feedback,
    EXT_headless_surface,
    EXT_mesh_shader,

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

// Returns the null-terminated list of optional extensions.
const char** GetOptionalExtensions();

// Returns the type of support for the specified Vulkan instance extension.
VKExtSupport GetVulkanInstanceExtensionSupport(const char* extensionName);

// Returns true if `layerName` is a Vulkan instance layer the engine wants enabled
// for debugging (currently VK_LAYER_KHRONOS_validation).
bool VKIsInstanceDebugLayer(const char* layerName);

// Returns true if Vulkan instance extension `extensionName` is recognized by the engine
// and should be enabled. `debugLayerEnabled` gates DebugOnly-tagged extensions like
// VK_EXT_debug_report and VK_EXT_debug_utils.
bool VKIsInstanceExtensionEnabled(const char* extensionName, bool debugLayerEnabled);

// Returns a pointer to a null-terminated array of Vulkan device extension names that the LLGL
// Vulkan backend requires (VK_KHR_swapchain, VK_KHR_maintenance1, ...). Used by both the non-XR
// device-picking path and the OpenXR Vulkan binding so a device created either way ends up with
// the same set of LLGL-required extensions enabled. Return type matches GetOptionalExtensions().
const char** VKGetRequiredDeviceExtensions();


} // /namespace LLGL


#endif



// ================================================================================
