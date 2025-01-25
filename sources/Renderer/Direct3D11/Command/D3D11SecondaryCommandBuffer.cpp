/*
 * D3D11SecondaryCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11SecondaryCommandBuffer.h"
#include "D3D11Command.h"
#include "../RenderState/D3D11ResourceHeap.h"
#include "../RenderState/D3D11PipelineState.h"
#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11BufferArray.h"
#include "../D3D11Types.h"
#include "../../CheckedCast.h"
#include <LLGL/IndirectArguments.h>
#include <cstring>


namespace LLGL
{


static constexpr std::size_t g_initialSizeForD3DVirtualCmdBuffer = 4096;

D3D11SecondaryCommandBuffer::D3D11SecondaryCommandBuffer(const CommandBufferDescriptor& /*desc*/)
:
    D3D11CommandBuffer { /*isSecondaryCmdBuffer:*/ true      },
    buffer_            { g_initialSizeForD3DVirtualCmdBuffer }
{
}

/* ----- Encoding ----- */

void D3D11SecondaryCommandBuffer::Begin()
{
    buffer_.Clear();
}

void D3D11SecondaryCommandBuffer::End()
{
    // dummy
}

void D3D11SecondaryCommandBuffer::Execute(CommandBuffer& /*secondaryCommandBuffer*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Blitting ----- */

void D3D11SecondaryCommandBuffer::UpdateBuffer(
    Buffer&         /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    const void*     /*data*/,
    std::uint16_t   /*dataSize*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::CopyBuffer(
    Buffer&         /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    Buffer&         /*srcBuffer*/,
    std::uint64_t   /*srcOffset*/,
    std::uint64_t   /*size*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::CopyBufferFromTexture(
    Buffer&                 /*dstBuffer*/,
    std::uint64_t           /*dstOffset*/,
    Texture&                /*srcTexture*/,
    const TextureRegion&    /*srcRegion*/,
    std::uint32_t           /*rowStride*/,
    std::uint32_t           /*layerStride*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::FillBuffer(
    Buffer&         /*dstBuffer*/,
    std::uint64_t   /*dstOffset*/,
    std::uint32_t   /*value*/,
    std::uint64_t   /*fillSize*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::CopyTexture(
    Texture&                /*dstTexture*/,
    const TextureLocation&  /*dstLocation*/,
    Texture&                /*srcTexture*/,
    const TextureLocation&  /*srcLocation*/,
    const Extent3D&         /*extent*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::CopyTextureFromBuffer(
    Texture&                /*dstTexture*/,
    const TextureRegion&    /*dstRegion*/,
    Buffer&                 /*srcBuffer*/,
    std::uint64_t           /*srcOffset*/,
    std::uint32_t           /*rowStride*/,
    std::uint32_t           /*layerStride*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                /*dstTexture*/,
    const TextureRegion&    /*dstRegion*/,
    const Offset2D&         /*srcOffset*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::GenerateMips(Texture& /*texture*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::GenerateMips(Texture& /*texture*/, const TextureSubresource& /*subresource*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Viewport and Scissor ----- */

void D3D11SecondaryCommandBuffer::SetViewport(const Viewport& /*viewport*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::SetViewports(std::uint32_t /*numViewports*/, const Viewport* /*viewports*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::SetScissor(const Scissor& /*scissor*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::SetScissors(std::uint32_t /*numScissors*/, const Scissor* /*scissors*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Buffers ------ */

void D3D11SecondaryCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto* bufferD3D = LLGL_CAST(D3D11Buffer*, &buffer);
    auto cmd = AllocCommand<D3D11CmdSetVertexBuffer>(D3D11OpcodeSetVertexBuffer);
    {
        cmd->buffer = bufferD3D;
    }
}

void D3D11SecondaryCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto* bufferArrayD3D = LLGL_CAST(D3D11BufferArray*, &bufferArray);
    auto cmd = AllocCommand<D3D11CmdSetVertexBufferArray>(D3D11OpcodeSetVertexBufferArray);
    {
        cmd->bufferArray = bufferArrayD3D;
    }
}

void D3D11SecondaryCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto* bufferD3D = LLGL_CAST(D3D11Buffer*, &buffer);
    auto cmd = AllocCommand<D3D11CmdSetIndexBuffer>(D3D11OpcodeSetIndexBuffer);
    {
        cmd->buffer = bufferD3D;
        cmd->format = bufferD3D->GetDXFormat();
        cmd->offset = 0;
    }
}

void D3D11SecondaryCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto* bufferD3D = LLGL_CAST(D3D11Buffer*, &buffer);
    auto cmd = AllocCommand<D3D11CmdSetIndexBuffer>(D3D11OpcodeSetIndexBuffer);
    {
        cmd->buffer = bufferD3D;
        cmd->format = DXTypes::ToDXGIFormat(format);
        cmd->offset = static_cast<UINT>(offset);
    }
}

/* ----- Resources ----- */

void D3D11SecondaryCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto* resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap*, &resourceHeap);
    auto cmd = AllocCommand<D3D11CmdSetResourceHeap>(D3D11OpcodeSetResourceHeap);
    {
        cmd->resourceHeap   = resourceHeapD3D;
        cmd->descriptorSet  = descriptorSet;
    }
}

void D3D11SecondaryCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    auto cmd = AllocCommand<D3D11CmdSetResource>(D3D11OpcodeSetResource);
    {
        cmd->descriptor = descriptor;
        cmd->resource   = &resource;
    }
}

void D3D11SecondaryCommandBuffer::ResourceBarrier(
    std::uint32_t       /*numBuffers*/,
    Buffer* const *     /*buffers*/,
    std::uint32_t       /*numTextures*/,
    Texture* const *    /*textures*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void D3D11SecondaryCommandBuffer::BeginRenderPass(
    RenderTarget&       /*renderTarget*/,
    const RenderPass*   /*renderPass*/,
    std::uint32_t       /*numClearValues*/,
    const ClearValue*   /*clearValues*/,
    std::uint32_t       /*swapBufferIndex*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::EndRenderPass()
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::Clear(long /*flags*/, const ClearValue& /*clearValue*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::ClearAttachments(std::uint32_t /*numAttachments*/, const AttachmentClear* /*attachments*/)
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Pipeline States ----- */

void D3D11SecondaryCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto* pipelineStateD3D = LLGL_CAST(D3D11PipelineState*, &pipelineState);
    auto cmd = AllocCommand<D3D11CmdSetPipelineState>(D3D11OpcodeSetPipelineState);
    {
        cmd->pipelineState = pipelineStateD3D;
    }
}

void D3D11SecondaryCommandBuffer::SetBlendFactor(const float color[4])
{
    auto cmd = AllocCommand<D3D11CmdSetBlendFactor>(D3D11OpcodeSetBlendFactor);
    {
        cmd->color[0] = color[0];
        cmd->color[1] = color[1];
        cmd->color[2] = color[2];
        cmd->color[3] = color[3];
    }
}

void D3D11SecondaryCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace /*stencilFace*/)
{
    auto cmd = AllocCommand<D3D11CmdSetStencilRef>(D3D11OpcodeSetStencilRef);
    {
        cmd->stencilRef = reference;
    }
}

void D3D11SecondaryCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    auto cmd = AllocCommand<D3D11CmdSetUniforms>(D3D11OpcodeSetUniforms, dataSize);
    {
        cmd->first      = first;
        cmd->dataSize   = dataSize;
        std::memcpy(cmd + 1, data, dataSize);
    }
}

/* ----- Queries ----- */

void D3D11SecondaryCommandBuffer::BeginQuery(QueryHeap& /*queryHeap*/, std::uint32_t /*query*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::EndQuery(QueryHeap& /*queryHeap*/, std::uint32_t /*query*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::BeginRenderCondition(QueryHeap& /*queryHeap*/, std::uint32_t /*query*/, const RenderConditionMode /*mode*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::EndRenderCondition()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Stream Output ------ */

void D3D11SecondaryCommandBuffer::BeginStreamOutput(std::uint32_t /*numBuffers*/, Buffer* const * /*buffers*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::EndStreamOutput()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Drawing ----- */

void D3D11SecondaryCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    auto cmd = AllocCommand<D3D11CmdDraw>(D3D11OpcodeDraw);
    {
        cmd->vertexCount            = numVertices;
        cmd->startVertexLocation    = firstVertex;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    auto cmd = AllocCommand<D3D11CmdDrawIndexed>(D3D11OpcodeDrawIndexed);
    {
        cmd->indexCount         = numIndices;
        cmd->startIndexLocation = firstIndex;
        cmd->baseVertexLocation = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto cmd = AllocCommand<D3D11CmdDrawIndexed>(D3D11OpcodeDrawIndexed);
    {
        cmd->indexCount         = numIndices;
        cmd->startIndexLocation = firstIndex;
        cmd->baseVertexLocation = vertexOffset;
    }
}

void D3D11SecondaryCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    auto cmd = AllocCommand<D3D11CmdDrawInstanced>(D3D11OpcodeDrawInstanced);
    {
        cmd->vertexCountPerInstance = numVertices;
        cmd->instanceCount          = numInstances;
        cmd->startVertexLocation    = firstVertex;
        cmd->startInstanceLocation  = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    auto cmd = AllocCommand<D3D11CmdDrawInstanced>(D3D11OpcodeDrawInstanced);
    {
        cmd->vertexCountPerInstance = numVertices;
        cmd->instanceCount          = numInstances;
        cmd->startVertexLocation    = firstVertex;
        cmd->startInstanceLocation  = firstInstance;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    auto cmd = AllocCommand<D3D11CmdDrawIndexedInstanced>(D3D11OpcodeDrawIndexedInstanced);
    {
        cmd->indexCountPerInstance  = numIndices;
        cmd->instanceCount          = numInstances;
        cmd->startIndexLocation     = firstIndex;
        cmd->baseVertexLocation     = 0;
        cmd->startInstanceLocation  = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    auto cmd = AllocCommand<D3D11CmdDrawIndexedInstanced>(D3D11OpcodeDrawIndexedInstanced);
    {
        cmd->indexCountPerInstance  = numIndices;
        cmd->instanceCount          = numInstances;
        cmd->startIndexLocation     = firstIndex;
        cmd->baseVertexLocation     = vertexOffset;
        cmd->startInstanceLocation  = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    auto cmd = AllocCommand<D3D11CmdDrawIndexedInstanced>(D3D11OpcodeDrawIndexedInstanced);
    {
        cmd->indexCountPerInstance  = numIndices;
        cmd->instanceCount          = numInstances;
        cmd->startIndexLocation     = firstIndex;
        cmd->baseVertexLocation     = vertexOffset;
        cmd->startInstanceLocation  = firstInstance;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto cmd = AllocCommand<D3D11CmdDrawInstancedIndirect>(D3D11OpcodeDrawInstancedIndirect);
    { 
        cmd->bufferForArgs              = bufferD3D.GetNative();
        cmd->alignedByteOffsetForArgs   = static_cast<UINT>(offset);
        cmd->numCommands                = 1;
        cmd->stride                     = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto cmd = AllocCommand<D3D11CmdDrawInstancedIndirect>(D3D11OpcodeDrawInstancedIndirectN);
    { 
        cmd->bufferForArgs              = bufferD3D.GetNative();
        cmd->alignedByteOffsetForArgs   = static_cast<UINT>(offset);
        cmd->numCommands                = numCommands;
        cmd->stride                     = stride;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto cmd = AllocCommand<D3D11CmdDrawInstancedIndirect>(D3D11OpcodeDrawIndexedInstancedIndirect);
    { 
        cmd->bufferForArgs              = bufferD3D.GetNative();
        cmd->alignedByteOffsetForArgs   = static_cast<UINT>(offset);
        cmd->numCommands                = 1;
        cmd->stride                     = 0;
    }
}

void D3D11SecondaryCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto cmd = AllocCommand<D3D11CmdDrawInstancedIndirect>(D3D11OpcodeDrawIndexedInstancedIndirectN);
    { 
        cmd->bufferForArgs              = bufferD3D.GetNative();
        cmd->alignedByteOffsetForArgs   = static_cast<UINT>(offset);
        cmd->numCommands                = numCommands;
        cmd->stride                     = stride;
    }
}

void D3D11SecondaryCommandBuffer::DrawStreamOutput()
{
    AllocOpcode(D3D11OpcodeDrawAuto);
}

/* ----- Compute ----- */

void D3D11SecondaryCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    auto cmd = AllocCommand<D3D11CmdDispatch>(D3D11OpcodeDispatch);
    {
        cmd->threadGroupCountX = numWorkGroupsX;
        cmd->threadGroupCountY = numWorkGroupsY;
        cmd->threadGroupCountZ = numWorkGroupsZ;
    }
}

void D3D11SecondaryCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto cmd = AllocCommand<D3D11CmdDispatchIndirect>(D3D11OpcodeDispatchIndirect);
    { 
        cmd->bufferForArgs              = bufferD3D.GetNative();
        cmd->alignedByteOffsetForArgs   = static_cast<UINT>(offset);
    }
}

/* ----- Debugging ----- */

void D3D11SecondaryCommandBuffer::PushDebugGroup(const char* /*name*/)
{
    // dummy - command not allowed in secondary command buffer
}

void D3D11SecondaryCommandBuffer::PopDebugGroup()
{
    // dummy - command not allowed in secondary command buffer
}

/* ----- Extensions ----- */

void D3D11SecondaryCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy - command not allowed in secondary command buffer
}

bool D3D11SecondaryCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return false; // dummy - command not allowed in secondary command buffer
}


/*
 * ======= Private: =======
 */

void D3D11SecondaryCommandBuffer::AllocOpcode(const D3D11Opcode opcode)
{
    buffer_.AllocOpcode(opcode);
}

template <typename TCommand>
TCommand* D3D11SecondaryCommandBuffer::AllocCommand(const D3D11Opcode opcode, std::size_t payloadSize)
{
    return buffer_.AllocCommand<TCommand>(opcode, payloadSize);
}



} // /namespace LLGL



// ================================================================================
