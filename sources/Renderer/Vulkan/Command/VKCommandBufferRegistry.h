/*
 * VKCommandBufferRegistry.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COMMAND_BUFFER_REGISTRY_H
#define LLGL_VK_COMMAND_BUFFER_REGISTRY_H


#include <vulkan/vulkan.h>
#include <cstdint>


namespace LLGL
{


/*
Tracks the VkCommandBuffer handles LLGL allocates, so the Vulkan debug messenger can tell whether a validation
error originated from a command buffer LLGL recorded/submitted versus one owned by another component that shares
the same VkDevice (e.g. an OpenXR runtime compositor, which records its own barriers/draws on our device). The
registry is only populated while tracking is enabled, which the debug messenger turns on; outside debug usage it
is inert, so there is no overhead in release builds.
*/

// Enables or disables command-buffer tracking. The debug messenger enables it when it is installed.
void VKSetCommandBufferTrackingEnabled(bool enabled);

// Registers command buffers LLGL just allocated. No-op while tracking is disabled.
void VKRegisterCommandBuffers(std::uint32_t count, const VkCommandBuffer* commandBuffers);

// Unregisters command buffers LLGL is about to free.
void VKUnregisterCommandBuffers(std::uint32_t count, const VkCommandBuffer* commandBuffers);

// Returns true if the specified command buffer was allocated by LLGL while tracking was enabled.
bool VKIsLLGLCommandBuffer(VkCommandBuffer commandBuffer);


} // /namespace LLGL


#endif



// ================================================================================
