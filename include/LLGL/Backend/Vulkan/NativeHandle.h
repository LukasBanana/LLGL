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


namespace LLGL
{

namespace Vulkan
{


struct RenderSystemNativeHandle
{
    VkInstance          instance;
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;
    VkQueue             queue;
    std::uint32_t       queueGraphicsFamily;
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
