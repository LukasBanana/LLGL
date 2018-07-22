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
#include "RenderState/D3D11Query.h"
#include "RenderState/D3D11ResourceHeap.h"
#include "RenderState/D3D11RenderPass.h"

#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"
#include "Buffer/D3D11StorageBuffer.h"
#include "Buffer/D3D11StreamOutputBuffer.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"


namespace LLGL
{


#define VS_STAGE(FLAG)  ( ((FLAG) & StageFlags::VertexStage        ) != 0 )
#define HS_STAGE(FLAG)  ( ((FLAG) & StageFlags::TessControlStage   ) != 0 )
#define DS_STAGE(FLAG)  ( ((FLAG) & StageFlags::TessEvaluationStage) != 0 )
#define GS_STAGE(FLAG)  ( ((FLAG) & StageFlags::GeometryStage      ) != 0 )
#define PS_STAGE(FLAG)  ( ((FLAG) & StageFlags::FragmentStage      ) != 0 )
#define CS_STAGE(FLAG)  ( ((FLAG) & StageFlags::ComputeStage       ) != 0 )
#define SRV_STAGE(FLAG) ( ((FLAG) & StageFlags::ReadOnlyResource   ) != 0 )

D3D11CommandBuffer::D3D11CommandBuffer(D3D11StateManager& stateMngr, const ComPtr<ID3D11DeviceContext>& context) :
    stateMngr_ { stateMngr },
    context_   { context   }
{
}

/* ----- Encoding ----- */

void D3D11CommandBuffer::Begin()
{
    // dummy
}

void D3D11CommandBuffer::End()
{
    // dummy
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

/* ----- Configuration ----- */

void D3D11CommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
{
    stateMngr_.SetViewports(1, &viewport);
}

void D3D11CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    stateMngr_.SetViewports(numViewports, viewports);
}

void D3D11CommandBuffer::SetScissor(const Scissor& scissor)
{
    stateMngr_.SetScissors(1, &scissor);
}

void D3D11CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    stateMngr_.SetScissors(numScissors, scissors);
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
    auto& vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer&, buffer);

    ID3D11Buffer* buffers[] = { vertexBufferD3D.GetNative() };
    UINT strides[] = { vertexBufferD3D.GetStride() };
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
    auto& indexBufferD3D = LLGL_CAST(D3D11IndexBuffer&, buffer);
    context_->IASetIndexBuffer(indexBufferD3D.GetNative(), indexBufferD3D.GetFormat(), 0);
}

/* ----- Constant Buffers ------ */

void D3D11CommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    /* Set constant buffer resource to all shader stages */
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, buffer);
    auto resource = constantBufferD3D.GetNative();
    SetConstantBuffersOnStages(slot, 1, &resource, stageFlags);
}

/* ----- Storage Buffers ------ */

void D3D11CommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    auto& storageBufferD3D = LLGL_CAST(D3D11StorageBuffer&, buffer);

    if (storageBufferD3D.HasUAV() && !SRV_STAGE(stageFlags))
    {
        /* Set UAVs to specified shader stages */
        ID3D11UnorderedAccessView* uavList[] = { storageBufferD3D.GetUAV() };
        UINT auvCounts[] = { storageBufferD3D.GetInitialCount() };
        SetUnorderedAccessViewsOnStages(slot, 1, uavList, auvCounts, stageFlags);
    }
    else
    {
        /* Set SRVs to specified shader stages */
        ID3D11ShaderResourceView* srvList[] = { storageBufferD3D.GetSRV() };
        SetShaderResourcesOnStages(slot, 1, srvList, stageFlags);
    }
}

/* ----- Stream Output Buffers ------ */

void D3D11CommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    auto& streamOutputBufferD3D = LLGL_CAST(D3D11StreamOutputBuffer&, buffer);

    ID3D11Buffer* buffers[] = { streamOutputBufferD3D.GetNative() };
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


/* ----- Textures ----- */

void D3D11CommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long stageFlags)
{
    /* Set texture resource to all shader stages */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    auto resource = textureD3D.GetSRV();
    SetShaderResourcesOnStages(slot, 1, &resource, stageFlags);
}

/* ----- Sampler States ----- */

void D3D11CommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags)
{
    /* Set sampler state object to all shader stages */
    auto& samplerD3D = LLGL_CAST(D3D11Sampler&, sampler);
    auto resource = samplerD3D.GetNative();
    SetSamplersOnStages(slot, 1, &resource, stageFlags);
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
    graphicsPipelineD3D.Bind(stateMngr_);
}

void D3D11CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineD3D = LLGL_CAST(D3D11ComputePipeline&, computePipeline);
    computePipelineD3D.Bind(stateMngr_);
}

/* ----- Queries ----- */

void D3D11CommandBuffer::BeginQuery(Query& query)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    if (queryD3D.GetD3DQueryType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Begin disjoint query first, and insert the beginning timestamp query */
        context_->Begin(queryD3D.GetNative());
        context_->End(queryD3D.GetTimeStampQueryBegin());
    }
    else
    {
        /* Begin standard query */
        context_->Begin(queryD3D.GetNative());
    }
}

void D3D11CommandBuffer::EndQuery(Query& query)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    if (queryD3D.GetD3DQueryType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Insert the ending timestamp query, and end the disjoint query */
        context_->End(queryD3D.GetTimeStampQueryEnd());
        context_->End(queryD3D.GetNative());
    }
    else
    {
        /* End standard query */
        context_->End(queryD3D.GetNative());
    }
}

bool D3D11CommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    switch (queryD3D.GetD3DQueryType())
    {
        /* Query result from data of type: UINT64 */
        case D3D11_QUERY_OCCLUSION:
        {
            UINT64 data = 0;
            if (context_->GetData(queryD3D.GetNative(), &data, sizeof(data), 0) == S_OK)
            {
                result = data;
                return true;
            }
        }
        break;

        /* Query result from special case query type: TimeElapsed */
        case D3D11_QUERY_TIMESTAMP_DISJOINT:
        {
            UINT64 startTime = 0;
            if (context_->GetData(queryD3D.GetTimeStampQueryBegin(), &startTime, sizeof(startTime), 0) == S_OK)
            {
                UINT64 endTime = 0;
                if (context_->GetData(queryD3D.GetTimeStampQueryEnd(), &endTime, sizeof(endTime), 0) == S_OK)
                {
                    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
                    if (context_->GetData(queryD3D.GetNative(), &disjointData, sizeof(disjointData), 0) == S_OK)
                    {
                        if (disjointData.Disjoint == FALSE)
                        {
                            /* Normalize elapsed time to nanoseconds */
                            static const double nanoseconds = 1000000000.0;

                            auto deltaTime      = (endTime - startTime);
                            auto scale          = (nanoseconds / static_cast<double>(disjointData.Frequency));
                            auto elapsedTime    = (static_cast<double>(deltaTime) * scale);

                            result = static_cast<std::uint64_t>(elapsedTime + 0.5);
                        }
                        else
                            result = 0;
                        return true;
                    }
                }
            }
        }
        break;

        /* Query result from data of type: BOOL */
        case D3D11_QUERY_OCCLUSION_PREDICATE:
        case D3D11_QUERY_SO_OVERFLOW_PREDICATE:
        {
            BOOL data = 0;
            if (context_->GetData(queryD3D.GetNative(), &data, sizeof(data), 0) == S_OK)
            {
                result = data;
                return true;
            }
        }
        break;

        /* Query result from data of type: D3D11_QUERY_DATA_SO_STATISTICS */
        case D3D11_QUERY_SO_STATISTICS:
        {
            D3D11_QUERY_DATA_SO_STATISTICS data;
            if (context_->GetData(queryD3D.GetNative(), &data, sizeof(data), 0) == S_OK)
            {
                result = data.NumPrimitivesWritten;
                return true;
            }
        }
        break;

        default:
        break;
    }

    return false;
}

bool D3D11CommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    /* Query result from data of type: D3D11_QUERY_DATA_PIPELINE_STATISTICS */
    if (queryD3D.GetD3DQueryType() == D3D11_QUERY_PIPELINE_STATISTICS)
    {
        D3D11_QUERY_DATA_PIPELINE_STATISTICS data;
        if (context_->GetData(queryD3D.GetNative(), &data, sizeof(data), 0) == S_OK)
        {
            result.numPrimitivesGenerated               = data.CInvocations;
            result.numVerticesSubmitted                 = data.IAVertices;
            result.numPrimitivesSubmitted               = data.IAPrimitives;
            result.numVertexShaderInvocations           = data.VSInvocations;
            result.numTessControlShaderInvocations      = data.HSInvocations;
            result.numTessEvaluationShaderInvocations   = data.DSInvocations;
            result.numGeometryShaderInvocations         = data.GSInvocations;
            result.numFragmentShaderInvocations         = data.PSInvocations;
            result.numComputeShaderInvocations          = data.CSInvocations;
            result.numGeometryPrimitivesGenerated       = data.GSPrimitives;
            result.numClippingInputPrimitives           = data.CInvocations; // <-- TODO: workaround
            result.numClippingOutputPrimitives          = data.CPrimitives;
            return true;
        }
    }

    return false;
}

void D3D11CommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);
    context_->SetPredication(queryD3D.GetPredicate(), (mode >= RenderConditionMode::WaitInverted));
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

/* ----- Compute ----- */

void D3D11CommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    context_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
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

#undef VS_STAGE
#undef HS_STAGE
#undef DS_STAGE
#undef GS_STAGE
#undef PS_STAGE
#undef CS_STAGE
#undef SRV_STAGE

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
