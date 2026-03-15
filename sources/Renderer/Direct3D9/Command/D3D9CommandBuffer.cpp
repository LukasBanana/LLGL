/*
 * D3D9CommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9CommandBuffer.h"
#include "D3D9CommandExecutor.h"
#include "D3D9Command.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/ForRange.h>

#include "../D3D9SwapChain.h"

#include "../Buffer/D3D9VertexBuffer.h"
#include "../Buffer/D3D9IndexBuffer.h"
#include "../Buffer/D3D9BufferArray.h"

#include "../RenderState/D3D9ConstantsCache.h"
#include "../RenderState/D3D9QueryHeap.h"
#include "../RenderState/D3D9FixedFunctionPSO.h"
#include "../RenderState/D3D9ProgrammablePSO.h"
#include "../RenderState/D3D9PipelineLayout.h"
#include "../RenderState/D3D9ResourceHeap.h"
#include "../RenderState/D3D9StateManager.h"

#include "../Texture/D3D9Texture.h"
#include "../Texture/D3D9RenderTarget.h"
#include "../Texture/D3D9EmulatedSampler.h"

#include <LLGL/RenderingDebugger.h>
#include <LLGL/IndirectArguments.h>

#include <string.h>


namespace LLGL
{


D3D9CommandBuffer::D3D9CommandBuffer(D3D9StateManager* stateMngr, const CommandBufferDescriptor& desc) :
    device_    { stateMngr->GetDevice() },
    stateMngr_ { stateMngr              },
    desc       { desc                   }
{
}

/* ----- Encoding ----- */

void D3D9CommandBuffer::Begin()
{
    buffer_.Clear();

    if (IsPrimary())
        AllocOpcode(D3D9OpcodeBeginScene);
}

void D3D9CommandBuffer::End()
{
    if (IsPrimary())
        AllocOpcode(D3D9OpcodeEndScene);

    if (IsImmediateSubmit())
        ExecuteVirtualCommands();

    ResetRenderStates();
}

void D3D9CommandBuffer::Execute(CommandBuffer& secondaryCommandBuffer)
{
    if (IsPrimary())
    {
        auto& secondaryCommandBufferD3D9 = LLGL_CAST(D3D9CommandBuffer&, secondaryCommandBuffer);
        if (!secondaryCommandBufferD3D9.IsPrimary())
        {
            auto cmd = AllocCommand<D3D9CmdExecute>(D3D9OpcodeExecute);
            cmd->vcmdBuffer = secondaryCommandBufferD3D9.GetVcmdBuffer();
        }
    }
}

/* ----- Blitting ----- */

void D3D9CommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint64_t   dataSize)
{
    LLGL_ASSERT(dstOffset + dataSize <= UINT32_MAX, "D3D9 buffer range out of bounds: [%u, %u+%u)", dstOffset, dstOffset, dataSize);
    auto dstBufferD3D9 = LLGL_CAST(D3D9Buffer*, &dstBuffer);
    const std::size_t length = static_cast<std::size_t>(dataSize);
    auto cmd = AllocCommand<D3D9CmdBufferWrite>(D3D9OpcodeBufferWrite, length);
    {
        cmd->dstBuffer  = dstBufferD3D9;
        cmd->dstOffset  = static_cast<UINT>(dstOffset);
        cmd->dataSize   = static_cast<UINT>(length);
        ::memcpy(cmd + 1, data, length);
    }
}

void D3D9CommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    //todo
}

void D3D9CommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& srcTextureD3D9 = LLGL_CAST(D3D9Texture&, srcTexture);
    //todo
}

void D3D9CommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    //auto& dstBufferD3D9 = LLGL_CAST(D3D9Buffer&, dstBuffer);
    //todo
}

void D3D9CommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureD3D9 = LLGL_CAST(D3D9Texture&, dstTexture);
    auto& srcTextureD3D9 = LLGL_CAST(D3D9Texture&, srcTexture);
    //todo
}

void D3D9CommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureD3D9 = LLGL_CAST(D3D9Texture&, dstTexture);
    //todo
}

void D3D9CommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    //todo
}

void D3D9CommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureD3D = LLGL_CAST(D3D9Texture&, texture);
    auto cmd = AllocCommand<D3D9CmdGenerateMips>(D3D9OpcodeGenerateMips);
    cmd->texture = textureD3D.GetNative();
}

void D3D9CommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    //todo
}

/* ----- Viewport and Scissor ----- */

void D3D9CommandBuffer::SetViewport(const Viewport& viewport)
{
    auto cmd = AllocCommand<D3D9CmdSetViewport>(D3D9OpcodeSetViewport);
    {
        cmd->viewport.X         = static_cast<DWORD>(viewport.x);
        cmd->viewport.Y         = static_cast<DWORD>(viewport.y);
        cmd->viewport.Width     = static_cast<DWORD>(viewport.width);
        cmd->viewport.Height    = static_cast<DWORD>(viewport.height);
        cmd->viewport.MinZ      = viewport.minDepth;
        cmd->viewport.MaxZ      = viewport.maxDepth;
    }
}

void D3D9CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    LLGL_ASSERT(numViewports == 1, "D3D9 backend only supports a single viewport, but %u are specified", numViewports);
    SetViewport(viewports[0]);
}

void D3D9CommandBuffer::SetScissor(const Scissor& scissor)
{
    auto cmd = AllocCommand<D3D9CmdSetScissorRect>(D3D9OpcodeSetScissorRect);
    {
        cmd->scissorRect.left   = static_cast<LONG>(scissor.x);
        cmd->scissorRect.top    = static_cast<LONG>(scissor.y);
        cmd->scissorRect.right  = static_cast<LONG>(scissor.x + scissor.width);
        cmd->scissorRect.bottom = static_cast<LONG>(scissor.y + scissor.height);
    }
}

void D3D9CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    LLGL_ASSERT(numScissors == 1, "D3D9 backend only supports a single scissor rectangle, but %u are specified", numScissors);
    SetScissor(scissors[0]);
}

/* ----- Buffers ------ */

void D3D9CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    LLGL_ASSERT((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0);
    auto& vertexBufferD3D9 = LLGL_CAST(D3D9VertexBuffer&, buffer);
    AllocSetStreamSourceCommand(0, vertexBufferD3D9.GetNative(), vertexBufferD3D9.GetStride(), 0);
}

void D3D9CommandBuffer::SetVertexBuffer(Buffer& buffer, std::uint32_t numVertexAttribs, const VertexAttribute* vertexAttribs)
{
    LLGL_ASSERT((buffer.GetBindFlags() & BindFlags::VertexBuffer) != 0);
    if (numVertexAttribs > 0 && vertexAttribs != nullptr)
    {
        auto& vertexBufferD3D9 = LLGL_CAST(D3D9VertexBuffer&, buffer);
        AllocSetStreamSourceCommand(0, vertexBufferD3D9.GetNative(), vertexAttribs[0].stride, 0);
    }
}

void D3D9CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    D3D9BufferArray& bufferArrayD3D = LLGL_CAST(D3D9BufferArray&, bufferArray);
    for_range(i, static_cast<UINT>(bufferArrayD3D.GetNativeBuffersAndStrides().size()))
    {
        const D3D9BufferArray::D3DBufferAndStride& entry = bufferArrayD3D.GetNativeBuffersAndStrides()[i];
        AllocSetStreamSourceCommand(i, entry.vertexBuffer, entry.stride, 0);
    }
}

void D3D9CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    LLGL_ASSERT((buffer.GetBindFlags() & BindFlags::IndexBuffer) != 0);
    auto& indexBufferD3D9 = LLGL_CAST(D3D9IndexBuffer&, buffer);

    auto cmd = AllocCommand<D3D9CmdSetIndices>(D3D9OpcodeSetIndices);
    {
        cmd->indexBuffer = indexBufferD3D9.GetNative();
    }

    renderState_.indexBufferOffset = 0;
}

void D3D9CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    LLGL_ASSERT((buffer.GetBindFlags() & BindFlags::IndexBuffer) != 0);
    auto& indexBufferD3D9 = LLGL_CAST(D3D9IndexBuffer&, buffer);

    auto cmd = AllocCommand<D3D9CmdSetIndices>(D3D9OpcodeSetIndices);
    {
        cmd->indexBuffer = indexBufferD3D9.GetNative();
    }

    const std::uint32_t formatSize = GetFormatAttribs(format).bitSize / 8;
    renderState_.indexBufferOffset = (formatSize > 0 ? static_cast<UINT>(offset / formatSize) : 0);
}

/* ----- Resources ----- */

void D3D9CommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    //auto& resourceHeapD3D9 = LLGL_CAST(D3D9ResourceHeap&, resourceHeap);
    //todo
}

void D3D9CommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    if (boundPipelineLayout_ == nullptr)
        return;

    const D3D9ResourceBindingTable& bindingTable = boundPipelineLayout_->GetResourceBindingTable();
    if (descriptor >= bindingTable.resourceBindings.size())
        return;

    const D3D9ResourceBinding& resourceBinding = bindingTable.resourceBindings[descriptor];
    if (resourceBinding.type != resource.GetResourceType())
        return;

    if (resourceBinding.combiners > 0)
    {
        const DWORD* stages = &(boundPipelineLayout_->GetCombinedSamplerStages()[resourceBinding.stage]);
        SetCombinedResource(resourceBinding, resource, stages);
    }
    else
        SetSingleResource(resourceBinding.type, resourceBinding.stage, resource);
}

void D3D9CommandBuffer::ResourceBarrier(
    std::uint32_t       /*numBuffers*/,
    Buffer* const *     /*buffers*/,
    std::uint32_t       /*numTextures*/,
    Texture* const *    /*textures*/)
{
    // dummy
}

/* ----- Render Passes ----- */

void D3D9CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       /*swapBufferIndex*/)
{
    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainD3D9 = LLGL_CAST(D3D9SwapChain&, renderTarget);
        const UINT numBackBuffers = swapChainD3D9.GetNumBackBuffers();
        auto cmd = AllocCommand<D3D9CmdSetRenderTargets>(D3D9OpcodeSetRenderTargets, sizeof(IDirect3DSurface9*) * numBackBuffers);
        {
            cmd->count                  = numBackBuffers;
            cmd->depthStencilSurface    = swapChainD3D9.GetDepthStencilSurface();

            IDirect3DSurface9** renderTargets = reinterpret_cast<IDirect3DSurface9**>(cmd + 1);
            for_range(i, numBackBuffers)
                renderTargets[i] = swapChainD3D9.GetBackBufferSurface(i);
        }
    }
    else
    {
        //auto& renderTargetD3D9 = LLGL_CAST(D3D9RenderTarget&, renderTarget);
        //todo
    }
}

void D3D9CommandBuffer::EndRenderPass()
{
    //todo
}

static DWORD ToD3DClearFlags(long flags)
{
    DWORD d3dFlags = 0;

    if ((flags & ClearFlags::Color) != 0)
        d3dFlags |= D3DCLEAR_TARGET;
    if ((flags & ClearFlags::Depth) != 0)
        d3dFlags |= D3DCLEAR_ZBUFFER;
    if ((flags & ClearFlags::Stencil) != 0)
        d3dFlags |= D3DCLEAR_STENCIL;

    return d3dFlags;
}

void D3D9CommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if ((flags & ClearFlags::All) != 0)
    {
        auto cmd = AllocCommand<D3D9CmdClear>(D3D9OpcodeClear);
        {
            cmd->flags      = ToD3DClearFlags(flags);
            cmd->color      = D3DCOLOR_COLORVALUE(clearValue.color[0], clearValue.color[1], clearValue.color[2], clearValue.color[3]);
            cmd->z          = clearValue.depth;
            cmd->stencil    = clearValue.stencil;
        }
    }
}

void D3D9CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //todo
}

/* ----- Pipeline States ----- */

void D3D9CommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto& pipelineStateD3D = LLGL_CAST(D3D9PipelineState&, pipelineState);
    {
        auto cmd = AllocCommand<D3D9CmdSetPipelineState>(D3D9OpcodeSetPipelineState);
        cmd->pipelineState = &pipelineStateD3D;
    }

    if (pipelineStateD3D.IsProgrammablePipeline())
    {
        auto& programmablePsoD3D9 = LLGL_CAST(D3D9ProgrammablePSO&, pipelineStateD3D);

        /* Bind constants cache and invalidate after settings a new vertex and pixel shader */
        boundConstantsCache_ = programmablePsoD3D9.GetConstantsCache();
        if (boundConstantsCache_ != nullptr)
            boundConstantsCache_->Invalidate();

        SetStreamSourceFreqInstanceData(programmablePsoD3D9.GetStreamSourceFreq());
    }
    else
    {
        boundConstantsCache_ = nullptr;
        SetStreamSourceFreqInstanceData({});
    }

    /* Cache pipeline render states only used for current command encoding */
    renderState_.primitiveType = pipelineStateD3D.GetPrimitiveType();
    boundPipelineLayout_ = pipelineStateD3D.GetPipelineLayout();
}

void D3D9CommandBuffer::SetBlendFactor(const float color[4])
{
    //todo
}

void D3D9CommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    //todo
}

void D3D9CommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->SetUniforms(first, data, dataSize);
}

/* ----- Queries ----- */

void D3D9CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //auto& queryHeapD3D9 = LLGL_CAST(D3D9QueryHeap&, queryHeap);
    //todo
}

void D3D9CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    //auto& queryHeapD3D9 = LLGL_CAST(D3D9QueryHeap&, queryHeap);
    //todo
}

void D3D9CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    //auto& queryHeapD3D9 = LLGL_CAST(D3D9QueryHeap&, queryHeap);
    //todo
}

void D3D9CommandBuffer::EndRenderCondition()
{
    //todo
}

/* ----- Stream Output ------ */

void D3D9CommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    // dummy
}

void D3D9CommandBuffer::EndStreamOutput()
{
    // dummy
}

/* ----- Drawing ----- */

void D3D9CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    AllocDrawCommand(firstVertex, numVertices);
}

void D3D9CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    AllocDrawIndexedCommand(0, 0, numIndices, firstIndex);
}

void D3D9CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    AllocDrawIndexedCommand(vertexOffset, 0, numIndices, firstIndex);
}

void D3D9CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    AllocDrawCommand(firstVertex, numVertices, numInstances);
}

void D3D9CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    LLGL_ASSERT(firstInstance == 0, "D3D9 does not support instance offset");
    AllocDrawCommand(firstVertex, numVertices, numInstances);
}

void D3D9CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    AllocDrawIndexedCommand(0, 0, numIndices, firstIndex, numInstances);
}

void D3D9CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    AllocDrawIndexedCommand(vertexOffset, 0, numIndices, firstIndex, numInstances);
}

void D3D9CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    LLGL_ASSERT(firstInstance == 0, "D3D9 does not support instance offset");
    AllocDrawIndexedCommand(vertexOffset, 0, numIndices, firstIndex, numInstances);
}

void D3D9CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("indirect instanced draw commands");
}

void D3D9CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("indirect instanced draw commands");
}

void D3D9CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("indexed-indirect instanced draw commands");
}

void D3D9CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("indexed-indirect instanced draw commands");
}

void D3D9CommandBuffer::DrawStreamOutput()
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("stream-output draw commands");
}

/* ----- Compute ----- */

void D3D9CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("compute commands");
}

void D3D9CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("indirect compute commands");
}

/* ----- Debugging ----- */

void D3D9CommandBuffer::PushDebugGroup(const char* name)
{
    // dummy
}

void D3D9CommandBuffer::PopDebugGroup()
{
    // dummy
}

/* ----- Extensions ----- */

void D3D9CommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    // dummy
}

bool D3D9CommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return (nativeHandle == nullptr || nativeHandleSize == 0); // dummy
}


/*
 * ======= Internal: =======
 */

void D3D9CommandBuffer::ExecuteVirtualCommands()
{
    ExecuteD3D9VirtualCommandBuffer(buffer_, stateMngr_);
    if ((desc.flags & CommandBufferFlags::MultiSubmit) == 0)
        buffer_.Clear();
}


/*
 * ======= Private: =======
 */

void D3D9CommandBuffer::ResetRenderStates()
{
    boundConstantsCache_ = nullptr;
    boundPipelineLayout_ = nullptr;
}

void D3D9CommandBuffer::AllocOpcode(const D3D9Opcode opcode)
{
    buffer_.AllocOpcode(opcode);
}

template <typename TCommand>
TCommand* D3D9CommandBuffer::AllocCommand(const D3D9Opcode opcode, std::size_t payloadSize)
{
    return buffer_.AllocCommand<TCommand>(opcode, payloadSize);
}

static UINT GetPrimitiveCount(D3DPRIMITIVETYPE primitiveType, UINT numVertices)
{
    switch (primitiveType)
    {
        case D3DPT_POINTLIST:
            return numVertices;

        case D3DPT_LINELIST:
            return numVertices / 2;

        case D3DPT_LINESTRIP:
            return (numVertices >= 2 ? numVertices - 1 : 0);

        case D3DPT_TRIANGLELIST:
            return numVertices / 3;

        case D3DPT_TRIANGLESTRIP:
        case D3DPT_TRIANGLEFAN:
            return (numVertices >= 3 ? numVertices - 2 : 0);
    }
    return 0;
}

void D3D9CommandBuffer::FlushConstantsCache()
{
    if (boundConstantsCache_ != nullptr)
        boundConstantsCache_->AllocCommands(buffer_);
}

void D3D9CommandBuffer::AllocSetStreamSourceCommand(UINT stream, IDirect3DVertexBuffer9* vertexBuffer, UINT stride, UINT offset)
{
    auto cmd = AllocCommand<D3D9CmdSetStreamSource>(D3D9OpcodeSetStreamSource);
    {
        cmd->stream         = stream;
        cmd->vertexBuffer   = vertexBuffer;
        cmd->stride         = stride;
        cmd->offset         = offset;
    }
}

void D3D9CommandBuffer::AllocDrawCommand(UINT startVertex, UINT numVertices, UINT numInstances)
{
    FlushConstantsCache();
    SetNumInstances(numInstances);
    auto cmd = AllocCommand<D3D9CmdDraw>(D3D9OpcodeDraw);
    {
        cmd->primitiveType  = renderState_.primitiveType;
        cmd->startVertex    = startVertex;
        cmd->primitiveCount = GetPrimitiveCount(renderState_.primitiveType, numVertices);
    }
}

void D3D9CommandBuffer::AllocDrawIndexedCommand(INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT numInstances)
{
    FlushConstantsCache();
    SetNumInstances(numInstances);
    auto cmd = AllocCommand<D3D9CmdDrawIndexed>(D3D9OpcodeDrawIndexed);
    {
        cmd->primitiveType      = renderState_.primitiveType;
        cmd->baseVertexIndex    = baseVertexIndex;
        cmd->minVertexIndex     = minVertexIndex;
        cmd->numVertices        = numVertices;
        cmd->startIndex         = startIndex + renderState_.indexBufferOffset;
        cmd->primitiveCount     = GetPrimitiveCount(renderState_.primitiveType, numVertices);
    }
}

D3D9CmdSetRenderStates::D3DRenderState* D3D9CommandBuffer::AllocSetRenderStatesCommand(UINT count)
{
    auto cmd = AllocCommand<D3D9CmdSetRenderStates>(D3D9OpcodeSetRenderStates, sizeof(D3D9CmdSetRenderStates::D3DRenderState) * count);
    cmd->numRenderStates = count;
    return reinterpret_cast<D3D9CmdSetRenderStates::D3DRenderState*>(cmd + 1);
}

void D3D9CommandBuffer::AllocSetStreamSourceFreqIndexDataCommand(UINT numInstances)
{
    auto cmd = AllocCommand<D3D9CmdSetStreamSourceFreqIndexData>(D3D9OpcodeSetStreamSourceFreqIndexData);
    cmd->numInstance = numInstances;
}

void D3D9CommandBuffer::AllocSetStreamSourceFreqInstanceDataCommand(ArrayView<D3D9StreamSourceFreq> streamSourceFreq)
{
    auto cmd = AllocCommand<D3D9CmdSetStreamSourceFreqInstanceData>(D3D9OpcodeSetStreamSourceFreqInstanceData, streamSourceFreq.size() * sizeof(D3D9StreamSourceFreq));
    cmd->count = static_cast<UINT>(streamSourceFreq.size());
    if (cmd->count > 0)
        ::memcpy(cmd + 1, streamSourceFreq.data(), streamSourceFreq.size() * sizeof(D3D9StreamSourceFreq));
}

void D3D9CommandBuffer::SetStreamSourceFreqInstanceData(ArrayView<D3D9StreamSourceFreq> streamSourceFreq)
{
    if (streamSourceFreq.empty())
    {
        /* Disable instanced drawing */
        if (renderState_.isInstancedVS)
        {
            renderState_.isInstancedVS = false;
            AllocSetStreamSourceFreqInstanceDataCommand({});
            AllocSetStreamSourceFreqIndexDataCommand(0);
        }
    }
    else
    {
        /* Enable instanced drawing */
        renderState_.isInstancedVS = true;
        AllocSetStreamSourceFreqInstanceDataCommand(streamSourceFreq);
    }
}

void D3D9CommandBuffer::SetNumInstances(UINT numInstances)
{
    if (renderState_.isInstancedVS)
        AllocSetStreamSourceFreqIndexDataCommand(numInstances);
}

void D3D9CommandBuffer::SetCombinedResource(const D3D9ResourceBinding& resourceBinding, Resource& resource, const DWORD* stages)
{
    for_range(i, resourceBinding.combiners)
        SetSingleResource(resourceBinding.type, stages[i], resource);
}

void D3D9CommandBuffer::SetSingleResource(ResourceType type, DWORD stage, Resource& resource)
{
    switch (type)
    {
        case ResourceType::Buffer:
        {
            //TODO
        }
        break;

        case ResourceType::Texture:
        {
            D3D9Texture& textureD3D = LLGL_CAST(D3D9Texture&, resource);
            auto cmd = AllocCommand<D3D9CmdBindTexture>(D3D9OpcodeBindTexture);
            cmd->stage = stage;
            cmd->texture = textureD3D.GetNative();
        }
        break;

        case ResourceType::Sampler:
        {
            D3D9EmulatedSampler& samplerD3D = LLGL_CAST(D3D9EmulatedSampler&, resource);
            auto cmd = AllocCommand<D3D9CmdBindSampler>(D3D9OpcodeBindSampler);
            cmd->stage = stage;
            cmd->emulatedSampler = &(samplerD3D);
        }
        break;

        default:
            break;
    }
}


} // /namespace LLGL



// ================================================================================
