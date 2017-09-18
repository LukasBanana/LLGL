/*
 * VKCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKCommandBuffer.h"
#include "VKRenderContext.h"
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

/* ----- Configuration ----- */

void VKCommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    //todo
}

void VKCommandBuffer::SetViewport(const Viewport& viewport)
{
    //todo
}

void VKCommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    //todo
}

void VKCommandBuffer::SetScissor(const Scissor& scissor)
{
    //todo
}

void VKCommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
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

void VKCommandBuffer::SetClearStencil(int stencil)
{
    clearValue_.depthStencil.stencil = static_cast<uint32_t>(stencil);
}

void VKCommandBuffer::Clear(long flags)
{
    //todo
}

void VKCommandBuffer::ClearTarget(unsigned int targetIndex, const LLGL::ColorRGBAf& color)
{
    //todo
}

/* ----- Buffers ------ */

void VKCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    //todo
}

void VKCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    //todo
}

void VKCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    //todo
}

void VKCommandBuffer::SetConstantBuffer(Buffer& buffer, unsigned int slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetStorageBuffer(Buffer& buffer, unsigned int slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetStorageBufferArray(BufferArray& bufferArray, unsigned int startSlot, long /*shaderStageFlags*/)
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

void VKCommandBuffer::SetTexture(Texture& texture, unsigned int slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long /*shaderStageFlags*/)
{
    //todo
}

/* ----- Sampler States ----- */

void VKCommandBuffer::SetSampler(Sampler& sampler, unsigned int slot, long /*shaderStageFlags*/)
{
    //todo
}

void VKCommandBuffer::SetSamplerArray(SamplerArray& samplerArray, unsigned int startSlot, long /*shaderStageFlags*/)
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
    auto& renderContextVK = LLGL_CAST(VKRenderContext&, renderContext);

    const auto& renderPass = renderContextVK.GetSwapChainRenderPass();

    renderContextVK.SetPresentCommandBuffer(this);

    //todo
}

/* ----- Pipeline States ----- */

void VKCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    //todo
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

void VKCommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    //todo
}

void VKCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    //todo
}

void VKCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    //todo
}

void VKCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    //todo
}

void VKCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    //todo
}

void VKCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    //todo
}

void VKCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    //todo
}

void VKCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    //todo
}

/* ----- Compute ----- */

void VKCommandBuffer::Dispatch(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    //todo
}

/* ----- Misc ----- */

void VKCommandBuffer::SyncGPU()
{
    //todo
}


/*
 * ======= Private: =======
 */

void VKCommandBuffer::CreateCommandPool(uint32_t queueFamilyIndex)
{
    /* Create command pool */
    VkCommandPoolCreateInfo createInfo;

    createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext            = nullptr;
    createInfo.flags            = 0;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    VkResult result = vkCreateCommandPool(device_, &createInfo, nullptr, commandPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan command pool");
}

void VKCommandBuffer::CreateCommandBuffers(size_t bufferCount)
{
    /* Allocate command buffers */
    commandBufferList_.resize(bufferCount);

    VkCommandBufferAllocateInfo allocInfo;

    allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext                 = nullptr;
    allocInfo.commandPool           = commandPool_;
    allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount    = static_cast<uint32_t>(bufferCount);

    VkResult result = vkAllocateCommandBuffers(device_, &allocInfo, commandBufferList_.data());
    VKThrowIfFailed(result, "failed to allocate Vulkan command buffers");
}


} // /namespace LLGL



// ================================================================================
