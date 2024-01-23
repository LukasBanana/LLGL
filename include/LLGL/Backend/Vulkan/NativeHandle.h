/*
 * NativeHandle.h (Vulkan)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VULKAN_NATIVE_HANDLE_H
#define LLGL_VULKAN_NATIVE_HANDLE_H


#include <cstdint>
#include <vulkan/vulkan.h>
#include <LLGL/Deprecated.h>


namespace LLGL
{

namespace Vulkan
{


/**
\brief Native handle structure for the Vulkan render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    //! Native handle to the Vulkan instance.
    VkInstance          instance;

    //! Native handle to the physical Vulkan device.
    VkPhysicalDevice    physicalDevice;

    //! Native handle to the logical Vulkan device.
    VkDevice            device;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queue is deprecated since 0.04b; Use vkGetDeviceQueue on the logical device instead!")
    VkQueue             queue;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queueGraphicsFamily is deprecated since 0.04b; Use vkGetPhysicalDeviceQueueFamilyProperties on the physical device instead!.")
    std::uint32_t       queueGraphicsFamily;

    LLGL_DEPRECATED("RenderSystemNativeHandle::queuePresentFamily is deprecated since 0.04b; Use vkGetPhysicalDeviceQueueFamilyProperties on the physical device instead!.")
    std::uint32_t       queuePresentFamily;
};

struct CommandBufferNativeHandle
{
    VkCommandBuffer commandBuffer;
};


} // /namespace Vulkan

} // /namespace LLGL


#endif



// ================================================================================
