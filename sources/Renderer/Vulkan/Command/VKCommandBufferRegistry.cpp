/*
 * VKCommandBufferRegistry.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKCommandBufferRegistry.h"
#include <mutex>
#include <unordered_set>
#include <atomic>

namespace LLGL
{


static std::mutex                           g_commandBufferRegistryMutex;
static std::unordered_set<VkCommandBuffer>  g_commandBufferRegistry;
static std::atomic_bool                     g_commandBufferTrackingEnabled(false);

void VKSetCommandBufferTrackingEnabled(bool enabled)
{
    g_commandBufferTrackingEnabled = enabled;
    if (!enabled)
    {
        std::lock_guard<std::mutex> guard{g_commandBufferRegistryMutex};
        if (!g_commandBufferTrackingEnabled)
            g_commandBufferRegistry.clear();
    }
}

void VKRegisterCommandBuffers(std::uint32_t count, const VkCommandBuffer* commandBuffers)
{
    if (commandBuffers == nullptr)
        return;

    if (!g_commandBufferTrackingEnabled)
        return;

    std::lock_guard<std::mutex> guard{g_commandBufferRegistryMutex};
    if (!g_commandBufferTrackingEnabled)
        return;

    for (std::uint32_t i = 0; i < count; ++i)
    {
        if (commandBuffers[i] != VK_NULL_HANDLE)
            g_commandBufferRegistry.insert(commandBuffers[i]);
    }
}

void VKUnregisterCommandBuffers(std::uint32_t count, const VkCommandBuffer* commandBuffers)
{
    if (commandBuffers == nullptr)
        return;

    if (!g_commandBufferTrackingEnabled)
        return;

    std::lock_guard<std::mutex> guard{ g_commandBufferRegistryMutex };
    if (!g_commandBufferTrackingEnabled)
        return;

    for (std::uint32_t i = 0; i < count; ++i)
        g_commandBufferRegistry.erase(commandBuffers[i]);
}

bool VKIsLLGLCommandBuffer(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE)
        return false;

    if (!g_commandBufferTrackingEnabled)
        return false;

    std::lock_guard<std::mutex> guard{ g_commandBufferRegistryMutex };
    return (g_commandBufferRegistry.find(commandBuffer) != g_commandBufferRegistry.end());
}


} // /namespace LLGL



// ================================================================================
