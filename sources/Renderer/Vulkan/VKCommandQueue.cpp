/*
 * VKCommandQueue.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

bool VKCommandQueue::QueryResult(
    QueryHeap&      queryHeap,
    std::uint32_t   firstQuery,
    std::uint32_t   numQueries,
    void*           data,
    std::size_t     dataSize)
{
    auto& queryHeapVK = LLGL_CAST(VKQueryHeap&, queryHeap);

    /* Determine flags and stride */
    VkQueryResultFlags  resultFlags = 0;
    VkDeviceSize        stride      = sizeof(std::uint32_t);

    if (dataSize != numQueries * sizeof(std::uint32_t))
    {
        resultFlags |= VK_QUERY_RESULT_64_BIT;
        stride      = sizeof(std::uint64_t);
    }

    /* Store result directly into output parameter */
    auto stateResult = vkGetQueryPoolResults(
        device_,
        queryHeapVK.GetVkQueryPool(),
        firstQuery,
        numQueries,
        dataSize,
        data,
        stride,
        resultFlags
    );

    /* Check if result is not ready yet */
    if (stateResult == VK_NOT_READY)
        return false;

    VKThrowIfFailed(stateResult, "failed to retrieve results from Vulkan query pool");

    return true;
}

#if 0
bool VKCommandBuffer::QueryPipelineStatisticsResult(QueryHeap& queryHeap, QueryPipelineStatistics& result)
{

    /* Copy result to output parameter */
    result.inputAssemblyVertices            = intermediateResults[ 0]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
    result.inputAssemblyPrimitives          = intermediateResults[ 1]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
    result.vertexShaderInvocations          = intermediateResults[ 2]; // VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
    result.tessControlShaderInvocations     = intermediateResults[ 8]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
    result.tessEvaluationShaderInvocations  = intermediateResults[ 9]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
    result.geometryShaderInvocations        = intermediateResults[ 3]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
    result.fragmentShaderInvocations        = intermediateResults[ 7]; // VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
    result.computeShaderInvocations         = intermediateResults[10]; // VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
    result.geometryShaderPrimitives         = intermediateResults[ 4]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
    result.clippingInvocations              = intermediateResults[ 5]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
    result.clippingPrimitives               = intermediateResults[ 6]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT

    return true;
}
#endif

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
