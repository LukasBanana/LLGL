/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKRenderContext.h"
#include "VKTypes.h"
#include "RenderState/VKRenderPass.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "RenderState/VKComputePipeline.h"
#include "RenderState/VKResourceHeap.h"
#include "RenderState/VKQuery.h"
#include "Texture/VKSampler.h"
#include "Texture/VKRenderTarget.h"
#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"
#include "Buffer/VKIndexBuffer.h"
#include "../CheckedCast.h"
#include "../StaticLimits.h"
#include <cstddef>


namespace LLGL
{


static const std::uint32_t g_maxNumViewportsPerBatch = 16;

VKCommandBuffer::VKCommandBuffer(
    const VKPtr<VkDevice>&          device,
    VkQueue                         graphicsQueue,
    const QueueFamilyIndices&       queueFamilyIndices,
    const CommandBufferDescriptor&  desc) :
        device_             { device                           },
        commandPool_        { device, vkDestroyCommandPool     },
        queuePresentFamily_ { queueFamilyIndices.presentFamily }
{
    std::size_t bufferCount = std::max(1u, desc.numNativeBuffers);

    /* Translate creation flags */
    if ((desc.flags & CommandBufferFlags::DeferredSubmit) != 0)
        usageFlags_ = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    /* Create native command buffer objects */
    CreateCommandPool(queueFamilyIndices.graphicsFamily);
    CreateCommandBuffers(bufferCount);
    CreateRecordingFences(graphicsQueue, bufferCount);

    /* Acquire first native command buffer */
    AcquireNextBuffer();
}

VKCommandBuffer::~VKCommandBuffer()
{
    vkFreeCommandBuffers(
        device_,
        commandPool_,
        static_cast<std::uint32_t>(commandBufferList_.size()),
        commandBufferList_.data()
    );
}

/* ----- Encoding ----- */

void VKCommandBuffer::Begin()
{
    /* Use next internal VkCommandBuffer object to reduce latency */
    AcquireNextBuffer();

    /* Wait for fence before recording */
    vkWaitForFences(device_, 1, &recordingFence_, VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &recordingFence_);

    /* Begin recording of current command buffer */
    VkCommandBufferBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.flags             = usageFlags_;
        beginInfo.pInheritanceInfo  = nullptr;
    }
    auto result = vkBeginCommandBuffer(commandBuffer_, &beginInfo);
    VKThrowIfFailed(result, "failed to begin Vulkan command buffer");
}

void VKCommandBuffer::End()
{
    /* End encoding of current command buffer */
    auto result = vkEndCommandBuffer(commandBuffer_);
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");
}

void VKCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);

    auto size   = static_cast<VkDeviceSize>(dataSize);
    auto offset = static_cast<VkDeviceSize>(dstOffset);

    vkCmdUpdateBuffer(commandBuffer_, dstBufferVK.GetVkBuffer(), offset, size, data);
}

void VKCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferVK = LLGL_CAST(VKBuffer&, dstBuffer);
    auto& srcBufferVK = LLGL_CAST(VKBuffer&, srcBuffer);

    VkBufferCopy region;
    {
        region.srcOffset    = static_cast<VkDeviceSize>(srcOffset);
        region.dstOffset    = static_cast<VkDeviceSize>(dstOffset);
        region.size         = static_cast<VkDeviceSize>(size);
    }
    vkCmdCopyBuffer(commandBuffer_, srcBufferVK.GetVkBuffer(), dstBufferVK.GetVkBuffer(), 1, &region);
}

/* ----- Configuration ----- */

void VKCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

#if 0//TODO: currently unused
// Check if VkViewport and Viewport structures can be safely reinterpret-casted
static bool IsCompatibleToVkViewport()
{
    return
    (
        sizeof(VkViewport)             == sizeof(Viewport)             &&
        offsetof(VkViewport, x       ) == offsetof(Viewport, x       ) &&
        offsetof(VkViewport, y       ) == offsetof(Viewport, y       ) &&
        offsetof(VkViewport, width   ) == offsetof(Viewport, width   ) &&
        offsetof(VkViewport, height  ) == offsetof(Viewport, height  ) &&
        offsetof(VkViewport, minDepth) == offsetof(Viewport, minDepth) &&
        offsetof(VkViewport, maxDepth) == offsetof(Viewport, maxDepth)
    );
}
#endif

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    #if 0
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport type */
        vkCmdSetViewport(commandBuffer_, 0, 1, reinterpret_cast<const VkViewport*>(&viewport));
    }
    else
    #endif
    {
        /* Convert viewport to VkViewport type */
        VkViewport viewportVK;
        VKTypes::Convert(viewportVK, viewport);
        vkCmdSetViewport(commandBuffer_, 0, 1, &viewportVK);
    }
}

void VKCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    #if 0
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport types */
        vkCmdSetViewport(commandBuffer_, 0, numViewports, reinterpret_cast<const VkViewport*>(viewports));
    }
    else
    #endif
    {
        VkViewport viewportsVK[g_maxNumViewportsPerBatch];

        for (std::uint32_t i = 0, first = 0, count = 0; i < numViewports; numViewports -= count)
        {
            /* Convert viewport to VkViewport types */
            count = std::min(numViewports, g_maxNumViewportsPerBatch);

            for (first = i; i < first + count; ++i)
                VKTypes::Convert(viewportsVK[i - first], viewports[i]);

            vkCmdSetViewport(commandBuffer_, first, count, viewportsVK);
        }
    }
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    if (scissorEnabled_)
    {
        VkRect2D scissorVK;
        VKTypes::Convert(scissorVK, scissor);
        vkCmdSetScissor(commandBuffer_, 0, 1, &scissorVK);
    }
}

void VKCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (scissorEnabled_)
    {
        VkRect2D scissorsVK[g_maxNumViewportsPerBatch];

        for (std::uint32_t i = 0, first = 0, count = 0; i < numScissors; numScissors -= count)
        {
            /* Convert viewport to VkViewport types */
            count = std::min(numScissors, g_maxNumViewportsPerBatch);

            for (first = i; i < first + count; ++i)
                VKTypes::Convert(scissorsVK[i - first], scissors[i]);

            vkCmdSetScissor(commandBuffer_, first, count, scissorsVK);
        }
    }
}

/* ----- Clear ----- */

static void Convert(VkClearColorValue& dst, const ColorRGBAf& src)
{
    dst.float32[0] = src.r;
    dst.float32[1] = src.g;
    dst.float32[2] = src.b;
    dst.float32[3] = src.a;
}

void VKCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    Convert(clearColor_, color);
}

void VKCommandBuffer::SetClearDepth(float depth)
{
    clearDepthStencil_.depth = depth;
}

void VKCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearDepthStencil_.stencil = stencil;
}

static VkImageAspectFlags GetDepthStencilAspectMask(long flags)
{
    VkImageAspectFlags aspectMask = 0;

    if ((flags & ClearFlags::Depth) != 0)
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if ((flags & ClearFlags::Stencil) != 0)
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

    return aspectMask;
}

void VKCommandBuffer::Clear(long flags)
{
    VkClearAttachment attachments[LLGL_MAX_NUM_ATTACHMENTS];

    std::uint32_t numAttachments = 0;

    /* Fill clear descriptors for color attachments */
    if ((flags & ClearFlags::Color) != 0)
    {
        numAttachments = std::min(numColorAttachments_, LLGL_MAX_NUM_COLOR_ATTACHMENTS);
        for (std::uint32_t i = 0; i < numAttachments; ++i)
        {
            auto& attachment = attachments[i];
            {
                attachment.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
                attachment.colorAttachment  = i;
                attachment.clearValue.color = clearColor_;
            }
        }
    }

    /* Fill clear descriptor for depth-stencil attachment */
    if ((flags & ClearFlags::DepthStencil) != 0 && hasDSVAttachment_)
    {
        auto& attachment = attachments[numAttachments++];
        {
            attachment.aspectMask               = GetDepthStencilAspectMask(flags);
            attachment.colorAttachment          = 0; // ignored
            attachment.clearValue.depthStencil  = clearDepthStencil_;
        }
    }

    /* Clear all framebuffer attachments */
    ClearFramebufferAttachments(numAttachments, attachments);
}

void VKCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    /* Convert clear attachment descriptors */
    VkClearAttachment attachmentsVK[LLGL_MAX_NUM_ATTACHMENTS];

    std::uint32_t numAttachmentsVK = 0;

    for (std::uint32_t i = 0, n = std::min(numAttachments, LLGL_MAX_NUM_ATTACHMENTS); i < n; ++i)
    {
        auto& dst = attachmentsVK[numAttachmentsVK];
        const auto& src = attachments[i];

        if ((src.flags & ClearFlags::Color) != 0)
        {
            /* Convert color clear command */
            dst.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            dst.colorAttachment = src.colorAttachment;
            Convert(dst.clearValue.color, src.clearValue.color);
            ++numAttachmentsVK;
        }
        else if (hasDSVAttachment_)
        {
            /* Convert depth-stencil clear command */
            dst.aspectMask      = 0;
            dst.colorAttachment = 0;

            if ((src.flags & ClearFlags::Depth) != 0)
            {
                dst.aspectMask                      |= VK_IMAGE_ASPECT_DEPTH_BIT;
                dst.clearValue.depthStencil.depth   = src.clearValue.depth;
            }
            if ((src.flags & ClearFlags::Stencil) != 0)
            {
                dst.aspectMask                      |= VK_IMAGE_ASPECT_STENCIL_BIT;
                dst.clearValue.depthStencil.stencil = src.clearValue.stencil;
            }

            ++numAttachmentsVK;
        }
    }

    ClearFramebufferAttachments(numAttachmentsVK, attachmentsVK);
}

/* ----- Input Assembly ------ */

void VKCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    VkBuffer buffers[] = { bufferVK.GetVkBuffer() };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer_, 0, 1, buffers, offsets);
}

void VKCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayVK = LLGL_CAST(VKBufferArray&, bufferArray);
    vkCmdBindVertexBuffers(
        commandBuffer_,
        0,
        static_cast<std::uint32_t>(bufferArrayVK.GetBuffers().size()),
        bufferArrayVK.GetBuffers().data(),
        bufferArrayVK.GetOffsets().data()
    );
}

void VKCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& indexBufferVK = LLGL_CAST(VKIndexBuffer&, buffer);
    vkCmdBindIndexBuffer(commandBuffer_, indexBufferVK.GetVkBuffer(), 0, indexBufferVK.GetIndexType());
}

/* ----- Stream Output Buffers ------ */

void VKCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    //todo
}

void VKCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    //todo
}

void VKCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    //todo
}

void VKCommandBuffer::EndStreamOutput()
{
    //todo
}

/* ----- Resource Heaps ----- */

//private
void VKCommandBuffer::BindResourceHeap(VKResourceHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t firstSet)
{
    vkCmdBindDescriptorSets(
        commandBuffer_,
        bindingPoint,
        resourceHeapVK.GetVkPipelineLayout(),
        firstSet,
        static_cast<std::uint32_t>(resourceHeapVK.GetVkDescriptorSets().size()),
        resourceHeapVK.GetVkDescriptorSets().data(),
        0,
        nullptr
    );
}

void VKCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceHeap&, resourceHeap);
    BindResourceHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_GRAPHICS, firstSet);
}

void VKCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceHeap&, resourceHeap);
    BindResourceHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_COMPUTE, firstSet);
}

/* ----- Render Passes ----- */

void VKCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    if (renderTarget.IsRenderContext())
    {
        /* Get Vulkan render context object */
        auto& renderContextVK = LLGL_CAST(VKRenderContext&, renderTarget);

        /* Store information about framebuffer attachments */
        renderPass_             = renderContextVK.GetSwapChainRenderPass().GetVkRenderPass();
        framebuffer_            = renderContextVK.GetVkFramebuffer();
        framebufferExtent_      = renderContextVK.GetVkExtent();
        numColorAttachments_    = renderContextVK.GetNumColorAttachments();
        hasDSVAttachment_       = (renderContextVK.HasDepthAttachment() || renderContextVK.HasStencilAttachment());
    }
    else
    {
        /* Get Vulkan render target object and store its extent for subsequent commands */
        auto& renderTargetVK = LLGL_CAST(VKRenderTarget&, renderTarget);

        /* Store information about framebuffer attachments */
        renderPass_             = renderTargetVK.GetVkRenderPass();
        framebuffer_            = renderTargetVK.GetVkFramebuffer();
        framebufferExtent_      = renderTargetVK.GetVkExtent();
        numColorAttachments_    = renderTargetVK.GetNumColorAttachments();
        hasDSVAttachment_       = (renderTargetVK.HasDepthAttachment() || renderTargetVK.HasStencilAttachment());
    }

    scissorRectInvalidated_ = true;

    /* Declare array or clear values */
    VkClearValue clearValuesVK[LLGL_MAX_NUM_ATTACHMENTS];
    std::uint32_t numClearValuesVK = 0;

    /* Get native render pass object either from RenderTarget or RenderPass interface */
    if (renderPass != nullptr)
    {
        /* Get native VkRenderPass object */
        auto renderPassVK = LLGL_CAST(const VKRenderPass*, renderPass);
        renderPass_ = renderPassVK->GetVkRenderPass();

        /* Fill array of clear values */
        numClearValuesVK        = renderPassVK->GetNumClearValues();
        auto clearValuesMask    = renderPassVK->GetClearValuesMask();
        auto depthStencilIndex  = renderPassVK->GetDepthStencilIndex();

        for (std::uint32_t i = 0; i < numClearValuesVK; ++i)
        {
            /* Check if current attachment index requires a clear value */
            if (((clearValuesMask >> i) & 0x1ull) != 0)
            {
                auto& dst = clearValuesVK[i];

                if (numClearValues > 0)
                {
                    /* Set specified clear parameter */
                    if (i == depthStencilIndex)
                    {
                        dst.depthStencil.depth      = clearValues->depth;
                        dst.depthStencil.stencil    = clearValues->stencil;
                    }
                    else
                        Convert(dst.color, clearValues->color);

                    /* Get next clear value parameter */
                    --numClearValues;
                    ++clearValues;
                }
                else
                {
                    /* Set global clear parameters (from SetClearColor, SetClearDepth, SetClearStencil) */
                    if (i == depthStencilIndex)
                        dst.depthStencil = clearDepthStencil_;
                    else
                        dst.color = clearColor_;
                }
            }
        }
    }

    /* Record begin of render pass */
    VkRenderPassBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.renderPass        = renderPass_;
        beginInfo.framebuffer       = framebuffer_;
        beginInfo.renderArea.offset = { 0, 0 };
        beginInfo.renderArea.extent = framebufferExtent_;
        beginInfo.clearValueCount   = numClearValuesVK;
        beginInfo.pClearValues      = clearValuesVK;
    }
    vkCmdBeginRenderPass(commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKCommandBuffer::EndRenderPass()
{
    /* Record and of render pass */
    vkCmdEndRenderPass(commandBuffer_);

    /* Reset render pass and framebuffer attributes */
    renderPass_     = VK_NULL_HANDLE;
    framebuffer_    = VK_NULL_HANDLE;
}


/* ----- Pipeline States ----- */

void VKCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineVK = LLGL_CAST(VKGraphicsPipeline&, graphicsPipeline);

    /* Bind graphics pipeline */
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineVK.GetVkPipeline());

    /* Scissor rectangle must be updated (if scissor test is disabled) */
    scissorEnabled_ = graphicsPipelineVK.IsScissorEnabled();
    if (!scissorEnabled_ && scissorRectInvalidated_ && graphicsPipelineVK.HasDynamicScissor())
    {
        /* Set scissor to render target resolution */
        VkRect2D scissorRect;
        {
            scissorRect.offset = { 0, 0 };
            scissorRect.extent = framebufferExtent_;
        }
        vkCmdSetScissor(commandBuffer_, 0, 1, &scissorRect);

        /* Avoid scissor update with each graphics pipeline binding (as long as render pass does not change) */
        scissorRectInvalidated_ = false;
    }
}

void VKCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineVK = LLGL_CAST(VKComputePipeline&, computePipeline);
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineVK.GetVkPipeline());
}

/* ----- Queries ----- */

void VKCommandBuffer::BeginQuery(Query& query)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);

    /* Determine control flags (for either 'SamplesPassed' or 'AnySamplesPassed') */
    VkQueryControlFlags flags = 0;

    if (query.GetType() == QueryType::SamplesPassed)
        flags |= VK_QUERY_CONTROL_PRECISE_BIT;

    vkCmdBeginQuery(commandBuffer_, queryVK.GetVkQueryPool(), 0, flags);
}

void VKCommandBuffer::EndQuery(Query& query)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);
    vkCmdEndQuery(commandBuffer_, queryVK.GetVkQueryPool(), 0);
}

bool VKCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);

    /* Store result directly into output parameter */
    auto stateResult = vkGetQueryPoolResults(
        device_, queryVK.GetVkQueryPool(), 0, 1,
        sizeof(result), &result, sizeof(std::uint64_t),
        VK_QUERY_RESULT_64_BIT
    );

    /* Check if result is not ready yet */
    if (stateResult == VK_NOT_READY)
        return false;

    VKThrowIfFailed(stateResult, "failed to retrieve results from Vulkan query pool");

    return true;
}

bool VKCommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);

    /* Store results in intermediate memory */
    std::uint64_t intermediateResults[11];

    auto stateResult = vkGetQueryPoolResults(
        device_, queryVK.GetVkQueryPool(), 0, 1,
        sizeof(intermediateResults), intermediateResults, sizeof(std::uint64_t),
        VK_QUERY_RESULT_64_BIT
    );

    /* Check if result is not ready yet */
    if (stateResult == VK_NOT_READY)
        return false;

    VKThrowIfFailed(stateResult, "failed to retrieve results from Vulkan query pool");

    /* Copy result to output parameter */
    result.numPrimitivesGenerated               = 0;
    result.numVerticesSubmitted                 = intermediateResults[ 0]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
    result.numPrimitivesSubmitted               = intermediateResults[ 1]; // VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
    result.numVertexShaderInvocations           = intermediateResults[ 2]; // VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
    result.numTessControlShaderInvocations      = intermediateResults[ 8]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
    result.numTessEvaluationShaderInvocations   = intermediateResults[ 9]; // VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
    result.numGeometryShaderInvocations         = intermediateResults[ 3]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
    result.numFragmentShaderInvocations         = intermediateResults[ 7]; // VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
    result.numComputeShaderInvocations          = intermediateResults[10]; // VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
    result.numGeometryPrimitivesGenerated       = intermediateResults[ 4]; // VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
    result.numClippingInputPrimitives           = intermediateResults[ 5]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
    result.numClippingOutputPrimitives          = intermediateResults[ 6]; // VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT

    return true;
}

void VKCommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    //todo
}

void VKCommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Drawing ----- */

void VKCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    vkCmdDraw(commandBuffer_, numVertices, 1, firstVertex, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, 1, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, firstInstance);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    vkCmdDrawIndexed(commandBuffer_, numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

/* ----- Compute ----- */

void VKCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    vkCmdDispatch(commandBuffer_, groupSizeX, groupSizeY, groupSizeZ);
}

/* ----- Extended functions ----- */

void VKCommandBuffer::AcquireNextBuffer()
{
    commandBufferIndex_ = (commandBufferIndex_ + 1) % commandBufferList_.size();
    commandBuffer_      = commandBufferList_[commandBufferIndex_];
    recordingFence_     = recordingFenceList_[commandBufferIndex_].Get();
}


/*
 * ======= Private: =======
 */

void VKCommandBuffer::CreateCommandPool(std::uint32_t queueFamilyIndex)
{
    /* Create command pool */
    VkCommandPoolCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = queueFamilyIndex;
    }
    auto result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");
}

void VKCommandBuffer::CreateCommandBuffers(std::size_t bufferCount)
{
    /* Allocate command buffers */
    commandBufferList_.resize(bufferCount);

    VkCommandBufferAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.commandPool           = commandPool_;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount    = static_cast<std::uint32_t>(bufferCount);
    }
    auto result = vkAllocateCommandBuffers(device_, &allocInfo, commandBufferList_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffers");
}

void VKCommandBuffer::CreateRecordingFences(VkQueue graphicsQueue, std::size_t numFences)
{
    recordingFenceList_.reserve(numFences);

    VkFenceCreateInfo createInfo;
    {
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
    }

    for (std::size_t i = 0; i < numFences; ++i)
    {
        VKPtr<VkFence> fence { device_, vkDestroyFence };
        {
            /* Create fence for command buffer recording */
            auto result = vkCreateFence(device_, &createInfo, nullptr, fence.ReleaseAndGetAddressOf());
            VKThrowIfFailed(result, "failed to create Vulkan fence");

            /* Initial fence signal */
            vkQueueSubmit(graphicsQueue, 0, nullptr, fence);
        }
        recordingFenceList_.emplace_back(std::move(fence));
    }
}

void VKCommandBuffer::ClearFramebufferAttachments(std::uint32_t numAttachments, const VkClearAttachment* attachments)
{
    if (numAttachments > 0)
    {
        /* Clear framebuffer attachments at the entire image region */
        VkClearRect clearRect;
        {
            clearRect.rect.offset.x     = 0;
            clearRect.rect.offset.y     = 0;
            clearRect.rect.extent       = framebufferExtent_;
            clearRect.baseArrayLayer    = 0;
            clearRect.layerCount        = 1;
        }
        vkCmdClearAttachments(commandBuffer_, numAttachments, attachments, 1, &clearRect);
    }
}

//TODO: current unused; previously used for 'Clear' function
#if 0

void VKCommandBuffer::BeginClearImage(
    VkImageMemoryBarrier& clearToPresentBarrier, VkImage image, const VkImageAspectFlags clearFlags,
    const VkClearColorValue* clearColor, const VkClearDepthStencilValue* clearDepthStencil)
{
    /* Initialize image subresource range */
    VkImageSubresourceRange subresourceRange;
    {
        subresourceRange.aspectMask     = clearFlags;
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = 1;
    }

    /* Initialize pre-barrier */
    VkImageMemoryBarrier presentToClearBarrier;
    {
        presentToClearBarrier.sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        presentToClearBarrier.pNext                 = nullptr;
        presentToClearBarrier.srcAccessMask         = VK_ACCESS_MEMORY_READ_BIT;
        presentToClearBarrier.dstAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
        presentToClearBarrier.oldLayout             = VK_IMAGE_LAYOUT_UNDEFINED;
        presentToClearBarrier.newLayout             = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        presentToClearBarrier.srcQueueFamilyIndex   = queuePresentFamily_;
        presentToClearBarrier.dstQueueFamilyIndex   = queuePresentFamily_;
        presentToClearBarrier.image                 = image;
        presentToClearBarrier.subresourceRange      = subresourceRange;
    }

    /* Initialize post-barrier */
    {
        clearToPresentBarrier.sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        clearToPresentBarrier.pNext                 = nullptr;
        clearToPresentBarrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
        clearToPresentBarrier.dstAccessMask         = VK_ACCESS_MEMORY_READ_BIT;
        clearToPresentBarrier.oldLayout             = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        clearToPresentBarrier.newLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        clearToPresentBarrier.srcQueueFamilyIndex   = queuePresentFamily_;
        clearToPresentBarrier.dstQueueFamilyIndex   = queuePresentFamily_;
        clearToPresentBarrier.image                 = image;
        clearToPresentBarrier.subresourceRange      = subresourceRange;
    }

    /* Record barrier and clear color commands */
    vkCmdPipelineBarrier(
        commandBuffer_,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &presentToClearBarrier
    );

    if ((clearFlags & VK_IMAGE_ASPECT_COLOR_BIT) != 0)
        vkCmdClearColorImage(commandBuffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearColor, 1, &subresourceRange);
    if ((clearFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)
        vkCmdClearDepthStencilImage(commandBuffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearDepthStencil, 1, &subresourceRange);
}

void VKCommandBuffer::EndClearImage(VkImageMemoryBarrier& clearToPresentBarrier)
{
    vkCmdPipelineBarrier(
        commandBuffer_,                         // VkCommandBuffer
        VK_PIPELINE_STAGE_TRANSFER_BIT,         // source stage
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,   // destination stage
        0,                                      // no dependencie flags
        0, nullptr,                             // no memory barriers
        0, nullptr,                             // no buffer memory barriers
        1, &clearToPresentBarrier               // image memory barriers
    );
}

#endif


} // /namespace LLGL



// ================================================================================
