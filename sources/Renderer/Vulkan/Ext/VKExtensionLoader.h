/*
 * VKExtensionLoader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
