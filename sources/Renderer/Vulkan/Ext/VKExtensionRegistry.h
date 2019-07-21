/*
 * VKExtensionRegistry.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_EXTENSION_REGISTRY_H
#define LLGL_VK_EXTENSION_REGISTRY_H


namespace LLGL
{


// Vulkan extension enumeration.
enum class VKExt
{
    /* Khronos extensions */
    KHR_maintenance1,

    /* Multivendor extensions */
    EXT_debug_marker,
    EXT_conditional_rendering,

    /* Enumeration entry counter */
    Count,
};


// Registers the specified Vulkan extension support.
void RegisterExtension(VKExt extension);

// Returns true if the specified Vulkan extension is supported.
bool HasExtension(const VKExt extension);


} // /namespace LLGL


#endif



// ================================================================================
