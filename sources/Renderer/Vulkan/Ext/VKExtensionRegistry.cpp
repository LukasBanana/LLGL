/*
 * VKExtensionRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKExtensionRegistry.h"
#include "../Vulkan.h"
#include <LLGL/Container/Strings.h>
#include <LLGL/Platform/Platform.h>
#include <cstring>


namespace LLGL
{


static bool g_VKRegisteredExtensions[static_cast<std::size_t>(VKExt::Count)] = {};

static const char* g_VKOptionalExtensions[] =
    {
#if VK_EXT_conditional_rendering
        VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
    #endif
    #if VK_EXT_conservative_rasterization
        VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    #endif
    #if VK_EXT_debug_marker
        VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    #endif
    #if VK_EXT_debug_report
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    #endif
    #if VK_EXT_debug_utils
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    #endif
    #if VK_KHR_get_physical_device_properties2
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    #endif
    #if VK_KHR_imageless_framebuffer
        VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME,
    #endif
    #if VK_KHR_image_format_list
        // Required for VK_KHR_imageless_framebuffer
        VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
    #endif
    #if VK_KHR_portability_enumeration
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    #endif
    #if VK_KHR_sampler_mirror_clamp_to_edge
        VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    #endif
    #if VK_EXT_transform_feedback
        VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
    #endif
    #if VK_EXT_nested_command_buffer
        VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
    #endif
    #if VK_EXT_headless_surface
        VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
    #endif
    #if VK_KHR_fragment_shading_rate
        VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
    #endif
    #if VK_KHR_create_renderpass2
        // Required by VK_KHR_fragment_shading_rate
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    #endif
    #if VK_KHR_multiview
        VK_KHR_MULTIVIEW_EXTENSION_NAME,
    #endif
    #if VK_EXT_mesh_shader
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
    #endif
    #if VK_KHR_spirv_1_4
        // Required for VK_EXT_mesh_shader
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    #endif
    #if VK_KHR_shader_float_controls
        // Required by VK_KHR_spirv_1_4
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    #endif
    #if VK_GOOGLE_hlsl_functionality1
        // Adds support for reflection in HLSL shaders cross-compiled to SPIR-V
        VK_GOOGLE_HLSL_FUNCTIONALITY1_EXTENSION_NAME,
    #endif
    #if VK_EXT_shader_viewport_index_layer
        // For enhanced shader control over viewport and layer selection in multiview and array rendering
        VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME,
    #endif
        nullptr,
};

void RegisterExtension(VKExt extension)
{
    g_VKRegisteredExtensions[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const VKExt extension)
{
    return g_VKRegisteredExtensions[static_cast<std::size_t>(extension)];
}

const char** GetOptionalExtensions()
{
    return g_VKOptionalExtensions;
}

static bool IsVulkanInstanceExtRequired(const StringView& name)
{
    return
    (
        name == VK_KHR_SURFACE_EXTENSION_NAME
        #if defined LLGL_OS_WIN32
        || name == VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        #elif defined LLGL_OS_LINUX
        || name == VK_KHR_XLIB_SURFACE_EXTENSION_NAME
        #if LLGL_LINUX_ENABLE_WAYLAND
        || name == VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
        #endif
        #elif defined LLGL_OS_ANDROID
        || name == VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
        #elif defined LLGL_OS_MACOS || defined LLGL_OS_IOS
        || name == VK_EXT_METAL_SURFACE_EXTENSION_NAME
        #endif
    );
}

static bool IsVulkanInstanceExtOptional(const StringView& name)
{
    return
    (
        false
        #if VK_KHR_get_physical_device_properties2
        || name == VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        #endif
        #if VK_KHR_portability_enumeration
        || name == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        #endif
        #if VK_EXT_headless_surface
        || name == VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME
        #endif
    );
}

static bool IsVulkanInstanceExtDebugOnly(const StringView& name)
{
    return
    (
        false
        #if VK_EXT_debug_marker
        || name == VK_EXT_DEBUG_MARKER_EXTENSION_NAME
        #endif
        #if VK_EXT_debug_report
        || name == VK_EXT_DEBUG_REPORT_EXTENSION_NAME
        #endif
        #if VK_EXT_debug_utils
        || name == VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        #endif
    );
}

VKExtSupport GetVulkanInstanceExtensionSupport(const char* extensionName)
{
    const StringView name = extensionName;
    if (IsVulkanInstanceExtRequired(name))
        return VKExtSupport::Required;
    if (IsVulkanInstanceExtOptional(name))
        return VKExtSupport::Optional;
    if (IsVulkanInstanceExtDebugOnly(name))
        return VKExtSupport::DebugOnly;
    return VKExtSupport::Unsupported;
}

bool VKIsInstanceDebugLayer(const char* layerName)
{
    return (layerName != nullptr && std::strcmp(layerName, VK_LAYER_KHRONOS_VALIDATION_NAME) == 0);
}

bool VKIsInstanceExtensionEnabled(const char* extensionName, bool debugLayerEnabled)
{
    if (extensionName == nullptr)
        return false;
    const VKExtSupport extSupport = GetVulkanInstanceExtensionSupport(extensionName);
    return
    (
        extSupport == VKExtSupport::Required ||
        extSupport == VKExtSupport::Optional ||
        (debugLayerEnabled && extSupport == VKExtSupport::DebugOnly)
    );
}

const char** VKGetRequiredDeviceExtensions()
{
    static const char* required[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
        nullptr,
    };
    return required;
}


} // /namespace LLGL



// ================================================================================
