/*
 * VKRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderContext.h"
#include "VKCore.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/* ----- Common ----- */

VKRenderContext::VKRenderContext(const VKPtr<VkInstance>& instance, RenderContextDescriptor desc, const std::shared_ptr<Surface>& surface) :
    instance_ { instance                      },
    surface_  { instance, vkDestroySurfaceKHR }
{
    SetOrCreateSurface(surface, desc.videoMode, nullptr);
    CreateVkSurface();
}

void VKRenderContext::Present()
{
    //todo
}

/* ----- Configuration ----- */

void VKRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    //todo
}

void VKRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    //todo
}


/*
 * ======= Private: =======
 */

void VKRenderContext::CreateVkSurface()
{
    /* Get hantive handle from context surface */
    NativeHandle nativeHandle;
    GetSurface().GetNativeHandle(&nativeHandle);

    #if defined LLGL_OS_WIN32

    /* Setup Win32 surface descriptor */
    VkWin32SurfaceCreateInfoKHR surfaceDesc;
    {
        surfaceDesc.sType       = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceDesc.pNext       = nullptr;
        surfaceDesc.flags       = 0;
        surfaceDesc.hinstance   = GetModuleHandle(NULL);
        surfaceDesc.hwnd        = nativeHandle.window;
    }
    VkResult result = vkCreateWin32SurfaceKHR(instance_, &surfaceDesc, nullptr, surface_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Win32 surface for Vulkan render context");

    #endif
}

std::vector<VkQueueFamilyProperties> VKRenderContext::QueryQueueFamilyProperties(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

QueueFamilyIndices VKRenderContext::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, const VkQueueFlags flags)
{
    QueueFamilyIndices indices;

    auto queueFamilies = QueryQueueFamilyProperties(device);

    int i = 0;
    for (const auto& family : queueFamilies)
    {
        /* Get graphics family index */
        if (family.queueCount > 0 && (family.queueFlags & flags) != 0)
            indices.graphicsFamily = i;

        /* Get present family index */
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (family.queueCount > 0 && presentSupport)
            indices.presentFamily = i;

        /* Check if queue family is complete */
        if (indices.Complete())
            break;

        ++i;
    }

    return indices;
}


} // /namespace LLGL



// ================================================================================
