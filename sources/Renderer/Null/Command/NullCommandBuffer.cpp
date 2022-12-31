/*
 * NullCommandBuffer.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullCommandBuffer.h"
#include "NullCommandExecutor.h"
#include "NullCommand.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"
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

void NullCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& deferredCommandBufferNull = LLGL_CAST(NullCommandBuffer&, deferredCommandBuffer);
    if ((deferredCommandBufferNull.desc.flags & CommandBufferFlags::Secondary) != 0)
        deferredCommandBufferNull.ExecuteVirtualCommands();
}

/* ----- Blitting ----- */

void NullCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferNull = LLGL_CAST(NullBuffer&, dstBuffer);
    //todo
}

void NullCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferNull = LLGL_CAST(NullBuffer&, dstBuffer);
    auto& srcBufferNull = LLGL_CAST(NullBuffer&, srcBuffer);
    //todo
}

void NullCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferNull = LLGL_CAST(NullBuffer&, dstBuffer);
    auto& srcTextureNull = LLGL_CAST(NullTexture&, srcTexture);
    //todo
}

void NullCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferNull = LLGL_CAST(NullBuffer&, dstBuffer);
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
    //todo
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
    auto& srcBufferNull = LLGL_CAST(NullBuffer&, srcBuffer);
    //todo
}

void NullCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    //todo
}

void NullCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    //todo
}

/* ----- Viewport and Scissor ----- */

void NullCommandBuffer::SetViewport(const Viewport& viewport)
{
    //todo
}

void NullCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    //todo
}

void NullCommandBuffer::SetScissor(const Scissor& scissor)
{
    //todo
}

void NullCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    //tood
}

/* ----- Buffers ------ */

void NullCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    //todo
}

void NullCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayNull = LLGL_CAST(NullBufferArray&, bufferArray);
    //todo
}

void NullCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    //todo
}

void NullCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    //todo
}

/* ----- Resources ----- */

void NullCommandBuffer::SetResourceHeap(
    ResourceHeap&           resourceHeap,
    std::uint32_t           firstSet,
    const PipelineBindPoint bindPoint)
{
    auto& resourceHeapNull = LLGL_CAST(NullResourceHeap&, resourceHeap);
    //todo
}

void NullCommandBuffer::SetResource(
    Resource&       resource,
    std::uint32_t   slot,
    long            bindFlags,
    long            stageFlags)
{
    //todo
}

void NullCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{
    //todo
}

/* ----- Render Passes ----- */

void NullCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainNull = LLGL_CAST(NullSwapChain&, renderTarget);
        //todo
    }
    else
    {
        auto& renderTargetNull = LLGL_CAST(NullRenderTarget&, renderTarget);
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
    auto& pipelineStateNull = LLGL_CAST(NullPipelineState&, pipelineState);
    //todo
}

void NullCommandBuffer::SetBlendFactor(const ColorRGBAf& color)
{
    //todo
}

void NullCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    //todo
}

void NullCommandBuffer::SetUniform(
    UniformLocation location,
    const void*     data,
    std::uint32_t   dataSize)
{
    //todo
}

void NullCommandBuffer::SetUniforms(
    UniformLocation location,
    std::uint32_t   count,
    const void*     data,
    std::uint32_t   dataSize)
{
    //todo
}

/* ----- Queries ----- */

void NullCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
}

void NullCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
    //todo
}

void NullCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapNull = LLGL_CAST(NullQueryHeap&, queryHeap);
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
    // dummy
}

void NullCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    // dummy
}

void NullCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    // dummy
}

void NullCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    // dummy
}

void NullCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    // dummy
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    // dummy
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    // dummy
}

void NullCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    // dummy
}

void NullCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    // dummy
}

void NullCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    // dummy
}

void NullCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    // dummy
}

void NullCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
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

void NullCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
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


} // /namespace LLGL



// ================================================================================
