/*
 * VKExtensionLoader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKExtensionLoader.h"
#include "VKExtensions.h"
#include <LLGL/Log.h>
#include <functional>
#include <string>


namespace LLGL
{


/* --- Internal functions --- */

template <typename T>
bool LoadVKProc(VkInstance instance, T& procAddr, const char* procName)
{
    /* Load Vulkan procedure address */
    procAddr = reinterpret_cast<T>(vkGetInstanceProcAddr(instance, procName));
    
    /* Check for errors */
    if (!procAddr)
    {
        Log::StdErr() << "failed to load Vulkan procedure: " << procName << std::endl;
        return false;
    }
    
    return true;
}

/* --- Hardware buffer extensions --- */

#define LOAD_VKPROC(NAME)                   \
    if (!LoadVKProc(instance, NAME, #NAME)) \
        return false

#ifdef LLGL_OS_WIN32

static bool Load_VK_KHR_win32_surface(VkInstance instance)
{
    LOAD_VKPROC( vkCreateWin32SurfaceKHR );
    return true;
}

#endif // /LLGL_OS_WIN32

#undef LOAD_VKPROC


/* --- Common extension loading functions --- */

// Global member to store if the extension have already been loaded
static bool g_extAlreadyLoaded = false;

void LoadAllExtensions(VkInstance instance)
{
    /* Only load GL extensions once */
    if (g_extAlreadyLoaded)
        return;
    
    auto LoadExtension = [&](const std::string& extName, const std::function<bool(VkInstance)>& extLoadingProc) -> void
    {
        /* Try to load OpenGL extension */
        if (!extLoadingProc(instance))
        {
            /* Loading extension failed */
            Log::StdErr() << "failed to load Vulkan extension: " << extName << std::endl;
        }
    };

    #define LOAD_VKEXT(NAME) \
        LoadExtension("VK_" + std::string(#NAME), Load_VK_##NAME)
        
    /* Load platform specific extensions */
    #ifdef LLGL_OS_WIN32
    LOAD_VKEXT( KHR_win32_surface );
    #endif // /LLGL_OS_WIN32

    #undef LOAD_VKEXT
    
    g_extAlreadyLoaded = true;
}

bool AreExtensionsLoaded()
{
    return g_extAlreadyLoaded;
}


} // /namespace LLGL



// ================================================================================
