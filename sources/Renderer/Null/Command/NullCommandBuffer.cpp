/*
 * NullCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullCommandBuffer.h"
#include "NullCommandExecutor.h"
#include "NullCommand.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/TypeInfo.h>

#include "../NullSwapChain.h"
#include "../Buffer/NullBuffer.h"
#include "../Buffer/NullBufferArray.h"
#include "../RenderState/NullQueryHeap.h"
#include "../RenderState/NullPipelineState.h"
#include "../RenderState/NullResourceHeap.h"
#include "../Texture/NullTexture.h"
#include "../Texture/NullRenderTarget.h"

#include <LLGL/RenderingDebugger.h>
#include <LLGL/IndirectArguments.h>


namespace LLGL
{


NullCommandBuffer::NullCommandBuffer(const CommandBufferDescriptor& desc) :
    desc { desc }
{
}

/* ----- Encoding ----- */

void NullCommandBuffer::Begin()
{
    buffer_.Clear();
}

void NullCommandBuffer::End()
{
    if ((desc.flags & CommandBufferFlags::ImmediateSubmit) != 0)
        ExecuteVirtualCommands();
}

void NullCommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    auto& secondaryCommandBufferNull = LLGL_CAST(NullCommandBuffer&, secondaryCommandBuffer);
    if ((secondaryCommandBufferNull.desc.flags & CommandBufferFlags::Secondary) != 0)
        secondaryCommandBufferNull.ExecuteVirtualCommands();
}

/* ----- Blitting ----- */

void NullCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto dstBufferNull = LLGL_CAST(NullBuffer*, &dstBuffer);
    auto cmd = AllocCommand<NullCmdBufferWrite>(NullOpcodeBufferWrite, dataSize);
    {
        cmd->buffer = dstBufferNull;
        cmd->offset = static_cast<std::size_t>(dstOffset);
        cmd->size   = dataSize;
        ::memcpy(cmd + 1, data, dataSize);
    }
}

void NullCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto cmd = AllocCommand<NullCmdCopySubresource>(NullOpcodeCopySubresource);
    {
        cmd->srcResource    = &srcBuffer;
        cmd->srcSubresource = 0;
        cmd->srcX           = srcOffset;
        cmd->srcY           = 0;
        cmd->srcZ           = 0;
        cmd->dstResource    = &dstBuffer;
        cmd->dstSubresource = 0;
        cmd->dstX           = dstOffset;
        cmd->dstY           = 0;
        cmd->dstZ           = 0;
        cmd->width          = size;
        cmd->height         = 1;
        cmd->depth          = 1;
        cmd->rowStride      = 0;
        cmd->layerStride    = 0;
    }
}

static Extent3D GetSubresourceExtent(TextureType type, const Extent3D& extent, std::uint32_t numArrayLayers)
{
    switch (type)
    {
        case TextureType::Texture1DArray:
            return Extent3D{ extent.width, numArrayLayers, 1 };
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMSArray:
            return Extent3D{ extent.width, extent.height, numArrayLayers };
        default:
            return extent;
    }
}

void NullCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& srcTextureNull = LLGL_CAST(NullTexture&, srcTexture);
    const auto extent = GetSubresourceExtent(srcTextureNull.GetType(), srcRegion.extent, srcRegion.subresource.numArrayLayers);
    auto cmd = AllocCommand<NullCmdCopySubresource>(NullOpcodeCopySubresource);
    {
        cmd->srcResource    = &srcTextureNull;
        cmd->srcSubresource = srcTextureNull.PackSubresourceIndex(srcRegion.subresource.baseMipLevel, srcRegion.subresource.baseArrayLayer);
        cmd->srcX           = srcRegion.offset.x;
        cmd->srcY           = srcRegion.offset.y;
        cmd->srcZ           = srcRegion.offset.z;
        cmd->dstResource    = &dstBuffer;
        cmd->dstSubresource = 0;
        cmd->dstX           = dstOffset;
        cmd->dstY           = 0;
        cmd->dstZ           = 0;
        cmd->width          = extent.width;
        cmd->height         = extent.height;
        cmd->depth          = extent.depth;
        cmd->rowStride      = rowStride;
        cmd->layerStride    = layerStride;
    }
}

void NullCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    //auto& dstBufferNull = LLGL_CAST(NullBuffer&, dstBuffer);
    //todo
}

void NullCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureNull = LLGL_CAST(NullTexture&, dstTexture);
    auto& srcTextureNull = LLGL_CAST(NullTexture&, srcTexture);
    auto cmd = AllocCommand<NullCmdCopySubresource>(NullOpcodeCopySubresource);
    {
        cmd->srcResource    = &srcTextureNull;
        cmd->srcSubresource = srcTextureNull.PackSubresourceIndex(srcLocation.mipLevel, srcLocation.arrayLayer);
        cmd->srcX           = srcLocation.offset.x;
        cmd->srcY           = srcLocation.offset.y;
        cmd->srcZ           = srcLocation.offset.z;
        cmd->dstResource    = &dstTextureNull;
        cmd->dstSubresource = dstTextureNull.PackSubresourceIndex(srcLocation.mipLevel, srcLocation.arrayLayer);
        cmd->dstX           = dstLocation.offset.x;
        cmd->dstY           = dstLocation.offset.y;
        cmd->dstZ           = dstLocation.offset.z;
        cmd->width          = extent.width;
        cmd->height         = extent.height;
        cmd->depth          = extent.depth;
        cmd->rowStride      = 0;
        cmd->layerStride    = 0;
    }
}

void NullCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureNull = LLGL_CAST(NullTexture&, dstTexture);
    const auto extent = GetSubresourceExtent(dstTextureNull.GetType(), dstRegion.extent, dstRegion.subresource.numArrayLayers);
    auto cmd = AllocCommand<NullCmdCopySubresource>(NullOpcodeCopySubresource);
    {
        cmd->srcResource    = &srcBuffer;
        cmd->srcSubresource = 0;
        cmd->srcX           = srcOffset;
        cmd->srcY           = 0;
        cmd->srcZ           = 0;
        cmd->dstResource    = &dstTextureNull;
        cmd->dstSubresource = dstTextureNull.PackSubresourceIndex(dstRegion.subresource.baseMipLevel, dstRegion.subresource.baseArrayLayer);
        cmd->dstX           = dstRegion.offset.x;
        cmd->dstY           = dstRegion.offset.y;
        cmd->dstZ           = dstRegion.offset.z;
        cmd->width          = extent.width;
        cmd->height         = extent.height;
        cmd->depth          = extent.depth;
        cmd->rowStride      = rowStride;
        cmd->layerStride    = layerStride;
    }
}

void NullCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    //todo
}

void NullCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    auto cmd = AllocCommand<NullCmdGenerateMips>(NullOpcodeGenerateMips);
    {
        cmd->texture        = &textureNull;
        cmd->baseArrayLayer = 0;
        cmd->numArrayLayers = textureNull.desc.arrayLayers;
        cmd->baseMipLevel   = 0;
        cmd->numMipLevels   = textureNull.desc.mipLevels;
    }
}

void NullCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    auto cmd = AllocCommand<NullCmdGenerateMips>(NullOpcodeGenerateMips);
    {
        cmd->texture        = &textureNull;
        cmd->baseArrayLayer = subresource.baseArrayLayer;
        cmd->numArrayLayers = subresource.numArrayLayers;
        cmd->baseMipLevel   = subresource.baseMipLevel;
        cmd->numMipLevels   = subresource.numMipLevels;
    }
}

/* ----- Viewport and Scissor ----- */

void NullCommandBuffer::SetViewport(const Viewport& viewport)
{
    renderState_.viewports = { viewport };
}

void NullCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    renderState_.viewports = SmallVector<Viewport>(viewports, viewports + numViewports);
}

void NullCommandBuffer::SetScissor(const Scissor& scissor)
{
    renderState_.scissors = { scissor };
}

void NullCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    renderState_.scissors = SmallVector<Scissor>(scissors, scissors + numScissors);
}

/* ----- Buffers ------ */

void NullCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    renderState_.vertexBuffers = { &bufferNull };
}

void NullCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayNull = LLGL_CAST(NullBufferArray&, bufferArray);
    renderState_.vertexBuffers = SmallVector<const NullBuffer*>(bufferArrayNull.buffers.begin(), bufferArrayNull.buffers.end());
}

void NullCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    renderState_.indexBuffer        = &bufferNull;
    renderState_.indexBufferFormat  = bufferNull.desc.format;
    renderState_.indexBufferOffset  = 0;
}

void NullCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    renderState_.indexBuffer        = &bufferNull;
    renderState_.indexBufferFormat  = format;
    renderState_.indexBufferOffset  = offset;
}

/* ----- Resources ----- */

void NullCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    //auto& resourceHeapNull = LLGL_CAST(NullResourceHeap&, resourceHeap);
    //todo
}

void NullCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    //todo
}

void NullCommandBuffer::ResourceBarrier(
    std::uint32_t       /*numBuffers*/,
    Buffer* const *     /*buffers*/,
    std::uint32_t       /*numTextures*/,
    Texture* const *    /*textures*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void NullCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        //auto& swapChainNull = LLGL_CAST(NullSwapChain&, renderTarget);
        //todo
    }
    else
    {
        //auto& renderTargetNull = LLGL_CAST(NullRenderTarget&, renderTarget);
        //todo
    }
}

void NullCommandBuffer::EndRenderPass()
{
    //todo
}

void NullCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    //todo
}

void NullCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //todo
}

/* ----- Pipeline States ----- */

void NullCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    //auto& pipelineStateNull = LLGL_CAST(NullPipelineState&, pipelineState);
    //todo
}

void NullCommandBuffer::SetBlendFactor(const float color[4])
{
    //todo
}

void NullCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    //todo
}

void NullCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    //todo
}

/* ----- Queries ----- */

void NullCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
}

void NullCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
}

void NullCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
}

void NullCommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Stream Output ------ */

void NullCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    // dummy
}

void NullCommandBuffer::EndStreamOutput()
{
    // dummy
}

/* ----- Drawing ----- */

void NullCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    DrawIndirectArguments drawArgs;
    {
        drawArgs.numVertices    = numVertices;
        drawArgs.numInstances   = 1;
        drawArgs.firstVertex    = firstVertex;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    DrawIndexedIndirectArguments drawArgs;
    {
        drawArgs.numIndices     = numIndices;
        drawArgs.numInstances   = 1;
        drawArgs.firstIndex     = firstIndex;
        drawArgs.vertexOffset   = 0;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    DrawIndexedIndirectArguments drawArgs;
    {
        drawArgs.numIndices     = numIndices;
        drawArgs.numInstances   = 1;
        drawArgs.firstIndex     = firstIndex;
        drawArgs.vertexOffset   = vertexOffset;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    DrawIndirectArguments drawArgs;
    {
        drawArgs.numVertices    = numVertices;
        drawArgs.numInstances   = numInstances;
        drawArgs.firstVertex    = firstVertex;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawCommand(drawArgs);
}

void NullCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    DrawIndirectArguments drawArgs;
    {
        drawArgs.numVertices    = numVertices;
        drawArgs.numInstances   = numInstances;
        drawArgs.firstVertex    = firstVertex;
        drawArgs.firstInstance  = firstInstance;
    }
    AllocDrawCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    DrawIndexedIndirectArguments drawArgs;
    {
        drawArgs.numIndices     = numIndices;
        drawArgs.numInstances   = numInstances;
        drawArgs.firstIndex     = firstIndex;
        drawArgs.vertexOffset   = 0;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    DrawIndexedIndirectArguments drawArgs;
    {
        drawArgs.numIndices     = numIndices;
        drawArgs.numInstances   = numInstances;
        drawArgs.firstIndex     = firstIndex;
        drawArgs.vertexOffset   = vertexOffset;
        drawArgs.firstInstance  = 0;
    }
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    DrawIndexedIndirectArguments drawArgs;
    {
        drawArgs.numIndices     = numIndices;
        drawArgs.numInstances   = numInstances;
        drawArgs.firstIndex     = firstIndex;
        drawArgs.vertexOffset   = vertexOffset;
        drawArgs.firstInstance  = firstInstance;
    }
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    DrawIndirectArguments drawArgs;
    bufferNull.Read(offset, &drawArgs, sizeof(drawArgs));
    AllocDrawCommand(drawArgs);
}

void NullCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    DrawIndirectArguments drawArgs;
    while (numCommands-- > 0)
    {
        bufferNull.Read(offset, &drawArgs, sizeof(drawArgs));
        AllocDrawCommand(drawArgs);
        offset += stride;
    }
}

void NullCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    DrawIndexedIndirectArguments drawArgs;
    bufferNull.Read(offset, &drawArgs, sizeof(drawArgs));
    AllocDrawIndexedCommand(drawArgs);
}

void NullCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    DrawIndexedIndirectArguments drawArgs;
    while (numCommands-- > 0)
    {
        bufferNull.Read(offset, &drawArgs, sizeof(drawArgs));
        AllocDrawIndexedCommand(drawArgs);
        offset += stride;
    }
}

void NullCommandBuffer::DrawStreamOutput()
{
    // dummy
}

/* ----- Compute ----- */

void NullCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    // dummy
}

void NullCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    // dummy
}

/* ----- Debugging ----- */

void NullCommandBuffer::PushDebugGroup(const char* name)
{
    const std::size_t length = ::strlen(name);
    auto cmd = AllocCommand<NullCmdPushDebugGroup>(NullOpcodePushDebugGroup, length + 1);
    {
        ::memcpy(cmd + 1, name, length + 1);
    }
}

void NullCommandBuffer::PopDebugGroup()
{
    AllocOpcode(NullOpcodePopDebugGroup);
}

/* ----- Extensions ----- */

void NullCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool NullCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return (nativeHandle == nullptr || nativeHandleSize == 0); // dummy
}


/*
 * ======= Internal: =======
 */

void NullCommandBuffer::ExecuteVirtualCommands()
{
    ExecuteNullVirtualCommandBuffer(buffer_);
    if ((desc.flags & CommandBufferFlags::MultiSubmit) == 0)
        buffer_.Clear();
}


/*
 * ======= Private: =======
 */

void NullCommandBuffer::AllocOpcode(const NullOpcode opcode)
{
    buffer_.AllocOpcode(opcode);
}

template <typename TCommand>
TCommand* NullCommandBuffer::AllocCommand(const NullOpcode opcode, std::size_t payloadSize)
{
    return buffer_.AllocCommand<TCommand>(opcode, payloadSize);
}

void NullCommandBuffer::AllocDrawCommand(const DrawIndirectArguments& args)
{
    auto cmd = AllocCommand<NullCmdDraw>(NullOpcodeDraw, sizeof(const NullBuffer*) * renderState_.vertexBuffers.size());
    {
        cmd->args               = args;
        cmd->numVertexBuffers   = renderState_.vertexBuffers.size();
        ::memcpy(cmd + 1, renderState_.vertexBuffers.data(), sizeof(const NullBuffer*) * renderState_.vertexBuffers.size());
    }
}

void NullCommandBuffer::AllocDrawIndexedCommand(const DrawIndexedIndirectArguments& args)
{
    auto cmd = AllocCommand<NullCmdDrawIndexed>(NullOpcodeDrawIndexed, sizeof(const NullBuffer*) * renderState_.vertexBuffers.size());
    {
        cmd->args               = args;
        cmd->indexBuffer        = renderState_.indexBuffer;
        cmd->indexBufferFormat  = renderState_.indexBufferFormat;
        cmd->indexBufferOffset  = renderState_.indexBufferOffset;
        cmd->numVertexBuffers   = renderState_.vertexBuffers.size();
        ::memcpy(cmd + 1, renderState_.vertexBuffers.data(), sizeof(const NullBuffer*) * renderState_.vertexBuffers.size());
    }
}


} // /namespace LLGL



// ================================================================================
