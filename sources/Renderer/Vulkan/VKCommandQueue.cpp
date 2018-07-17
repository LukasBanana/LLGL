/*
 * VKCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandQueue.h"
#include "VKCommandBuffer.h"
#include "RenderState/VKFence.h"
#include "../CheckedCast.h"


namespace LLGL
{


VKCommandQueue::VKCommandQueue(const VKPtr<VkDevice>& device, VkQueue graphicsQueue) :
    device_        { device        },
    graphicsQueue_ { graphicsQueue }
{
}

/* ----- Command Buffers ----- */

static VkCommandBufferUsageFlags GetVkCommandBufferUsageFlags(long flags)
{
    VkCommandBufferUsageFlags usageFlags = 0;

    if ((flags & RecordingFlags::OneTimeSubmit) != 0)
        usageFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    else
        usageFlags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    return usageFlags;
}

void VKCommandQueue::Begin(CommandBuffer& commandBuffer, long flags)
{
    auto& commandBufferVK = LLGL_CAST(VKCommandBuffer&, commandBuffer);

    /* Use next internal VkCommandBuffer object to reduce latency */
    commandBufferVK.AcquireNextBuffer();

    /* Wait for fence before recording */
    VkFence fence = commandBufferVK.GetQueueSubmitFence();
    vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &fence);

    /* Begin recording of current command buffer */
    VkCommandBufferBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.flags             = GetVkCommandBufferUsageFlags(flags);
        beginInfo.pInheritanceInfo  = nullptr;
    }
    auto result = vkBeginCommandBuffer(commandBufferVK.GetVkCommandBuffer(), &beginInfo);
    VKThrowIfFailed(result, "failed to begin Vulkan command buffer");
}

void VKCommandQueue::End(CommandBuffer& commandBuffer)
{
    auto& commandBufferVK = LLGL_CAST(VKCommandBuffer&, commandBuffer);

    /* End recording of current command buffer */
    auto result = vkEndCommandBuffer(commandBufferVK.GetVkCommandBuffer());
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");

    /* Immediately submit command buffer */
    Submit(commandBuffer);
}

void VKCommandQueue::Submit(CommandBuffer& commandBuffer)
{
    auto& commandBufferVK = LLGL_CAST(VKCommandBuffer&, commandBuffer);

    VkCommandBuffer commandBuffers[] = { commandBufferVK.GetVkCommandBuffer() };

    /* Submit command buffer to graphics queue */
    VkSubmitInfo submitInfo;
    {
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.waitSemaphoreCount   = 0;
        submitInfo.pWaitSemaphores      = nullptr;
        submitInfo.pWaitDstStageMask    = 0;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = commandBuffers;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores    = nullptr;
    }
    auto result = vkQueueSubmit(graphicsQueue_, 1, &submitInfo, commandBufferVK.GetQueueSubmitFence());
    VKThrowIfFailed(result, "failed to submit command buffer to Vulkan graphics queue");
}

/* ----- Fences ----- */

void VKCommandQueue::Submit(Fence& fence)
{
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    fenceVK.Reset(device_);
    vkQueueSubmit(graphicsQueue_, 0, nullptr, fenceVK.GetVkFence());
}

bool VKCommandQueue::WaitFence(Fence& fence, std::uint64_t timeout)
{
    auto& fenceVK = LLGL_CAST(VKFence&, fence);
    return fenceVK.Wait(device_, timeout);
}

void VKCommandQueue::WaitIdle()
{
    vkQueueWaitIdle(graphicsQueue_);
}


} // /namespace LLGL



// ================================================================================
