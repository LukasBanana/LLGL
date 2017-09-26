/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKRenderContext.h"
#include "RenderState/VKGraphicsPipeline.h"
#include "Texture/VKSampler.h"
#include "Texture/VKSamplerArray.h"
#include "Buffer/VKBuffer.h"
#include "../CheckedCast.h"


namespace LLGL
{


VKCommandBuffer::VKCommandBuffer(const VKPtr<VkDevice>& device, size_t bufferCount, const QueueFamilyIndices& queueFamilyIndices) :
    device_      { device                       },
    commandPool_ { device, vkDestroyCommandPool }
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
    clearValue_.color.float32[0] = color.r;
    clearValue_.color.float32[1] = color.g;
    clearValue_.color.float32[2] = color.b;
    clearValue_.color.float32[3] = color.a;
}

void VKCommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depthStencil.depth = depth;
}

void VKCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.depthStencil.stencil = stencil;
}

void VKCommandBuffer::Clear(long flags)
{
    //todo
}

void VKCommandBuffer::ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color)
{
    //todo
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
    //todo
}

void VKCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferVK = LLGL_CAST(VKBuffer&, buffer);
    vkCmdBindIndexBuffer(commandBuffer_, bufferVK.Get(), 0, VK_INDEX_TYPE_UINT32);
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
    auto renderPass = renderContextVK.GetSwapChainRenderPass().Get();
    auto framebuffer = renderContextVK.GetSwapChainFramebuffer();

    /* Begin command buffer and render pass */
    BeginCommandBuffer();
    BeginRenderPass(renderPass, framebuffer, renderContextVK.GetSwapChainExtent());
}


/* ----- Pipeline States ----- */

void VKCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineVK = LLGL_CAST(VKGraphicsPipeline&, graphicsPipeline);
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineVK.Get());
}

void VKCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

void VKCommandBuffer::BeginQuery(Query& query)
{
    //todo
}

void VKCommandBuffer::EndQuery(Query& query)
{
    //todo
}

bool VKCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    //todo
    return false;
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

/* ----- Misc ----- */

void VKCommandBuffer::SyncGPU()
{
    //todo
}

/* --- Extended functions --- */

void VKCommandBuffer::SetPresentIndex(std::uint32_t idx)
{
    commandBuffer_ = commandBufferList_[idx];
}

void VKCommandBuffer::BeginCommandBuffer()
{
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
        beginInfo.clearValueCount   = 1;
        beginInfo.pClearValues      = (&clearValue_);
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
        createInfo.flags            = 0;
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


} // /namespace LLGL



// ================================================================================
