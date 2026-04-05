/*
 * WGCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGCommandBuffer.h"
#include "../WGCore.h"
#include "../Buffer/WGBuffer.h"
#include "../RenderState/WGRenderPipeline.h"
#include "../RenderState/WGComputePipeline.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Backend/WebGPU/NativeHandle.h>


namespace LLGL
{


WGCommandBuffer::WGCommandBuffer(WGPUDevice device, WGPUQueue queue, const CommandBufferDescriptor& desc) :
    device_            { device                                                    },
    queue_             { queue                                                     },
    isImmediateSubmit_ { ((desc.flags & CommandBufferFlags::ImmediateSubmit) != 0) }
{
}

void WGCommandBuffer::Begin()
{
    ResetRenderStates();

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

    /* Submit immediately to queue if enabled */
    if (isImmediateSubmit_)
        wgpuQueueSubmit(queue_, 1, &commandBuffer_);
}

void WGCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferWG = LLGL_CAST(WGBuffer&, dstBuffer);
    wgpuCommandEncoderWriteBuffer(commandEncoder_, dstBufferWG.GetNative(), dstOffset, static_cast<const std::uint8_t*>(data), dataSize);
}

void WGCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferWG = LLGL_CAST(WGBuffer&, dstBuffer);
    auto& srcBufferWG = LLGL_CAST(WGBuffer&, srcBuffer);
    wgpuCommandEncoderCopyBufferToBuffer(commandEncoder_, srcBufferWG.GetNative(), srcOffset, dstBufferWG.GetNative(), dstOffset, size);
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
    renderEncoderState_.viewport = viewport;
    renderDirtyBits_ |= DirtyBit_Viewports;
}

void WGCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetScissor(const Scissor& scissor)
{
    renderEncoderState_.scissor = scissor;
    renderDirtyBits_ |= DirtyBit_Scissors;
}

void WGCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    renderEncoderState_.vertexBuffers[0] = bufferWG.GetNative();
    renderEncoderState_.vertexBufferCount = 1;
    renderDirtyBits_ |= DirtyBit_VertexBuffers;
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
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainWG = LLGL_CAST(WGSwapChain&, renderTarget);

        WGFramebuffer framebuffer = swapChainWG.GetCurrentFramebuffer();

        WGPURenderPassColorAttachment colorAttachment;
        {
            colorAttachment.nextInChain     = nullptr;
            colorAttachment.view            = framebuffer.colorTextureView;
            colorAttachment.depthSlice      = 0;
            colorAttachment.resolveTarget   = nullptr;
            colorAttachment.loadOp          = WGPULoadOp_Undefined;
            colorAttachment.storeOp         = WGPUStoreOp_Store;
            colorAttachment.clearValue.r    = 0.0;
            colorAttachment.clearValue.g    = 0.0;
            colorAttachment.clearValue.b    = 0.0;
            colorAttachment.clearValue.a    = 0.0;
        };

        LLGL_ASSERT(renderPassEncoder_ == nullptr);
        WGPURenderPassDescriptor renderPassDesc;
        {
            renderPassDesc.nextInChain              = nullptr;
            renderPassDesc.label                    = WGPU_STRING_VIEW_INIT;
            renderPassDesc.colorAttachmentCount     = 1;
            renderPassDesc.colorAttachments         = &colorAttachment;
            renderPassDesc.depthStencilAttachment   = nullptr;
            renderPassDesc.occlusionQuerySet        = nullptr;
            renderPassDesc.timestampWrites          = nullptr;
        }
        renderPassEncoder_ = wgpuCommandEncoderBeginRenderPass(commandEncoder_, &renderPassDesc);
    }
    else
    {
        LLGL_TRAP_NOT_IMPLEMENTED();
    }
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
    auto& pipelineStateWG = LLGL_CAST(WGPipelineState&, pipelineState);
    if (pipelineStateWG.IsRenderPipeline())
    {
        auto& renderPipelineWG = LLGL_CAST(WGRenderPipeline&, pipelineState);
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder_, renderPipelineWG.GetNative());
    }
    else
    {
        auto& computePipelineWG = LLGL_CAST(WGComputePipeline&, pipelineState);
        EnsureComputeEncoder();
        wgpuComputePassEncoderSetPipeline(computePassEncoder_, computePipelineWG.GetNative());
    }
}

void WGCommandBuffer::SetBlendFactor(const float color[4])
{
    if (renderPassEncoder_ != nullptr)
    {
        WGPUColor blendColor = { color[0], color[1], color[2], color[3] };
        wgpuRenderPassEncoderSetBlendConstant(renderPassEncoder_, &blendColor);
    }
}

void WGCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    if (renderPassEncoder_ != nullptr)
        wgpuRenderPassEncoderSetStencilReference(renderPassEncoder_, reference);
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
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, 1, firstVertex, 0);
}

void WGCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, 1, firstIndex, 0, 0);
}

void WGCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, 1, firstIndex, vertexOffset, 0);
}

void WGCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, numInstances, firstVertex, 0);
}

void WGCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDraw(renderPassEncoder_, numVertices, numInstances, firstVertex, firstInstance);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, 0, 0);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void WGCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexed(renderPassEncoder_, numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void WGCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndirect(renderPassEncoder_, bufferWG.GetNative(), offset);
}

void WGCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    FlushRenderEncoderStates();
    while (numCommands-- > 0)
    {
        wgpuRenderPassEncoderDrawIndirect(renderPassEncoder_, bufferWG.GetNative(), offset);
        offset += stride;
    }
}

void WGCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    FlushRenderEncoderStates();
    wgpuRenderPassEncoderDrawIndexedIndirect(renderPassEncoder_, bufferWG.GetNative(), offset);
}

void WGCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    FlushRenderEncoderStates();
    while (numCommands-- > 0)
    {
        wgpuRenderPassEncoderDrawIndexedIndirect(renderPassEncoder_, bufferWG.GetNative(), offset);
        offset += stride;
    }
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
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    EnsureComputeEncoder();
    wgpuComputePassEncoderDispatchWorkgroupsIndirect(computePassEncoder_, bufferWG.GetNative(), offset);
}

void WGCommandBuffer::PushDebugGroup(const char* name)
{
    wgpuCommandEncoderPushDebugGroup(commandEncoder_, ToWGStringView(name));
}

void WGCommandBuffer::PopDebugGroup()
{
    wgpuCommandEncoderPopDebugGroup(commandEncoder_);
}

void WGCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool WGCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(WebGPU::CommandBufferNativeHandle))
    {
        auto* nativeHandleWG = static_cast<WebGPU::CommandBufferNativeHandle*>(nativeHandle);
        nativeHandleWG->commandEncoder      = commandEncoder_;
        nativeHandleWG->renderPassEncoder   = renderPassEncoder_;
        nativeHandleWG->computePassEncoder  = computePassEncoder_;
        return true;
    }
    return false;
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

void WGCommandBuffer::FlushRenderEncoderStates()
{
    if (renderDirtyBits_ != 0)
    {
        if ((renderDirtyBits_ & DirtyBit_Viewports) != 0)
        {
            wgpuRenderPassEncoderSetViewport(
                renderPassEncoder_,
                renderEncoderState_.viewport.x,
                renderEncoderState_.viewport.y,
                renderEncoderState_.viewport.width,
                renderEncoderState_.viewport.height,
                renderEncoderState_.viewport.minDepth,
                renderEncoderState_.viewport.maxDepth
            );
        }
        if ((renderDirtyBits_ & DirtyBit_Scissors) != 0)
        {
            wgpuRenderPassEncoderSetScissorRect(
                renderPassEncoder_,
                static_cast<std::uint32_t>(renderEncoderState_.scissor.x),
                static_cast<std::uint32_t>(renderEncoderState_.scissor.y),
                static_cast<std::uint32_t>(renderEncoderState_.scissor.width),
                static_cast<std::uint32_t>(renderEncoderState_.scissor.height)
            );
        }
        if ((renderDirtyBits_ & DirtyBit_VertexBuffers) != 0)
        {
            for_range(slot, renderEncoderState_.vertexBufferCount)
            {
                wgpuRenderPassEncoderSetVertexBuffer(
                    renderPassEncoder_,
                    slot,
                    renderEncoderState_.vertexBuffers[slot],
                    0,
                    WGPU_WHOLE_SIZE
                );
            }
        }
        renderDirtyBits_ = 0;
    }
}

void WGCommandBuffer::ResetRenderStates()
{
    renderDirtyBits_    = 0;
    computeDirtyBits_   = 0;
}


} // /namespace LLGL



// ================================================================================
