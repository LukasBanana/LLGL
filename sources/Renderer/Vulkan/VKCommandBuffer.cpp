/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKRenderContext.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "RenderState/VKComputePipeline.h"
#include "RenderState/VKResourceViewHeap.h"
#include "RenderState/VKQuery.h"
#include "Texture/VKSampler.h"
#include "Texture/VKSamplerArray.h"
#include "Texture/VKRenderTarget.h"
#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"
#include "Buffer/VKIndexBuffer.h"
#include "../CheckedCast.h"
#include <cstddef>


namespace LLGL
{


static const std::uint32_t g_maxNumViewportsPerBatch = 16;

VKCommandBuffer::VKCommandBuffer(const VKPtr<VkDevice>& device, std::size_t bufferCount, const QueueFamilyIndices& queueFamilyIndices) :
    device_             { device                           },
    commandPool_        { device, vkDestroyCommandPool     },
    queuePresentFamily_ { queueFamilyIndices.presentFamily }
{
    CreateCommandPool(queueFamilyIndices.graphicsFamily);
    CreateCommandBuffers(bufferCount);
}

VKCommandBuffer::~VKCommandBuffer()
{
    vkFreeCommandBuffers(device_, commandPool_, static_cast<std::uint32_t>(commandBufferList_.size()), commandBufferList_.data());
}

/* ----- Configuration ----- */

void VKCommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    //todo
}

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

// Convert Viewport type to VkViewport type
static void Convert(VkViewport& dst, const Viewport& src)
{
    dst.x           = src.x;
    dst.y           = src.y;
    dst.width       = src.width;
    dst.height      = src.height;
    dst.minDepth    = src.minDepth;
    dst.maxDepth    = src.maxDepth;
}

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport type */
        vkCmdSetViewport(commandBuffer_, 0, 1, reinterpret_cast<const VkViewport*>(&viewport));
    }
    else
    {
        /* Convert viewport to VkViewport type */
        VkViewport viewportVK;
        Convert(viewportVK, viewport);
        vkCmdSetViewport(commandBuffer_, 0, 1, &viewportVK);
    }
}

void VKCommandBuffer::SetViewportArray(std::uint32_t numViewports, const Viewport* viewportArray)
{
    if (IsCompatibleToVkViewport())
    {
        /* Cast viewport to VkViewport types */
        vkCmdSetViewport(commandBuffer_, 0, numViewports, reinterpret_cast<const VkViewport*>(viewportArray));
    }
    else
    {
        VkViewport viewportsVK[g_maxNumViewportsPerBatch];

        for (std::uint32_t i = 0, first = 0, count = 0; i < numViewports; numViewports -= count)
        {
            /* Convert viewport to VkViewport types */
            count = std::min(numViewports, g_maxNumViewportsPerBatch);

            for (first = i; i < first + count; ++i)
                Convert(viewportsVK[i - first], viewportArray[i]);

            vkCmdSetViewport(commandBuffer_, first, count, viewportsVK);
        }
    }
}

// Convert Scissor type to VkRect2D type
static void Convert(VkRect2D& dst, const Scissor& src)
{
    dst.offset.x        = src.x;
    dst.offset.y        = src.y;
    dst.extent.width    = static_cast<std::uint32_t>(std::max(0, src.width));
    dst.extent.height   = static_cast<std::uint32_t>(std::max(0, src.height));
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    VkRect2D scissorVK;
    Convert(scissorVK, scissor);
    vkCmdSetScissor(commandBuffer_, 0, 1, &scissorVK);
}

void VKCommandBuffer::SetScissorArray(std::uint32_t numScissors, const Scissor* scissorArray)
{
    VkRect2D scissorsVK[g_maxNumViewportsPerBatch];

    for (std::uint32_t i = 0, first = 0, count = 0; i < numScissors; numScissors -= count)
    {
        /* Convert viewport to VkViewport types */
        count = std::min(numScissors, g_maxNumViewportsPerBatch);

        for (first = i; i < first + count; ++i)
            Convert(scissorsVK[i - first], scissorArray[i]);

        vkCmdSetScissor(commandBuffer_, first, count, scissorsVK);
    }
}

void VKCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearColor_.float32[0] = color.r;
    clearColor_.float32[1] = color.g;
    clearColor_.float32[2] = color.b;
    clearColor_.float32[3] = color.a;
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
    static const std::uint32_t maxNumAttachments = 32;

    VkClearAttachment attachments[maxNumAttachments + 1];

    std::uint32_t numAttachments = 0;
    
    /* Fill clear descriptors for color attachments */
    if ((flags & ClearFlags::Color) != 0)
    {
        numAttachments = std::min(numColorAttachments_, maxNumAttachments);
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
    if ((flags & ClearFlags::DepthStencil) != 0 && hasDepthStencilAttachment_)
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

void VKCommandBuffer::ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color)
{
    VkClearAttachment attachment;
    {
        attachment.aspectMask                   = VK_IMAGE_ASPECT_COLOR_BIT;
        attachment.colorAttachment              = targetIndex;
        attachment.clearValue.color.float32[0]  = color.r;
        attachment.clearValue.color.float32[1]  = color.g;
        attachment.clearValue.color.float32[2]  = color.b;
        attachment.clearValue.color.float32[3]  = color.a;
    }
    ClearFramebufferAttachments(1, &attachment);
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

/* ----- Resource View Heaps ----- */

//private
void VKCommandBuffer::BindResourceViewHeap(VKResourceViewHeap& resourceHeapVK, VkPipelineBindPoint bindingPoint, std::uint32_t startSlot)
{
    vkCmdBindDescriptorSets(
        commandBuffer_,
        bindingPoint,
        resourceHeapVK.GetVkPipelineLayout(),
        startSlot,
        static_cast<std::uint32_t>(resourceHeapVK.GetVkDescriptorSets().size()),
        resourceHeapVK.GetVkDescriptorSets().data(),
        0,
        nullptr
    );
}

void VKCommandBuffer::SetGraphicsResourceViewHeap(ResourceViewHeap& resourceHeap, std::uint32_t startSlot)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceViewHeap&, resourceHeap);
    BindResourceViewHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_GRAPHICS, startSlot);
}

void VKCommandBuffer::SetComputeResourceViewHeap(ResourceViewHeap& resourceHeap, std::uint32_t startSlot)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceViewHeap&, resourceHeap);
    BindResourceViewHeap(resourceHeapVK, VK_PIPELINE_BIND_POINT_COMPUTE, startSlot);
}

/* ----- Render Targets ----- */

void VKCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    auto& renderTargetVK = LLGL_CAST(VKRenderTarget&, renderTarget);

    /* Begin command buffer and render pass */
    if (!IsCommandBufferActive())
        BeginCommandBuffer();

    /* Set new render pass */
    SetRenderPass(
        renderTargetVK.GetVkRenderPass(),
        renderTargetVK.GetVkFramebuffer(),
        renderTargetVK.GetVkExtent()
    );

    /* Store information about framebuffer attachments */
    numColorAttachments_        = renderTargetVK.GetNumColorAttachments();
    hasDepthStencilAttachment_  = renderTargetVK.HasDepthStencilAttachment();
}

/*
TODO:
BeginCommandBuffer at this point is only a workaround!
Maybe it can be integrated into a Begin/EndRenderPass function with a new interface "RenderPass".
*/
void VKCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextVK = LLGL_CAST(VKRenderContext&, renderContext);

    //TODO:
    //  this must be done for all command buffers at the end of the "VKRenderContext::Present" function
    /* Switch internal command buffer for the respective render context presentation index */
    renderContextVK.SetPresentCommandBuffer(this);

    /* Begin command buffer and render pass */
    if (!IsCommandBufferActive())
        BeginCommandBuffer();

    /* Set new render pass */
    SetRenderPass(
        renderContextVK.GetSwapChainRenderPass(),
        renderContextVK.GetSwapChainFramebuffer(),
        renderContextVK.GetSwapChainExtent()
    );

    /* Store information about framebuffer attachments */
    numColorAttachments_        = 1;
    hasDepthStencilAttachment_  = false;
    //hasDepthStencilAttachment_  = true;
}


/* ----- Pipeline States ----- */

void VKCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineVK = LLGL_CAST(VKGraphicsPipeline&, graphicsPipeline);
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineVK.GetVkPipeline());
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

void VKCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numVertices, 1, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numVertices, 1, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, 0);
}

void VKCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset)
{
    vkCmdDraw(commandBuffer_, numVertices, numInstances, firstVertex, instanceOffset);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    vkCmdDrawIndexed(commandBuffer_, numVertices, numInstances, firstIndex, 0, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void VKCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset)
{
    vkCmdDrawIndexed(commandBuffer_, numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void VKCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    vkCmdDispatch(commandBuffer_, groupSizeX, groupSizeY, groupSizeZ);
}

/* --- Extended functions --- */

void VKCommandBuffer::SetPresentIndex(std::uint32_t idx)
{
    commandBuffer_          = commandBufferList_[idx];
    commandBufferActiveIt_  = commandBufferActiveList_.begin() + idx;
}

bool VKCommandBuffer::IsCommandBufferActive() const
{
    return *commandBufferActiveIt_;
}

void VKCommandBuffer::BeginCommandBuffer()
{
    /* Begin recording of current command buffer */
    VkCommandBufferBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.flags             = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo  = nullptr;
    }
    auto result = vkBeginCommandBuffer(commandBuffer_, &beginInfo);
    VKThrowIfFailed(result, "failed to begin Vulkan command buffer");

    /* Store activity state */
    *commandBufferActiveIt_ = true;
}

void VKCommandBuffer::EndCommandBuffer()
{
    /* End recording of current command buffer */
    auto result = vkEndCommandBuffer(commandBuffer_);
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");

    /* Store activity state */
    *commandBufferActiveIt_ = false;
}

void VKCommandBuffer::SetRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkExtent2D& extent)
{
    if (renderPass_)
        EndRenderPass();

    if (renderPass != VK_NULL_HANDLE)
    {
        /* Begin new render pass */
        BeginRenderPass(renderPass, framebuffer, extent);

        /* Store render pass and framebuffer attributes */
        renderPass_         = renderPass;
        framebuffer_        = framebuffer;
        framebufferExtent_  = extent;
    }
}

void VKCommandBuffer::SetRenderPassNull()
{
    if (renderPass_)
    {
        /* End current render pass */
        EndRenderPass();

        /* Reset render pass and framebuffer attributes */
        renderPass_     = VK_NULL_HANDLE;
        framebuffer_    = VK_NULL_HANDLE;
    }
}

void VKCommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkExtent2D& extent)
{
    /* Record begin of render pass */
    VkRenderPassBeginInfo beginInfo;
    {
        beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.renderPass        = renderPass;
        beginInfo.framebuffer       = framebuffer;
        beginInfo.renderArea.offset = { 0, 0 };
        beginInfo.renderArea.extent = extent;
        beginInfo.clearValueCount   = 0;//1;
        beginInfo.pClearValues      = nullptr;//(&clearValue_);
    }
    vkCmdBeginRenderPass(commandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKCommandBuffer::EndRenderPass()
{
    /* Record and of render pass */
    vkCmdEndRenderPass(commandBuffer_);
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

    commandBuffer_ = commandBufferList_.front();

    /* Allocate list to keep track of which command buffers are active */
    commandBufferActiveList_.resize(bufferCount);
    commandBufferActiveIt_ = commandBufferActiveList_.end();
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

//TODO: current unused; previously used for 'Clear' and 'ClearTarget' functions
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
