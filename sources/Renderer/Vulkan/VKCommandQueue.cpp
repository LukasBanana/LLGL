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
