/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKRenderContext.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "RenderState/VKResourceViewHeap.h"
#include "RenderState/VKQuery.h"
#include "Texture/VKSampler.h"
#include "Texture/VKSamplerArray.h"
#include "Buffer/VKBuffer.h"
#include "Buffer/VKBufferArray.h"
#include "Buffer/VKIndexBuffer.h"
#include "../CheckedCast.h"


namespace LLGL
{


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

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    //todo
}

void VKCommandBuffer::SetViewportArray(std::uint32_t numViewports, const Viewport* viewportArray)
{
    //todo
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    //todo
}

void VKCommandBuffer::SetScissorArray(std::uint32_t numScissors, const Scissor* scissorArray)
{
    //todo
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

void VKCommandBuffer::Clear(long flags)
{
    /* Initialize clear flags */
    VkImageAspectFlags clearFlags[2] = { 0, 0 };

    if (imageColor_ != VK_NULL_HANDLE)
    {
        if ((flags & ClearFlags::Color) != 0)
            clearFlags[0] |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (imageDepthStencil_ != VK_NULL_HANDLE)
    {
        if ((flags & ClearFlags::Depth) != 0)
            clearFlags[1] |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if ((flags & ClearFlags::Stencil) != 0)
            clearFlags[1] |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    /* Begin and end clear image commands interleaved */
    VkImageMemoryBarrier clearToPresentBarrier[2];

    if (clearFlags[0] != 0)
        BeginClearImage(clearToPresentBarrier[0], imageColor_, clearFlags[0], &clearColor_, nullptr);
    if (clearFlags[1] != 0)
        BeginClearImage(clearToPresentBarrier[1], imageDepthStencil_, clearFlags[1], nullptr, &clearDepthStencil_);
    if (clearFlags[0] != 0)
        EndClearImage(clearToPresentBarrier[0]);
    if (clearFlags[1] != 0)
        EndClearImage(clearToPresentBarrier[1]);
}

void VKCommandBuffer::ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color)
{
    if (targetIndex == 0 && imageColor_ != VK_NULL_HANDLE)
    {
        /* Copy clear value */
        VkClearColorValue clearColor;
        clearColor.float32[0] = color.r;
        clearColor.float32[1] = color.g;
        clearColor.float32[2] = color.b;
        clearColor.float32[3] = color.a;

        /* Clear color image target */
        VkImageMemoryBarrier clearToPresentBarrier;
        BeginClearImage(clearToPresentBarrier, imageColor_, VK_IMAGE_ASPECT_COLOR_BIT, &clearColor, nullptr);
        EndClearImage(clearToPresentBarrier);
    }
}

/* ----- Buffers ------ */

void VKCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);

    VkBuffer buffers[] = { bufferVK.Get() };
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
    vkCmdBindIndexBuffer(commandBuffer_, indexBufferVK.Get(), 0, indexBufferVK.GetIndexType());
}

void VKCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    //todo
}

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

/* ----- Textures ----- */

void VKCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    //todo
}

/* ----- Sampler States ----- */

void VKCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long /*shaderStageFlags*/)
{
    //todo
}

/* ----- Resource View Heaps ----- */

void VKCommandBuffer::SetGraphicsResourceViewHeap(ResourceViewHeap& resourceHeap, std::uint32_t startSlot)
{
    auto& resourceHeapVK = LLGL_CAST(VKResourceViewHeap&, resourceHeap);

    /* Bind descriptor set */
    vkCmdBindDescriptorSets(
        commandBuffer_,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        resourceHeapVK.GetVkPipelineLayout(),
        startSlot,
        static_cast<std::uint32_t>(resourceHeapVK.GetVkDescriptorSets().size()),
        resourceHeapVK.GetVkDescriptorSets().data(),
        0,
        nullptr
    );
}

/* ----- Render Targets ----- */

void VKCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void VKCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    /* Get render context object */
    auto& renderContextVK = LLGL_CAST(VKRenderContext&, renderContext);
    renderContextVK.SetPresentCommandBuffer(this);

    /* Get swap chain objects */
    renderPass_ = renderContextVK.GetSwapChainRenderPass().Get();
    framebuffer_ = renderContextVK.GetSwapChainFramebuffer();
    imageColor_ = renderContextVK.GetSwapChainImage();

    /* Begin command buffer and render pass */
    BeginCommandBuffer();
    BeginRenderPass(renderPass_, framebuffer_, renderContextVK.GetSwapChainExtent());
}


/* ----- Pipeline States ----- */

void VKCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineVK = LLGL_CAST(VKGraphicsPipeline&, graphicsPipeline);
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineVK.GetVkPipeline());
}

void VKCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

void VKCommandBuffer::BeginQuery(Query& query)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);

    /* Determine control flags (for either 'SamplesPassed' or 'AnySamplesPassed') */
    VkQueryControlFlags flags = 0;

    if (query.GetType() == QueryType::SamplesPassed)
        flags |= VK_QUERY_CONTROL_PRECISE_BIT;

    vkCmdBeginQuery(commandBuffer_, queryVK.GetQueryPool(), 0, flags);
}

void VKCommandBuffer::EndQuery(Query& query)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);
    vkCmdEndQuery(commandBuffer_, queryVK.GetQueryPool(), 0);
}

bool VKCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryVK = LLGL_CAST(VKQuery&, query);

    /* Store result directly into output parameter */
    auto stateResult = vkGetQueryPoolResults(
        device_, queryVK.GetQueryPool(), 0, 1,
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
        device_, queryVK.GetQueryPool(), 0, 1,
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
    commandBuffer_ = commandBufferList_[idx];
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
}

void VKCommandBuffer::EndCommandBuffer()
{
    auto result = vkEndCommandBuffer(commandBuffer_);
    VKThrowIfFailed(result, "failed to end Vulkan command buffer");
}

void VKCommandBuffer::BeginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, const VkExtent2D& extent)
{
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
}

void VKCommandBuffer::BeginClearImage(
    VkImageMemoryBarrier& clearToPresentBarrier, VkImage image,
    const VkImageAspectFlags clearFlags, const VkClearColorValue* clearColor, const VkClearDepthStencilValue* clearDepthStencil)
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
        presentToClearBarrier.sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        presentToClearBarrier.pNext                 = nullptr;
        presentToClearBarrier.srcAccessMask         = VK_ACCESS_TRANSFER_WRITE_BIT;
        presentToClearBarrier.dstAccessMask         = VK_ACCESS_MEMORY_READ_BIT;
        presentToClearBarrier.oldLayout             = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        presentToClearBarrier.newLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        presentToClearBarrier.srcQueueFamilyIndex   = queuePresentFamily_;
        presentToClearBarrier.dstQueueFamilyIndex   = queuePresentFamily_;
        presentToClearBarrier.image                 = image;
        presentToClearBarrier.subresourceRange      = subresourceRange;
    }

    /* Record barrier and clear color commands */
    vkCmdPipelineBarrier(commandBuffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToClearBarrier);

    if ((clearFlags & VK_IMAGE_ASPECT_COLOR_BIT) != 0)
        vkCmdClearColorImage(commandBuffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearColor, 1, &subresourceRange);
    if ((clearFlags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)
        vkCmdClearDepthStencilImage(commandBuffer_, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, clearDepthStencil, 1, &subresourceRange);
}

void VKCommandBuffer::EndClearImage(VkImageMemoryBarrier& clearToPresentBarrier)
{
    vkCmdPipelineBarrier(commandBuffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);
}


} // /namespace LLGL



// ================================================================================
