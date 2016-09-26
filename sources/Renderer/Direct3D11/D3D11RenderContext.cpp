/*
 * D3D11RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderContext.h"
#include "D3D11RenderSystem.h"
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
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"


namespace LLGL
{


D3D11RenderContext::D3D11RenderContext(
    D3D11RenderSystem& renderSystem,
    D3D11StateManager& stateMngr,
    const ComPtr<ID3D11DeviceContext>& context,
    RenderContextDescriptor desc,
    const std::shared_ptr<Window>& window) :
        renderSystem_   ( renderSystem ),
        stateMngr_      ( stateMngr    ),
        context_        ( context      ),
        desc_           ( desc         )
{
    /* Setup window for the render context */
    SetWindow(window, desc_.videoMode, nullptr);

    /* Create D3D objects */
    CreateSwapChain();
    CreateBackBufferAndRTV();
    CreateDepthStencilAndDSV(desc.videoMode.resolution.x, desc.videoMode.resolution.y);
    SetDefaultRenderTargets();

    /* Initialize viewport */
    auto resolution = desc_.videoMode.resolution.Cast<float>();
    SetViewports({ { 0.0f, 0.0f, resolution.x, resolution.y } });

    /* Initialize v-sync */
    SetVsync(desc_.vsync);
}

void D3D11RenderContext::Present()
{
    swapChain_->Present(swapChainInterval_, 0);
}

/* ----- Configuration ----- */

void D3D11RenderContext::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    // dummy
}

void D3D11RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        //todo

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);
    }
}

void D3D11RenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    desc_.vsync = vsyncDesc;
    swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
}

void D3D11RenderContext::SetViewports(const std::vector<Viewport>& viewports)
{
    stateMngr_.SetViewports(viewports);
}

void D3D11RenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    stateMngr_.SetScissors(scissors);
}

void D3D11RenderContext::SetClearColor(const ColorRGBAf& color)
{
    clearState_.color = color;
}

void D3D11RenderContext::SetClearDepth(float depth)
{
    clearState_.depth = depth;
}

void D3D11RenderContext::SetClearStencil(int stencil)
{
    clearState_.stencil = stencil;
}

void D3D11RenderContext::ClearBuffers(long flags)
{
    /* Clear color buffer */
    if ((flags & ClearBuffersFlags::Color) != 0)
        context_->ClearRenderTargetView(backBufferRTV_.Get(), clearState_.color.Ptr());
    
    /* Clear depth-stencil buffer */
    int dsvClearFlags = 0;

    if ((flags & ClearBuffersFlags::Depth) != 0)
        dsvClearFlags |= D3D11_CLEAR_DEPTH;
    if ((flags & ClearBuffersFlags::Stencil) != 0)
        dsvClearFlags |= D3D11_CLEAR_STENCIL;
        
    if (dsvClearFlags)
        context_->ClearDepthStencilView(backBufferDSV_.Get(), dsvClearFlags, clearState_.depth, clearState_.stencil);
}

/* ----- Hardware Buffers ------ */

void D3D11RenderContext::SetVertexBuffer(VertexBuffer& vertexBuffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D11VertexBuffer&, vertexBuffer);

    ID3D11Buffer* buffers[] = { vertexBufferD3D.hwBuffer.Get() };
    UINT strides[] = { vertexBufferD3D.GetStride() };
    UINT offsets[] = { 0 };

    context_->IASetVertexBuffers(0, 1, buffers, strides, offsets);
}

void D3D11RenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    auto& indexBufferD3D = LLGL_CAST(D3D11IndexBuffer&, indexBuffer);
    context_->IASetIndexBuffer(indexBufferD3D.hwBuffer.Get(), indexBufferD3D.GetFormat(), 0);
}

void D3D11RenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot)
{
    /* Set constant buffer resource to all shader stages */
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, constantBuffer);

    auto resource = constantBufferD3D.hwBuffer.Get();

    context_->VSSetConstantBuffers(slot, 1, &resource);
    context_->HSSetConstantBuffers(slot, 1, &resource);
    context_->DSSetConstantBuffers(slot, 1, &resource);
    context_->GSSetConstantBuffers(slot, 1, &resource);
    context_->PSSetConstantBuffers(slot, 1, &resource);
    context_->CSSetConstantBuffers(slot, 1, &resource);
}

void D3D11RenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    //todo
}

void* D3D11RenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    return nullptr;//todo
}

void D3D11RenderContext::UnmapStorageBuffer()
{
    //todo
}

/* ----- Textures ----- */

void D3D11RenderContext::SetTexture(Texture& texture, unsigned int slot)
{
    /* Set texture resource to all shader stages */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);

    auto resource = textureD3D.GetSRV();

    context_->VSSetShaderResources(slot, 1, &resource);
    context_->PSSetShaderResources(slot, 1, &resource);
    context_->GSSetShaderResources(slot, 1, &resource);
    context_->HSSetShaderResources(slot, 1, &resource);
    context_->DSSetShaderResources(slot, 1, &resource);
    context_->CSSetShaderResources(slot, 1, &resource);
}

void D3D11RenderContext::GenerateMips(Texture& texture)
{
    /* Generate MIP-maps for SRV of specified texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    context_->GenerateMips(textureD3D.GetSRV());
}

/* ----- Sampler States ----- */

void D3D11RenderContext::SetSampler(Sampler& sampler, unsigned int slot)
{
    /* Set sampler state object to all shader stages */
    auto& samplerD3D = LLGL_CAST(D3D11Sampler&, sampler);

    auto resource = samplerD3D.GetSamplerState();

    context_->VSSetSamplers(slot, 1, &resource);
    context_->PSSetSamplers(slot, 1, &resource);
    context_->GSSetSamplers(slot, 1, &resource);
    context_->HSSetSamplers(slot, 1, &resource);
    context_->DSSetSamplers(slot, 1, &resource);
    context_->CSSetSamplers(slot, 1, &resource);
}

/* ----- Render Targets ----- */

void D3D11RenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D11RenderContext::UnsetRenderTarget()
{
    SetDefaultRenderTargets();
}

/* ----- Pipeline States ----- */

void D3D11RenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineD3D = LLGL_CAST(D3D11GraphicsPipeline&, graphicsPipeline);
    graphicsPipelineD3D.Bind(context_.Get());
}

void D3D11RenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineD3D = LLGL_CAST(D3D11ComputePipeline&, computePipeline);
    computePipelineD3D.Bind(context_.Get());
}

/* ----- Queries ----- */

void D3D11RenderContext::BeginQuery(Query& query)
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

void D3D11RenderContext::EndQuery(Query& query)
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

bool D3D11RenderContext::QueryResult(Query& query, std::uint64_t& result)
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

void D3D11RenderContext::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryD3D = LLGL_CAST(D3D11Query&, query);
    context_->SetPredication(queryD3D.GetPredicateObject(), (mode >= RenderConditionMode::WaitInverted));
}

void D3D11RenderContext::EndRenderCondition()
{
    context_->SetPredication(nullptr, FALSE);
}

/* ----- Drawing ----- */

void D3D11RenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    context_->Draw(numVertices, firstVertex);
}

void D3D11RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    context_->DrawIndexed(numVertices, firstIndex, 0);
}

void D3D11RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    context_->DrawIndexed(numVertices, firstIndex, vertexOffset);
}

void D3D11RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D11RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    context_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D11RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D11RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D11RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    context_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void D3D11RenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    context_->Dispatch(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
}

/* ----- Misc ----- */

void D3D11RenderContext::SyncGPU()
{
    context_->Flush();
}


/*
 * ======= Private: =======
 */

void D3D11RenderContext::CreateSwapChain()
{
    /* Create swap chain for window handle */
    NativeHandle wndHandle;
    GetWindow().GetNativeHandle(&wndHandle);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    InitMemory(swapChainDesc);
    {
        swapChainDesc.BufferDesc.Width                      = desc_.videoMode.resolution.x;
        swapChainDesc.BufferDesc.Height                     = desc_.videoMode.resolution.y;
        swapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator      = desc_.vsync.refreshRate;
        swapChainDesc.BufferDesc.RefreshRate.Denominator    = desc_.vsync.interval;
        swapChainDesc.SampleDesc.Count                      = (desc_.antiAliasing.enabled ? std::max(1u, desc_.antiAliasing.samples) : 1);
        swapChainDesc.SampleDesc.Quality                    = 0;
        swapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount                           = 1;//(desc_.videoMode.swapChainMode == SwapChainMode::TripleBuffering ? 2 : 1);
        swapChainDesc.OutputWindow                          = wndHandle.window;
        swapChainDesc.Windowed                              = (desc_.videoMode.fullscreen ? FALSE : TRUE);
    }
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc);
}

void D3D11RenderContext::CreateBackBufferAndRTV()
{
    /* Get back buffer from swap chain */
    backBuffer_.Reset();
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer_));
    DXThrowIfFailed(hr, "failed to get back buffer from D3D11 swap chain");

    /* Create back buffer RTV */
    backBufferRTV_.Reset();
    hr = renderSystem_.GetDevice()->CreateRenderTargetView(backBuffer_.Get(), nullptr, &backBufferRTV_);
    DXThrowIfFailed(hr, "failed to create render-target-view (RTV) for D3D11 back buffer");
}

void D3D11RenderContext::CreateDepthStencilAndDSV(UINT width, UINT height)
{
    renderSystem_.CreateDXDepthStencilAndDSV(
        width,
        height,
        (desc_.antiAliasing.enabled ? std::max(1u, desc_.antiAliasing.samples) : 1),
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        depthStencil_,
        backBufferDSV_
    );
}

void D3D11RenderContext::SetDefaultRenderTargets()
{
    ID3D11RenderTargetView* rtv[] = { backBufferRTV_.Get() };
    context_->OMSetRenderTargets(1, rtv, backBufferDSV_.Get());
}


} // /namespace LLGL



// ================================================================================
