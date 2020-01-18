/*
 * VKExtensionLoader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKExtensionLoader.h"
#include "VKExtensions.h"
#include "VKExtensionRegistry.h"
#include <LLGL/Log.h>
#include <functional>
#include <cstring>


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
        Log::PostReport(Log::ReportType::Error, "failed to load Vulkan instance procedure: " + std::string(procName));
        return false;
    }

    return true;
}

template <typename T>
bool LoadVKProc(VkDevice device, T& procAddr, const char* procName)
{
    /* Load Vulkan procedure address */
    procAddr = reinterpret_cast<T>(vkGetDeviceProcAddr(device, procName));

    /* Check for errors */
    if (!procAddr)
    {
        Log::PostReport(Log::ReportType::Error, "failed to load Vulkan device procedure: " + std::string(procName));
        return false;
    }

    return true;
}

/* --- Hardware buffer extensions --- */

#define LOAD_VKPROC(NAME)                   \
    if (!LoadVKProc(handle, NAME, #NAME))   \
        return false

#ifdef LLGL_OS_WIN32

static bool Load_VK_KHR_win32_surface(VkInstance handle)
{
    LOAD_VKPROC( vkCreateWin32SurfaceKHR );
    return true;
}

#endif // /LLGL_OS_WIN32

static bool Load_VK_EXT_debug_marker(VkDevice handle)
{
    LOAD_VKPROC( vkDebugMarkerSetObjectTagEXT  );
    LOAD_VKPROC( vkDebugMarkerSetObjectNameEXT );
    LOAD_VKPROC( vkCmdDebugMarkerBeginEXT      );
    LOAD_VKPROC( vkCmdDebugMarkerEndEXT        );
    LOAD_VKPROC( vkCmdDebugMarkerInsertEXT     );
    return true;
}

static bool Load_VK_EXT_conditional_rendering(VkDevice handle)
{
    LOAD_VKPROC( vkCmdBeginConditionalRenderingEXT );
    LOAD_VKPROC( vkCmdEndConditionalRenderingEXT   );
    return true;
}

static bool Load_VK_EXT_transform_feedback(VkDevice handle)
{
    LOAD_VKPROC( vkCmdBindTransformFeedbackBuffersEXT );
    LOAD_VKPROC( vkCmdBeginTransformFeedbackEXT       );
    LOAD_VKPROC( vkCmdEndTransformFeedbackEXT         );
    LOAD_VKPROC( vkCmdBeginQueryIndexedEXT            );
    LOAD_VKPROC( vkCmdEndQueryIndexedEXT              );
    LOAD_VKPROC( vkCmdDrawIndirectByteCountEXT        );
    return true;
}

static bool Load_VK_KHR_get_physical_device_properties2(VkDevice handle)
{
    LOAD_VKPROC( vkGetPhysicalDeviceFeatures2KHR                    );
    LOAD_VKPROC( vkGetPhysicalDeviceProperties2KHR                  );
    LOAD_VKPROC( vkGetPhysicalDeviceFormatProperties2KHR            );
    LOAD_VKPROC( vkGetPhysicalDeviceImageFormatProperties2KHR       );
    LOAD_VKPROC( vkGetPhysicalDeviceQueueFamilyProperties2KHR       );
    LOAD_VKPROC( vkGetPhysicalDeviceMemoryProperties2KHR            );
    LOAD_VKPROC( vkGetPhysicalDeviceSparseImageFormatProperties2KHR );
    return true;
}

#undef LOAD_VKPROC


/* --- Common extension loading functions --- */

bool VKLoadInstanceExtensions(VkInstance instance)
{
    auto LoadExtension = [&](const std::string& extName, const std::function<bool(VkInstance)>& extLoadingProc) -> void
    {
        /* Try to load Vulkan extension */
        if (!extLoadingProc(instance))
        {
            /* Loading extension failed */
            Log::PostReport(Log::ReportType::Error, "failed to load Vulkan extension: " + extName);
        }
    };

    #define LOAD_VKEXT(NAME) \
        LoadExtension("VK_" + std::string(#NAME), Load_VK_##NAME)

    /* Load platform specific extensions */
    #ifdef LLGL_OS_WIN32
    LOAD_VKEXT( KHR_win32_surface );
    #endif // /LLGL_OS_WIN32

    #undef LOAD_VKEXT

    return true;
}

bool VKLoadDeviceExtensions(VkDevice device, const std::vector<const char*>& supportedExtensions)
{
    auto IsSupported = [&supportedExtensions](const char* extName) -> bool
    {
        for (auto extension : supportedExtensions)
        {
            if (std::strcmp(extName, extension) == 0)
                return true;
        }
        return false;
    };

    auto LoadExtension = [&](const VKExt extensionID, const char* extName, const std::function<bool(VkDevice)>& extLoadingProc) -> void
    {
        /* Check if extensions is included in the list of supported extension names */
        if (IsSupported(extName))
        {
            /* Try to load Vulkan extension */
            if (extLoadingProc(device))
                RegisterExtension(extensionID);
            else
                Log::PostReport(Log::ReportType::Error, "failed to load Vulkan extension: " + std::string(extName));
        }
    };

    auto EnableExtension = [&](const VKExt extensionID, const char* extName)
    {
        if (IsSupported(extName))
            RegisterExtension(extensionID);
    };

    #define LOAD_VKEXT(NAME) \
        LoadExtension(VKExt::NAME, "VK_" #NAME, Load_VK_##NAME)

    #define ENABLE_VKEXT(NAME) \
        EnableExtension(VKExt::NAME, "VK_" #NAME)

    /* Multi-vendor extensions */
    LOAD_VKEXT( KHR_get_physical_device_properties2 );
    LOAD_VKEXT( EXT_debug_marker                    );
    LOAD_VKEXT( EXT_conditional_rendering           );
    LOAD_VKEXT( EXT_transform_feedback              );

    ENABLE_VKEXT( EXT_conservative_rasterization );

    #undef LOAD_VKEXT

    return true;
}


} // /namespace LLGL



// ================================================================================
