/*
 * VKExtensionLoader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_EXTENSION_LOADER_H
#define LLGL_VK_EXTENSION_LOADER_H


#include "../Vulkan.h"
#include <vector>


namespace LLGL
{


// Loads all Vulkan extensions via the specified VkInstance handle.
bool VKLoadInstanceExtensions(VkInstance instance);

// Loads all Vulkan extensions via the specified VkDevice handle.
bool VKLoadDeviceExtensions(VkDevice device, const std::vector<const char*>& supportedExtensions);


} // /namespace LLGL


#endif



// ================================================================================
