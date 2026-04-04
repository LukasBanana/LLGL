/*
 * WGCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGCommandBuffer.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


WGCommandBuffer::WGCommandBuffer(WGPUDevice device, const CommandBufferDescriptor& desc) :
    device_ { device }
{
}

void WGCommandBuffer::Begin()
{
    /* Request new command encoder */
    commandEncoder_ = wgpuDeviceCreateCommandEncoder(device_, nullptr);
}

void WGCommandBuffer::End()
{
    /* Finish any active pass encoder */
    FlushPassEncoders();

    /* Finish command encoding and get final command buffer  */
    if (commandBuffer_ != nullptr)
    {
        wgpuCommandBufferRelease(commandBuffer_);
        commandBuffer_ = nullptr;
    }
    commandBuffer_ = wgpuCommandEncoderFinish(commandEncoder_, nullptr);

    /* Release command encoder */
    wgpuCommandEncoderRelease(commandEncoder_);
    commandEncoder_ = nullptr;

    //TODO: immediate submit
}

void WGCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::CopyBufferFromTexture(Buffer& dstBuffer, std::uint64_t dstOffset, Texture& srcTexture, const TextureRegion& srcRegion, std::uint32_t rowStride, std::uint32_t layerStride)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::FillBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, std::uint32_t value, std::uint64_t fillSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::CopyTexture(Texture& dstTexture, const TextureLocation& dstLocation, Texture& srcTexture, const TextureLocation& srcLocation, const Extent3D& extent)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::CopyTextureFromBuffer(Texture& dstTexture, const TextureRegion& dstRegion, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint32_t rowStride, std::uint32_t layerStride)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::CopyTextureFromFramebuffer(Texture& dstTexture, const TextureRegion& dstRegion, const Offset2D& srcOffset)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::GenerateMips(Texture& texture)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetViewport(const Viewport& viewport)
{
    wgpuRenderPassEncoderSetViewport(renderPassEncoder_, viewport.x, viewport.y, viewport.width, viewport.height, viewport.minDepth, viewport.maxDepth);
}

void WGCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetScissor(const Scissor& scissor)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    //todo
}

void WGCommandBuffer::SetVertexBuffer(Buffer& buffer, std::uint32_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::ResourceBarrier(std::uint32_t numBuffers, Buffer* const * buffers, std::uint32_t numTextures, Texture* const * textures)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::BeginRenderPass(RenderTarget& renderTarget, const RenderPass* renderPass, std::uint32_t numClearValues, const ClearValue* clearValues, std::uint32_t swapBufferIndex)
{
    LLGL_ASSERT(renderPassEncoder_ == nullptr);
    WGPURenderPassDescriptor renderPassDesc;
    {
        renderPassDesc.nextInChain              = nullptr;
        renderPassDesc.label                    = WGPU_STRING_VIEW_INIT; //WGPUStringView
        renderPassDesc.colorAttachmentCount     = 0; //size_t
        renderPassDesc.colorAttachments         = nullptr; //WGPURenderPassColorAttachment const *
        renderPassDesc.depthStencilAttachment   = nullptr; //WGPU_NULLABLE WGPURenderPassDepthStencilAttachment const *
        renderPassDesc.occlusionQuerySet        = nullptr;
        renderPassDesc.timestampWrites          = nullptr;
    }
    renderPassEncoder_ = wgpuCommandEncoderBeginRenderPass(commandEncoder_, nullptr);
}

void WGCommandBuffer::EndRenderPass()
{
    LLGL_ASSERT(renderPassEncoder_ != nullptr);
    wgpuRenderPassEncoderEnd(renderPassEncoder_);
    wgpuRenderPassEncoderRelease(renderPassEncoder_);
    renderPassEncoder_ = nullptr;
}

void WGCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    //todo
}

void WGCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetBlendFactor(const float color[4])
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::EndRenderCondition()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::EndStreamOutput()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, 1, firstVertex, 0);
}

void WGCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, 1, firstIndex, 0, 0);
}

void WGCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, 1, firstIndex, vertexOffset, 0);
}

void WGCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, numInstances, firstVertex, 0);
}

void WGCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, numInstances, firstVertex, firstInstance);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, 0, 0);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void WGCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::DrawStreamOutput()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    EnsureComputeEncoder();
    wgpuComputePassEncoderDispatchWorkgroups(computePassEncoder_, numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void WGCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::PushDebugGroup(const char* name)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::PopDebugGroup()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


/*
 * ======= Private: =======
 */

void WGCommandBuffer::EnsureComputeEncoder()
{
    LLGL_ASSERT(renderPassEncoder_ == nullptr);
    if (computePassEncoder_ == nullptr)
        computePassEncoder_ = wgpuCommandEncoderBeginComputePass(commandEncoder_, nullptr);
}

void WGCommandBuffer::FlushPassEncoders()
{
    if (computePassEncoder_ != nullptr)
    {
        wgpuComputePassEncoderRelease(computePassEncoder_);
        computePassEncoder_ = nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
