/*
 * VKExtensionRegistry.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKExtensionRegistry.h"
#include "../Vulkan.h"
#include "../../../Core/Exception.h"
#include <array>


namespace LLGL
{


static std::array<bool, static_cast<std::size_t>(VKExt::Count)> g_registeredExtensions { { false } };

static const char* g_optionalExtensions[] =
{
    VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
    VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
    VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME,
    //VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
    nullptr,
};

void RegisterExtension(VKExt extension)
{
    g_registeredExtensions[static_cast<std::size_t>(extension)] = true;
}

bool HasExtension(const VKExt extension)
{
    return g_registeredExtensions[static_cast<std::size_t>(extension)];
}

void AssertExtension(const VKExt extension, const char* extensionName, const char* funcName)
{
    if (!HasExtension(extension))
        ThrowVKExtensionNotSupportedExcept(funcName, extensionName);
}

const char** GetOptionalExtensions()
{
    return g_optionalExtensions;
}


} // /namespace LLGL



// ================================================================================
