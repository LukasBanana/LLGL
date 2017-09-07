/*
 * VKExtensionLoader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_EXTENSION_LOADER_H
#define LLGL_VK_EXTENSION_LOADER_H


#include "../Vulkan.h"


namespace LLGL
{


/* --- Common extension loading functions --- */

/**
Loads all available extensions and prints errors if an extension is available,
but their respective functions could not be loaded.
\param[in,out] extensions Specifies the extension map. This can be queried by the "QueryExtensions" function.
The respective entry will be set to true if all its functions have been loaded successfully.
\see QueryExtensions
*/
void LoadAllExtensions(VkInstance instance);

//! Returns true if all available extensions have been loaded.
bool AreExtensionsLoaded();


} // /namespace LLGL


#endif



// ================================================================================
