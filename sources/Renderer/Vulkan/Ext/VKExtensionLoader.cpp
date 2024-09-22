/*
 * VKExtensionLoader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKExtensionLoader.h"
#include "VKExtensions.h"
#include "VKExtensionRegistry.h"
#include "../../../Core/Exception.h"
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
    return (procAddr != nullptr);
}

template <typename T>
bool LoadVKProc(VkDevice device, T& procAddr, const char* procName)
{
    /* Load Vulkan procedure address */
    procAddr = reinterpret_cast<T>(vkGetDeviceProcAddr(device, procName));
    return (procAddr != nullptr);
}

/* --- Hardware buffer extensions --- */

using LoadVKExtensionInstanceProc   = std::function<bool(VkInstance instance, const char* extName, bool abortOnFailure)>;
using LoadVKExtensionDeviceProc     = std::function<bool(VkDevice   device  , const char* extName, bool abortOnFailure)>;

#define DECL_LOADVKEXT_PROC_BASE(EXTNAME, HNDL) \
    Load_VK_ ## EXTNAME(HNDL handle, const char* extName, bool abortOnFailure)

#define DECL_LOADVKEXT_PROC_INSTANCE(EXTNAME) \
    DECL_LOADVKEXT_PROC_BASE(EXTNAME, VkInstance)

#define DECL_LOADVKEXT_PROC(EXTNAME) \
    DECL_LOADVKEXT_PROC_BASE(EXTNAME, VkDevice)

#define LOAD_VKPROC(NAME)                                                           \
    if (!LoadVKProc(handle, NAME, #NAME))                                           \
    {                                                                               \
        if (abortOnFailure)                                                         \
            LLGL_TRAP("failed to load Vulkan procedure: %s [%s]", #NAME, extName);  \
        return false;                                                               \
    }

#if defined LLGL_OS_WIN32

static bool DECL_LOADVKEXT_PROC_INSTANCE(KHR_win32_surface)
{
    LOAD_VKPROC( vkCreateWin32SurfaceKHR );
    return true;
}

#elif defined LLGL_OS_LINUX

static bool DECL_LOADVKEXT_PROC_INSTANCE(KHR_xlib_surface)
{
    LOAD_VKPROC( vkCreateXlibSurfaceKHR );
    return true;
}

#elif defined LLGL_OS_ANDROID

static bool DECL_LOADVKEXT_PROC_INSTANCE(KHR_android_surface)
{
    LOAD_VKPROC( vkCreateAndroidSurfaceKHR );
    return true;
}

#endif // /LLGL_OS_WIN32

static bool DECL_LOADVKEXT_PROC(EXT_debug_marker)
{
    LOAD_VKPROC( vkDebugMarkerSetObjectTagEXT  );
    LOAD_VKPROC( vkDebugMarkerSetObjectNameEXT );
    LOAD_VKPROC( vkCmdDebugMarkerBeginEXT      );
    LOAD_VKPROC( vkCmdDebugMarkerEndEXT        );
    LOAD_VKPROC( vkCmdDebugMarkerInsertEXT     );
    return true;
}

static bool DECL_LOADVKEXT_PROC(EXT_conditional_rendering)
{
    LOAD_VKPROC( vkCmdBeginConditionalRenderingEXT );
    LOAD_VKPROC( vkCmdEndConditionalRenderingEXT   );
    return true;
}

static bool DECL_LOADVKEXT_PROC(EXT_transform_feedback)
{
    LOAD_VKPROC( vkCmdBindTransformFeedbackBuffersEXT );
    LOAD_VKPROC( vkCmdBeginTransformFeedbackEXT       );
    LOAD_VKPROC( vkCmdEndTransformFeedbackEXT         );
    LOAD_VKPROC( vkCmdBeginQueryIndexedEXT            );
    LOAD_VKPROC( vkCmdEndQueryIndexedEXT              );
    LOAD_VKPROC( vkCmdDrawIndirectByteCountEXT        );
    return true;
}

static bool DECL_LOADVKEXT_PROC(KHR_get_physical_device_properties2)
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

#undef DECL_LOADVKEXT_PROC_BASE
#undef DECL_LOADVKEXT_PROC_INSTANCE
#undef DECL_LOADVKEXT_PROC
#undef LOAD_VKPROC


/* --- Common extension loading functions --- */

bool VKLoadInstanceExtensions(VkInstance instance)
{
    constexpr bool abortOnFailure = true;

    auto LoadExtension = [instance, abortOnFailure](const char* extName, const LoadVKExtensionInstanceProc& extLoadingProc) -> void
    {
        /* Try to load Vulkan extension */
        extLoadingProc(instance, extName, abortOnFailure);
    };

    #define LOAD_VKEXT(NAME) \
        LoadExtension("VK_" #NAME, Load_VK_##NAME)

    /* Load platform specific extensions */
    #if defined LLGL_OS_WIN32
    LOAD_VKEXT( KHR_win32_surface );
    #elif defined LLGL_OS_LINUX
    LOAD_VKEXT( KHR_xlib_surface );
    #elif defined LLGL_OS_ANDROID
    LOAD_VKEXT( KHR_android_surface );
    #endif // /LLGL_OS_WIN32

    #undef LOAD_VKEXT

    return true;
}

bool VKLoadDeviceExtensions(VkDevice device, const std::vector<const char*>& supportedExtensions)
{
    constexpr bool abortOnFailure = true;

    auto IsSupported = [&supportedExtensions](const char* extName) -> bool
    {
        for (auto extension : supportedExtensions)
        {
            if (std::strcmp(extName, extension) == 0)
                return true;
        }
        return false;
    };

    auto LoadExtension = [device, &IsSupported, abortOnFailure](const VKExt extensionID, const char* extName, const LoadVKExtensionDeviceProc& extLoadingProc) -> void
    {
        /* Check if extensions is included in the list of supported extension names */
        if (IsSupported(extName))
        {
            /* Try to load Vulkan extension */
            if (extLoadingProc(device, extName, abortOnFailure))
                RegisterExtension(extensionID);
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
    ENABLE_VKEXT( EXT_nested_command_buffer      );

    #undef LOAD_VKEXT

    return true;
}


} // /namespace LLGL



// ================================================================================
