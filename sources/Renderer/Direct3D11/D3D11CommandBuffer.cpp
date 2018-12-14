/*
 * D3D11CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11CommandBuffer.h"
#include "D3D11RenderContext.h"
#include "D3D11Types.h"
#include "../CheckedCast.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"
#include <algorithm>

#include "RenderState/D3D11StateManager.h"
#include "RenderState/D3D11GraphicsPipelineBase.h"
#include "RenderState/D3D11ComputePipeline.h"
#include "RenderState/D3D11QueryHeap.h"
#include "RenderState/D3D11ResourceHeap.h"
#include "RenderState/D3D11RenderPass.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11BufferWithRV.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"


namespace LLGL
{


#define VS_STAGE(FLAG) ( ((FLAG) & StageFlags::VertexStage        ) != 0 )
#define HS_STAGE(FLAG) ( ((FLAG) & StageFlags::TessControlStage   ) != 0 )
#define DS_STAGE(FLAG) ( ((FLAG) & StageFlags::TessEvaluationStage) != 0 )
#define GS_STAGE(FLAG) ( ((FLAG) & StageFlags::GeometryStage      ) != 0 )
#define PS_STAGE(FLAG) ( ((FLAG) & StageFlags::FragmentStage      ) != 0 )
#define CS_STAGE(FLAG) ( ((FLAG) & StageFlags::ComputeStage       ) != 0 )


// Global array of null pointers to unbind resource slots
static void*    g_nullResources[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]   = {};
static UINT     g_zeroCounters[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT]       = {};

static bool HasBufferResourceViews(const Buffer& buffer)
{
    return ((buffer.GetBindFlags() & (BindFlags::SampleBuffer | BindFlags::RWStorageBuffer)) != 0);
}


D3D11CommandBuffer::D3D11CommandBuffer(
    const ComPtr<ID3D11DeviceContext>&          context,
    const std::shared_ptr<D3D11StateManager>&   stateMngr,
    const CommandBufferDescriptor&              desc)
:   context_   { context   },
    stateMngr_ { stateMngr }
{
    /* Store information whether the command buffer has an immediate or deferred context */
    if ((desc.flags & CommandBufferFlags::DeferredSubmit) != 0)
        hasDeferredContext_ = true;
}

/* ----- Encoding ----- */

void D3D11CommandBuffer::Begin()
{
    // dummy
}

void D3D11CommandBuffer::End()
{
    if (hasDeferredContext_)
    {
        /* Encode commands from deferred context into command list */
        context_->FinishCommandList(TRUE, commandList_.ReleaseAndGetAddressOf());
    }
}

void D3D11CommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    dstBufferD3D.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(dstOffset));
}

void D3D11CommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    auto& srcBufferD3D = LLGL_CAST(D3D11Buffer&, srcBuffer);

    context_->CopySubresourceRegion(
        dstBufferD3D.GetNative(),                               // pDstResource
        0,                                                      // DstSubresource
        static_cast<UINT>(dstOffset),                           // DstX
        0,                                                      // DstY
        0,                                                      // DstZ
        srcBufferD3D.GetNative(),                               // pSrcResource
        0,                                                      // SrcSubresource
        &CD3D11_BOX(                                            // pSrcBox
            static_cast<LONG>(srcOffset), 0, 0,
            static_cast<LONG>(srcOffset + size), 1, 1
        )
    );
}

void D3D11CommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& cmdBufferD3D = LLGL_CAST(D3D11CommandBuffer&, deferredCommandBuffer);
    if (auto commandList = cmdBufferD3D.commandList_.Get())
    {
        /* Execute encoded command list with immediate context */
        context_->ExecuteCommandList(commandList, TRUE);
    }
}

/* ----- Configuration ----- */

void D3D11CommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
{
    stateMngr_->SetViewports(1, &viewport);
}

void D3D11CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    stateMngr_->SetViewports(numViewports, viewports);
}

void D3D11CommandBuffer::SetScissor(const Scissor& scissor)
{
    stateMngr_->SetScissors(1, &scissor);
}

void D3D11CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    stateMngr_->SetScissors(numScissors, scissors);
}

/* ----- Clear ----- */

void D3D11CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearValue_.color = color;
}

void D3D11CommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depth = depth;
}

void D3D11CommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.stencil = (stencil & 0xff);
}

static UINT GetClearFlagsDSV(long flags)
{
    UINT clearFlagsDSV = 0;

    if ((flags & ClearFlags::Depth) != 0)
        clearFlagsDSV |= D3D11_CLEAR_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        clearFlagsDSV |= D3D11_CLEAR_STENCIL;

    return clearFlagsDSV;
}

void D3D11CommandBuffer::Clear(long flags)
{
    /* Clear color buffer */
    if ((flags & ClearFlags::Color) != 0)
    {
        for (auto rtv : framebufferView_.rtvList)
            context_->ClearRenderTargetView(rtv, clearValue_.color.Ptr());
    }

    /* Clear depth-stencil buffer */
    if (framebufferView_.dsv != nullptr)
    {
        if (auto clearFlagsDSV = GetClearFlagsDSV(flags))
        {
            context_->ClearDepthStencilView(
                framebufferView_.dsv,
                clearFlagsDSV,
                clearValue_.depth,
                static_cast<UINT8>(clearValue_.stencil)
            );
        }
    }
}

void D3D11CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    for (; numAttachments-- > 0; ++attachments)
    {
        if ((attachments->flags & ClearFlags::Color) != 0)
        {
            if (attachments->colorAttachment < framebufferView_.rtvList.size())
            {
                /* Clear color attachment */
                ClearColorBuffer(attachments->colorAttachment, attachments->clearValue.color);
            }
        }
        else if (framebufferView_.dsv != nullptr)
        {
            /* Clear depth and stencil buffer simultaneously */
            if (auto clearFlagsDSV = GetClearFlagsDSV(attachments->flags))
            {
                context_->ClearDepthStencilView(
                    framebufferView_.dsv,
                    clearFlagsDSV,
                    attachments->clearValue.depth,
                    static_cast<UINT8>(attachments->clearValue.stencil & 0xff)
                );
            }
        }
    }
}

/* ----- Input Assembly ------ */

void D3D11CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);

    ID3D11Buffer* buffers[] = { bufferD3D.GetNative() };
    UINT strides[] = { bufferD3D.GetStride() };
    UINT offsets[] = { 0 };

    context_->IASetVertexBuffers(0, 1, buffers, strides, offsets);
}

void D3D11CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D11BufferArray&, bufferArray);
    context_->IASetVertexBuffers(
        0,
        bufferArrayD3D.GetCount(),
        bufferArrayD3D.GetBuffers(),
        bufferArrayD3D.GetStrides(),
        bufferArrayD3D.GetOffsets()
    );
}

void D3D11CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->IASetIndexBuffer(bufferD3D.GetNative(), bufferD3D.GetFormat(), 0);
}

void D3D11CommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->IASetIndexBuffer(bufferD3D.GetNative(), D3D11Types::Map(format), static_cast<UINT>(offset));
}

/* ----- Stream Output Buffers ------ */

void D3D11CommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);

    ID3D11Buffer* buffers[] = { bufferD3D.GetNative() };
    UINT offsets[] = { 0 };

    context_->SOSetTargets(1, buffers, offsets);
}

void D3D11CommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayD3D = LLGL_CAST(D3D11BufferArray&, bufferArray);
    context_->SOSetTargets(
        bufferArrayD3D.GetCount(),
        bufferArrayD3D.GetBuffers(),
        bufferArrayD3D.GetOffsets()
    );
}

void D3D11CommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    // dummy
}

void D3D11CommandBuffer::EndStreamOutput()
{
    // dummy
}

/* ----- Resource Heaps ----- */

void D3D11CommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*firstSet*/)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);
    resourceHeapD3D.BindForGraphicsPipeline(context_.Get());
}

void D3D11CommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t /*firstSet*/)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);
    resourceHeapD3D.BindForComputePipeline(context_.Get());
}

/* ----- Render Passes ----- */

void D3D11CommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    /* Bind render target/context */
    if (renderTarget.IsRenderContext())
        BindRenderContext(LLGL_CAST(D3D11RenderContext&, renderTarget));
    else
        BindRenderTarget(LLGL_CAST(D3D11RenderTarget&, renderTarget));

    /* Clear attachments */
    if (renderPass)
    {
        auto renderPassD3D = LLGL_CAST(const D3D11RenderPass*, renderPass);
        ClearAttachmentsWithRenderPass(*renderPassD3D, numClearValues, clearValues);
    }
}

void D3D11CommandBuffer::EndRenderPass()
{
    // dummy
}

/* ----- Pipeline States ----- */

void D3D11CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineD3D = LLGL_CAST(D3D11GraphicsPipelineBase&, graphicsPipeline);
    graphicsPipelineD3D.Bind(*stateMngr_);
}

void D3D11CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineD3D = LLGL_CAST(D3D11ComputePipeline&, computePipeline);
    computePipelineD3D.Bind(*stateMngr_);
}

/* ----- Queries ----- */

void D3D11CommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Begin disjoint query first, and insert the beginning timestamp query */
        context_->Begin(queryHeapD3D.GetNative(query));
        context_->End(queryHeapD3D.GetNative(query + 1));
    }
    else
    {
        /* Begin standard query */
        context_->Begin(queryHeapD3D.GetNative(query));
    }
}

void D3D11CommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);

    query *= queryHeapD3D.GetGroupSize();

    if (queryHeapD3D.GetNativeType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Insert the ending timestamp query, and end the disjoint query */
        context_->End(queryHeapD3D.GetNative(query + 2));
        context_->End(queryHeapD3D.GetNative(query));
    }
    else
    {
        /* End standard query */
        context_->End(queryHeapD3D.GetNative(query));
    }
}

void D3D11CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapD3D = LLGL_CAST(D3D11QueryHeap&, queryHeap);
    context_->SetPredication(
        queryHeapD3D.GetPredicate(query * queryHeapD3D.GetGroupSize()),
        (mode >= RenderConditionMode::WaitInverted)
    );
}

void D3D11CommandBuffer::EndRenderCondition()
{
    context_->SetPredication(nullptr, FALSE);
}

/* ----- Drawing ----- */

void D3D11CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    context_->Draw(numVertices, firstVertex);
}

void D3D11CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    context_->DrawIndexed(numIndices, firstIndex, 0);
}

void D3D11CommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    context_->DrawIndexed(numIndices, firstIndex, vertexOffset);
}

void D3D11CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D11CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, 0, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    context_->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

void D3D11CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DrawInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11CommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    while (numCommands-- > 0)
    {
        context_->DrawInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
        offset += stride;
    }
}

void D3D11CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DrawIndexedInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

void D3D11CommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    while (numCommands-- > 0)
    {
        context_->DrawIndexedInstancedIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
        offset += stride;
    }
}

/* ----- Compute ----- */

void D3D11CommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    context_->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

void D3D11CommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    context_->DispatchIndirect(bufferD3D.GetNative(), static_cast<UINT>(offset));
}

/* ----- Direct Resource Access ------ */

void D3D11CommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    /* Set constant buffer resource to all shader stages */
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    auto resource = bufferD3D.GetNative();
    SetConstantBuffersOnStages(slot, 1, &resource, stageFlags);
}

void D3D11CommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    if (HasBufferResourceViews(buffer))
    {
        /* Set SRVs to specified shader stages */
        auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, buffer);
        ID3D11ShaderResourceView* srvList[] = { bufferD3D.GetSRV() };
        SetShaderResourcesOnStages(slot, 1, srvList, stageFlags);
    }
}

void D3D11CommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    if (HasBufferResourceViews(buffer))
    {
        /* Set UAVs to specified shader stages */
        auto& bufferD3D = LLGL_CAST(D3D11BufferWithRV&, buffer);
        ID3D11UnorderedAccessView* uavList[] = { bufferD3D.GetUAV() };
        UINT auvCounts[] = { bufferD3D.GetInitialCount() };
        SetUnorderedAccessViewsOnStages(slot, 1, uavList, auvCounts, stageFlags);
    }
}

void D3D11CommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long stageFlags)
{
    /* Set texture resource to all shader stages */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    auto resource = textureD3D.GetSRV();
    SetShaderResourcesOnStages(slot, 1, &resource, stageFlags);
}

void D3D11CommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags)
{
    /* Set sampler state object to all shader stages */
    auto& samplerD3D = LLGL_CAST(D3D11Sampler&, sampler);
    auto resource = samplerD3D.GetNative();
    SetSamplersOnStages(slot, 1, &resource, stageFlags);
}

void D3D11CommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{
    if (numSlots > 0)
    {
        /* Reset resource binding slots */
        switch (resourceType)
        {
            case ResourceType::Undefined:
                break;
            case ResourceType::Buffer:
                ResetBufferResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
            case ResourceType::Texture:
                ResetTextureResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
            case ResourceType::Sampler:
                ResetSamplerResourceSlots(firstSlot, numSlots, bindFlags, stageFlags);
                break;
        }
    }
}


/*
 * ======= Private: =======
 */

void D3D11CommandBuffer::SetConstantBuffersOnStages(UINT startSlot, UINT count, ID3D11Buffer* const* buffers, long stageFlags)
{
    if (VS_STAGE(stageFlags)) { context_->VSSetConstantBuffers(startSlot, count, buffers); }
    if (HS_STAGE(stageFlags)) { context_->HSSetConstantBuffers(startSlot, count, buffers); }
    if (DS_STAGE(stageFlags)) { context_->DSSetConstantBuffers(startSlot, count, buffers); }
    if (GS_STAGE(stageFlags)) { context_->GSSetConstantBuffers(startSlot, count, buffers); }
    if (PS_STAGE(stageFlags)) { context_->PSSetConstantBuffers(startSlot, count, buffers); }
    if (CS_STAGE(stageFlags)) { context_->CSSetConstantBuffers(startSlot, count, buffers); }
}

void D3D11CommandBuffer::SetShaderResourcesOnStages(UINT startSlot, UINT count, ID3D11ShaderResourceView* const* views, long stageFlags)
{
    if (VS_STAGE(stageFlags)) { context_->VSSetShaderResources(startSlot, count, views); }
    if (HS_STAGE(stageFlags)) { context_->HSSetShaderResources(startSlot, count, views); }
    if (DS_STAGE(stageFlags)) { context_->DSSetShaderResources(startSlot, count, views); }
    if (GS_STAGE(stageFlags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (PS_STAGE(stageFlags)) { context_->PSSetShaderResources(startSlot, count, views); }
    if (CS_STAGE(stageFlags)) { context_->CSSetShaderResources(startSlot, count, views); }
}

void D3D11CommandBuffer::SetSamplersOnStages(UINT startSlot, UINT count, ID3D11SamplerState* const* samplers, long stageFlags)
{
    if (VS_STAGE(stageFlags)) { context_->VSSetSamplers(startSlot, count, samplers); }
    if (HS_STAGE(stageFlags)) { context_->HSSetSamplers(startSlot, count, samplers); }
    if (DS_STAGE(stageFlags)) { context_->DSSetSamplers(startSlot, count, samplers); }
    if (GS_STAGE(stageFlags)) { context_->GSSetSamplers(startSlot, count, samplers); }
    if (PS_STAGE(stageFlags)) { context_->PSSetSamplers(startSlot, count, samplers); }
    if (CS_STAGE(stageFlags)) { context_->CSSetSamplers(startSlot, count, samplers); }
}

void D3D11CommandBuffer::SetUnorderedAccessViewsOnStages(
    UINT startSlot, UINT count, ID3D11UnorderedAccessView* const* views, const UINT* initialCounts, long stageFlags)
{
    if (PS_STAGE(stageFlags))
    {
        /* Set UAVs for pixel shader stage */
        context_->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
            startSlot, count, views, initialCounts
        );
    }

    if (CS_STAGE(stageFlags))
    {
        /* Set UAVs for compute shader stage */
        context_->CSSetUnorderedAccessViews(startSlot, count, views, initialCounts);
    }
}

void D3D11CommandBuffer::ResetBufferResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Reset vertex buffer slots */
    if ((bindFlags & BindFlags::VertexBuffer) != 0)
    {
        if ((stageFlags & StageFlags::VertexStage) != 0)
        {
            /* Clamp slot indices */
            firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) - 1);
            numSlots    = std::min(numSlots, std::uint32_t(D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT) - firstSlot);

            /* Unbind vertex buffers */
            context_->IASetVertexBuffers(
                firstSlot,
                numSlots,
                reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
                g_zeroCounters,
                g_zeroCounters
            );
        }
    }

    /* Reset index buffer slot */
    if ((bindFlags & BindFlags::IndexBuffer) != 0)
    {
        if (firstSlot == 0 && (stageFlags & StageFlags::VertexStage) != 0)
            context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    }

    /* Reset constant buffer slots */
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Clamp slot indices */
        firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT) - 1);
        numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT) - firstSlot);

        /* Unbind constant buffers */
        SetConstantBuffersOnStages(
            firstSlot,
            numSlots,
            reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
            stageFlags
        );
    }

    /* Reset stream-output buffer slots */
    if ((bindFlags & BindFlags::StreamOutputBuffer) != 0)
    {
        if (firstSlot == 0 && (stageFlags & (StageFlags::VertexStage | StageFlags::GeometryStage)) !=0 )
        {
            /* Clamp slot indices */
            firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_SO_BUFFER_SLOT_COUNT) - 1);
            numSlots    = std::min(numSlots, std::uint32_t(D3D11_SO_BUFFER_SLOT_COUNT) - firstSlot);

            /* Unbind stream-output buffers */
            context_->SOSetTargets(
                numSlots,
                reinterpret_cast<ID3D11Buffer* const*>(g_nullResources),
                g_zeroCounters
            );
        }
    }

    /* Reset sample buffer slots */
    if ((bindFlags & BindFlags::SampleBuffer) != 0)
        ResetResourceSlotsSRV(firstSlot, numSlots, stageFlags);

    /* Reset read/write storage buffer slots */
    if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
        ResetResourceSlotsUAV(firstSlot, numSlots, stageFlags);
}

void D3D11CommandBuffer::ResetTextureResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Reset sample buffer slots */
    if ((bindFlags & BindFlags::SampleBuffer) != 0)
        ResetResourceSlotsSRV(firstSlot, numSlots, stageFlags);

    /* Reset read/write storage buffer slots */
    if ((bindFlags & BindFlags::RWStorageBuffer) != 0)
        ResetResourceSlotsUAV(firstSlot, numSlots, stageFlags);
}

void D3D11CommandBuffer::ResetSamplerResourceSlots(std::uint32_t firstSlot, std::uint32_t numSlots, long bindFlags, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT) - firstSlot);

    /* Unbind sampler states */
    SetSamplersOnStages(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11SamplerState* const*>(g_nullResources),
        stageFlags
    );
}

void D3D11CommandBuffer::ResetResourceSlotsSRV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT) - firstSlot);

    /* Unbind SRVs */
    SetShaderResourcesOnStages(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11ShaderResourceView* const*>(g_nullResources),
        stageFlags
    );
}

void D3D11CommandBuffer::ResetResourceSlotsUAV(std::uint32_t firstSlot, std::uint32_t numSlots, long stageFlags)
{
    /* Clamp slot indices */
    firstSlot   = std::min(firstSlot, std::uint32_t(D3D11_1_UAV_SLOT_COUNT) - 1);
    numSlots    = std::min(numSlots, std::uint32_t(D3D11_1_UAV_SLOT_COUNT) - firstSlot);

    /* Unbind UAVs */
    SetUnorderedAccessViewsOnStages(
        firstSlot,
        numSlots,
        reinterpret_cast<ID3D11UnorderedAccessView* const*>(g_nullResources),
        nullptr,
        stageFlags
    );
}

#undef VS_STAGE
#undef HS_STAGE
#undef DS_STAGE
#undef GS_STAGE
#undef PS_STAGE
#undef CS_STAGE

void D3D11CommandBuffer::ResolveBoundRenderTarget()
{
    if (boundRenderTarget_)
        boundRenderTarget_->ResolveSubresources(context_.Get());
}

void D3D11CommandBuffer::BindFramebufferView()
{
    context_->OMSetRenderTargets(
        static_cast<UINT>(framebufferView_.rtvList.size()),
        framebufferView_.rtvList.data(),
        framebufferView_.dsv
    );
}

void D3D11CommandBuffer::BindRenderTarget(D3D11RenderTarget& renderTargetD3D)
{
    /* Resolve previously bound render target (in case mutli-sampling is used) */
    ResolveBoundRenderTarget();

    /* Set RTV list and DSV in framebuffer view */
    framebufferView_.rtvList    = renderTargetD3D.GetRenderTargetViews();
    framebufferView_.dsv        = renderTargetD3D.GetDepthStencilView();

    BindFramebufferView();

    /* Store current render target */
    boundRenderTarget_ = &renderTargetD3D;
}

void D3D11CommandBuffer::BindRenderContext(D3D11RenderContext& renderContextD3D)
{
    /* Resolve previously bound render target (in case mutli-sampling is used) */
    ResolveBoundRenderTarget();

    /* Set default RTVs to OM-stage */
    const auto& backBuffer = renderContextD3D.GetBackBuffer();

    framebufferView_.rtvList    = { backBuffer.rtv.Get() };
    framebufferView_.dsv        = backBuffer.dsv.Get();

    BindFramebufferView();

    /* Reset reference to render target */
    boundRenderTarget_ = nullptr;
}

void D3D11CommandBuffer::ClearAttachmentsWithRenderPass(
    const D3D11RenderPass&  renderPassD3D,
    std::uint32_t           numClearValues,
    const ClearValue*       clearValues)
{
    /* Clear color attachments */
    std::uint32_t idx = 0;
    ClearColorBuffers(renderPassD3D.GetClearColorAttachments(), numClearValues, clearValues, idx);

    /* Clear depth-stencil attachment */
    if (framebufferView_.dsv != nullptr)
    {
        if (auto clearFlagsDSV = renderPassD3D.GetClearFlagsDSV())
        {
            /* Get clear values */
            FLOAT depth     = clearValue_.depth;
            UINT8 stencil   = static_cast<UINT8>(clearValue_.stencil);

            if (idx < numClearValues)
            {
                depth   = clearValues[idx].depth;
                stencil = static_cast<UINT8>(clearValues[idx].stencil & 0xff);
            }

            /* Clear depth-stencil view */
            context_->ClearDepthStencilView(framebufferView_.dsv, clearFlagsDSV, depth, stencil);
        }
    }
}

void D3D11CommandBuffer::ClearColorBuffer(std::uint32_t idx, const ColorRGBAf& color)
{
    context_->ClearRenderTargetView(framebufferView_.rtvList[idx], color.Ptr());
}

void D3D11CommandBuffer::ClearColorBuffers(
    const std::uint8_t* colorBuffers,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t&      idx)
{
    std::uint32_t i = 0, n = static_cast<std::uint32_t>(framebufferView_.rtvList.size());

    numClearValues = std::min(numClearValues, n);

    /* Use specified clear values */
    for (; i < numClearValues; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
            ClearColorBuffer(colorBuffers[i], clearValues[idx++].color);
        else
            return;
    }

    /* Use default clear values */
    for (; i < n; ++i)
    {
        /* Check if attachment list has ended */
        if (colorBuffers[i] != 0xFF)
            ClearColorBuffer(colorBuffers[i], clearValue_.color);
        else
            return;
    }
}


} // /namespace LLGL



// ================================================================================
