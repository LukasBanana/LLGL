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


namespace LLGL
{


static bool g_VKRegisteredExtensions[static_cast<std::size_t>(VKExt::Count)] = {};

static const char* g_VKOptionalExtensions[] =
{
    #if VK_KHR_sampler_mirror_clamp_to_edge
    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    #endif
    #if VK_KHR_get_physical_device_properties2
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    #endif
    #if VK_EXT_debug_marker
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    #endif
    #if VK_EXT_conditional_rendering
    VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
    #endif
    #if VK_EXT_conservative_rasterization
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    #endif
    #if VK_EXT_transform_feedback
    VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
    #endif
    #if VK_EXT_nested_command_buffer
    VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME,
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
    );
}

static bool IsVulkanInstanceExtDebugOnly(const StringView& name)
{
    return
    (
        false
        #if VK_EXT_debug_report
        || name == VK_EXT_DEBUG_REPORT_EXTENSION_NAME
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


} // /namespace LLGL



// ================================================================================
