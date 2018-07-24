/*
 * VKCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandQueue.h"
#include "VKCommandBuffer.h"
#include "RenderState/VKFence.h"
#include "RenderState/VKQueryHeap.h"
#include "../CheckedCast.h"
#include "VKCore.h"


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

/* ----- Queries ----- */

bool VKCommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize)
{
    auto& queryHeapVK = LLGL_CAST(VKQueryHeap&, queryHeap);

    /* Determine flags */
    VkQueryResultFlags resultFlags = 0;

    if (dataSize != numQueries * sizeof(std::uint32_t))
        resultFlags |= VK_QUERY_RESULT_64_BIT;

    /* Store result directly into output parameter */
    auto stateResult = vkGetQueryPoolResults(
        device_,
        queryHeapVK.GetVkQueryPool(),
        firstQuery,
        numQueries,
        dataSize,
        data,
        sizeof(std::uint64_t),
        resultFlags
    );

    /* Check if result is not ready yet */
    if (stateResult == VK_NOT_READY)
        return false;

    VKThrowIfFailed(stateResult, "failed to retrieve results from Vulkan query pool");
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
