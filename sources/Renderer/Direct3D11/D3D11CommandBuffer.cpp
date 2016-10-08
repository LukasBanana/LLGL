/*
 * D3D11CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
#include "RenderState/D3D11GraphicsPipeline.h"
#include "RenderState/D3D11ComputePipeline.h"
#include "RenderState/D3D11Query.h"

#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11VertexBufferArray.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"
#include "Buffer/D3D11StorageBuffer.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11TextureArray.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"


namespace LLGL
{


#define VS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::VertexStage        ) != 0 )
#define HS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::TessControlStage   ) != 0 )
#define DS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::TessEvaluationStage) != 0 )
#define GS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::GeometryStage      ) != 0 )
#define PS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::FragmentStage      ) != 0 )
#define CS_STAGE(FLAG) ( ((FLAG) & ShaderStageFlags::ComputeStage       ) != 0 )

D3D11CommandBuffer::D3D11CommandBuffer(D3D11StateManager& stateMngr, const ComPtr<ID3D11DeviceContext>& context) :
    stateMngr_  ( stateMngr ),
    context_    ( context   )
{
}

/* ----- Configuration ----- */

void D3D11CommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    // dummy
}

void D3D11CommandBuffer::SetViewport(const Viewport& viewport)
{
    stateMngr_.SetViewports(1, &viewport);
}

void D3D11CommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    stateMngr_.SetViewports(numViewports, viewportArray);
}

void D3D11CommandBuffer::SetScissor(const Scissor& scissor)
{
    stateMngr_.SetScissors(1, &scissor);
}

void D3D11CommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
{
    stateMngr_.SetScissors(numScissors, scissorArray);
}

void D3D11CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearState_.color = color;
}

void D3D11CommandBuffer::SetClearDepth(float depth)
{
    clearState_.depth = depth;
}

void D3D11CommandBuffer::SetClearStencil(int stencil)
{
    clearState_.stencil = stencil;
}

void D3D11CommandBuffer::ClearBuffers(long flags)
{
    /* Clear color buffer */
    if ((flags & ClearBuffersFlags::Color) != 0)
    {
        for (auto rtv : framebufferView_.rtvList)
            context_->ClearRenderTargetView(rtv, clearState_.color.Ptr());
    }
    
    /* Clear depth-stencil buffer */
    int dsvClearFlags = 0;

    if ((flags & ClearBuffersFlags::Depth) != 0)
        dsvClearFlags |= D3D11_CLEAR_DEPTH;
    if ((flags & ClearBuffersFlags::Stencil) != 0)
        dsvClearFlags |= D3D11_CLEAR_STENCIL;
        
    if (dsvClearFlags && framebufferView_.dsv != nullptr)
        context_->ClearDepthStencilView(framebufferView_.dsv, dsvClearFlags, clearState_.depth, clearState_.stencil);
}

/* ----- Hardware Buffers ------ */

void D3D11CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer&, buffer);

    ID3D11Buffer* buffers[] = { vertexBufferD3D.Get() };
    UINT strides[] = { vertexBufferD3D.GetStride() };
    UINT offsets[] = { 0 };

    context_->IASetVertexBuffers(0, 1, buffers, strides, offsets);
}

void D3D11CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& vertexBufferArrayD3D = LLGL_CAST(D3D11VertexBufferArray&, bufferArray);

    context_->IASetVertexBuffers(
        0,
        static_cast<UINT>(vertexBufferArrayD3D.GetBuffers().size()),
        vertexBufferArrayD3D.GetBuffers().data(),
        vertexBufferArrayD3D.GetStrides().data(),
        vertexBufferArrayD3D.GetOffsets().data()
    );
}

void D3D11CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& indexBufferD3D = LLGL_CAST(D3D11IndexBuffer&, buffer);
    context_->IASetIndexBuffer(indexBufferD3D.Get(), indexBufferD3D.GetFormat(), 0);
}

void D3D11CommandBuffer::SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    /* Set constant buffer resource to all shader stages */
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, buffer);
    auto resource = constantBufferD3D.Get();
    SetConstantBuffersOnStages(slot, 1, &resource, shaderStageFlags);
}

void D3D11CommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags)
{
    /* Set constant buffer resource to all shader stages */
    auto& bufferArrayD3D = LLGL_CAST(D3D11BufferArray&, bufferArray);
    SetConstantBuffersOnStages(
        startSlot,
        static_cast<UINT>(bufferArrayD3D.GetBuffers().size()),
        bufferArrayD3D.GetBuffers().data(),
        shaderStageFlags
    );
}

void D3D11CommandBuffer::SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    auto& storageBufferD3D = LLGL_CAST(D3D11StorageBuffer&, buffer);

    if (storageBufferD3D.IsUAV())
    {
        /* Get UAV list and initial counts */
        ID3D11UnorderedAccessView* uavList[] = { storageBufferD3D.GetUAV() };
        UINT auvCounts[] = { storageBufferD3D.GetInitialCount() };

        if (PS_STAGE(shaderStageFlags))
        {
            /* Set UAVs for pixel shader stage */
            context_->OMSetRenderTargetsAndUnorderedAccessViews(
                D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
                slot, 1, uavList, auvCounts
            );
        }

        if (CS_STAGE(shaderStageFlags))
        {
            /* Set UAVs for compute shader stage */
            context_->CSSetUnorderedAccessViews(slot, 1, uavList, auvCounts);
        }
    }
    else
    {
        /* Set SRVs to specified shader stages */
        ID3D11ShaderResourceView* srvList[] = { storageBufferD3D.GetSRV() };
        SetShaderResourcesOnStages(slot, 1, srvList, shaderStageFlags);
    }
}

/* ----- Textures ----- */

void D3D11CommandBuffer::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    /* Set texture resource to all shader stages */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    auto resource = textureD3D.GetSRV();
    SetShaderResourcesOnStages(slot, 1, &resource, shaderStageFlags);
}

void D3D11CommandBuffer::SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags)
{
    /* Set texture resource to all shader stages */
    auto& textureArrayD3D = LLGL_CAST(D3D11TextureArray&, textureArray);
    SetShaderResourcesOnStages(
        startSlot,
        static_cast<UINT>(textureArrayD3D.GetResourceViews().size()),
        textureArrayD3D.GetResourceViews().data(),
        shaderStageFlags
    );
}

/* ----- Sampler States ----- */

void D3D11CommandBuffer::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    /* Set sampler state object to all shader stages */
    auto& samplerD3D = LLGL_CAST(D3D11Sampler&, sampler);
    auto resource = samplerD3D.GetSamplerState();
    SetSamplersOnStages(slot, 1, &resource, shaderStageFlags);
}

/* ----- Render Targets ----- */

//private
void D3D11CommandBuffer::ResolveBoundRenderTarget()
{
    if (boundRenderTarget_)
        boundRenderTarget_->ResolveSubresources(context_.Get());
}

void D3D11CommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    auto& renderTargetD3D = LLGL_CAST(D3D11RenderTarget&, renderTarget);

    /* Resolve previously bound render target (in case mutli-sampling is used) */
    ResolveBoundRenderTarget();

    /* Set RTV list and DSV in framebuffer view */
    framebufferView_.rtvList    = renderTargetD3D.GetRenderTargetViews();
    framebufferView_.dsv        = renderTargetD3D.GetDepthStencilView();

    SubmitFramebufferView();

    /* Store current render target */
    boundRenderTarget_ = &renderTargetD3D;
}

void D3D11CommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextD3D = LLGL_CAST(D3D11RenderContext&, renderContext);

    /* Resolve previously bound render target (in case mutli-sampling is used) */
    ResolveBoundRenderTarget();

    /* Set default RTVs to OM-stage */
    const auto& backBuffer = renderContextD3D.GetBackBuffer();

    framebufferView_.rtvList    = { backBuffer.rtv.Get() };
    framebufferView_.dsv        = backBuffer.dsv.Get();

    SubmitFramebufferView();

    /* Reset reference to render target */
    boundRenderTarget_ = nullptr;
}

/* ----- Pipeline States ----- */

void D3D11CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineD3D = LLGL_CAST(D3D11GraphicsPipeline&, graphicsPipeline);
    graphicsPipelineD3D.Bind(context_.Get());
}

void D3D11CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineD3D = LLGL_CAST(D3D11ComputePipeline&, computePipeline);
    computePipelineD3D.Bind(context_.Get());
}

/* ----- Queries ----- */

void D3D11CommandBuffer::BeginQuery(Query& query)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    if (queryD3D.GetQueryObjectType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Begin disjoint query first, and insert the beginning timestamp query */
        context_->Begin(queryD3D.GetQueryObject());
        context_->End(queryD3D.GetTimeStampQueryBegin());
    }
    else
    {
        /* Begin standard query */
        context_->Begin(queryD3D.GetQueryObject());
    }
}

void D3D11CommandBuffer::EndQuery(Query& query)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    if (queryD3D.GetQueryObjectType() == D3D11_QUERY_TIMESTAMP_DISJOINT)
    {
        /* Insert the ending timestamp query, and end the disjoint query */
        context_->End(queryD3D.GetTimeStampQueryEnd());
        context_->End(queryD3D.GetQueryObject());
    }
    else
    {
        /* End standard query */
        context_->End(queryD3D.GetQueryObject());
    }
}

bool D3D11CommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);

    switch (queryD3D.GetQueryObjectType())
    {
        /* Query result from data of type: UINT64 */
        case D3D11_QUERY_OCCLUSION:
        {
            UINT64 data = 0;
            if (context_->GetData(queryD3D.GetQueryObject(), &data, sizeof(data), 0) == S_OK)
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
                    if (context_->GetData(queryD3D.GetQueryObject(), &disjointData, sizeof(disjointData), 0) == S_OK)
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
            if (context_->GetData(queryD3D.GetQueryObject(), &data, sizeof(data), 0) == S_OK)
            {
                result = data;
                return true;
            }
        }
        break;

        /* Query result from data of type: D3D11_QUERY_DATA_PIPELINE_STATISTICS */
        case D3D11_QUERY_PIPELINE_STATISTICS:
        {
            D3D11_QUERY_DATA_PIPELINE_STATISTICS data;
            if (context_->GetData(queryD3D.GetQueryObject(), &data, sizeof(data), 0) == S_OK)
            {
                switch (queryD3D.GetType())
                {
                    case QueryType::PrimitivesGenerated:
                        result = data.CInvocations;
                        break;
                    case QueryType::VerticesSubmitted:
                        result = data.IAVertices;
                        break;
                    case QueryType::PrimitivesSubmitted:
                        result = data.IAPrimitives;
                        break;
                    case QueryType::VertexShaderInvocations:
                        result = data.VSInvocations;
                        break;
                    case QueryType::TessControlShaderInvocations:
                        result = data.HSInvocations;
                        break;
                    case QueryType::TessEvaluationShaderInvocations:
                        result = data.DSInvocations;
                        break;
                    case QueryType::GeometryShaderInvocations:
                        result = data.GSInvocations;
                        break;
                    case QueryType::FragmentShaderInvocations:
                        result = data.PSInvocations;
                        break;
                    case QueryType::ComputeShaderInvocations:
                        result = data.CSInvocations;
                        break;
                    case QueryType::GeometryPrimitivesGenerated:
                        result = data.GSPrimitives;
                        break;
                    case QueryType::ClippingInputPrimitives:
                        result = data.CInvocations; // <-- TODO: workaround
                        break;
                    case QueryType::ClippingOutputPrimitives:
                        result = data.CPrimitives;
                        break;
                    default:
                        return false;
                }
                return true;
            }
        }
        break;

        /* Query result from data of type: D3D11_QUERY_DATA_SO_STATISTICS */
        case D3D11_QUERY_SO_STATISTICS:
        {
            D3D11_QUERY_DATA_SO_STATISTICS data;
            if (context_->GetData(queryD3D.GetQueryObject(), &data, sizeof(data), 0) == S_OK)
            {
                result = data.NumPrimitivesWritten;
                return true;
            }
        }
        break;
    }

    return false;
}

void D3D11CommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);
    context_->SetPredication(queryD3D.GetPredicateObject(), (mode >= RenderConditionMode::WaitInverted));
}

void D3D11CommandBuffer::EndRenderCondition()
{
    context_->SetPredication(nullptr, FALSE);
}

/* ----- Drawing ----- */

void D3D11CommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    context_->Draw(numVertices, firstVertex);
}

void D3D11CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    context_->DrawIndexed(numVertices, firstIndex, 0);
}

void D3D11CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    context_->DrawIndexed(numVertices, firstIndex, vertexOffset);
}

void D3D11CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D11CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D11CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D11CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void D3D11CommandBuffer::DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    context_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
}

/* ----- Misc ----- */

void D3D11CommandBuffer::SyncGPU()
{
    context_->Flush();
}


/*
 * ======= Private: =======
 */

void D3D11CommandBuffer::SubmitFramebufferView()
{
    context_->OMSetRenderTargets(
        static_cast<UINT>(framebufferView_.rtvList.size()),
        framebufferView_.rtvList.data(),
        framebufferView_.dsv
    );
}

void D3D11CommandBuffer::SetConstantBuffersOnStages(UINT startSlot, UINT count, ID3D11Buffer* const* buffers, long shaderStageFlags)
{
    if (VS_STAGE(shaderStageFlags)) { context_->VSSetConstantBuffers(startSlot, count, buffers); }
    if (HS_STAGE(shaderStageFlags)) { context_->HSSetConstantBuffers(startSlot, count, buffers); }
    if (DS_STAGE(shaderStageFlags)) { context_->DSSetConstantBuffers(startSlot, count, buffers); }
    if (GS_STAGE(shaderStageFlags)) { context_->GSSetConstantBuffers(startSlot, count, buffers); }
    if (PS_STAGE(shaderStageFlags)) { context_->PSSetConstantBuffers(startSlot, count, buffers); }
    if (CS_STAGE(shaderStageFlags)) { context_->CSSetConstantBuffers(startSlot, count, buffers); }
}

void D3D11CommandBuffer::SetShaderResourcesOnStages(UINT startSlot, UINT count, ID3D11ShaderResourceView* const* views, long shaderStageFlags)
{
    if (VS_STAGE(shaderStageFlags)) { context_->VSSetShaderResources(startSlot, count, views); }
    if (HS_STAGE(shaderStageFlags)) { context_->HSSetShaderResources(startSlot, count, views); }
    if (DS_STAGE(shaderStageFlags)) { context_->DSSetShaderResources(startSlot, count, views); }
    if (GS_STAGE(shaderStageFlags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (PS_STAGE(shaderStageFlags)) { context_->PSSetShaderResources(startSlot, count, views); }
    if (CS_STAGE(shaderStageFlags)) { context_->CSSetShaderResources(startSlot, count, views); }
}

void D3D11CommandBuffer::SetSamplersOnStages(UINT startSlot, UINT count, ID3D11SamplerState* const* samplers, long shaderStageFlags)
{
    if (VS_STAGE(shaderStageFlags)) { context_->VSSetSamplers(startSlot, count, samplers); }
    if (HS_STAGE(shaderStageFlags)) { context_->HSSetSamplers(startSlot, count, samplers); }
    if (DS_STAGE(shaderStageFlags)) { context_->DSSetSamplers(startSlot, count, samplers); }
    if (GS_STAGE(shaderStageFlags)) { context_->GSSetSamplers(startSlot, count, samplers); }
    if (PS_STAGE(shaderStageFlags)) { context_->PSSetSamplers(startSlot, count, samplers); }
    if (CS_STAGE(shaderStageFlags)) { context_->CSSetSamplers(startSlot, count, samplers); }
}

#undef VS_STAGE
#undef HS_STAGE
#undef DS_STAGE
#undef GS_STAGE
#undef PS_STAGE
#undef CS_STAGE


} // /namespace LLGL



// ================================================================================
