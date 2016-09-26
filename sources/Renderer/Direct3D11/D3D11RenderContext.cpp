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
    CreateBackBuffer(desc.videoMode.resolution.x, desc.videoMode.resolution.y);
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
        auto prevVideoMode = GetVideoMode();

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);

        /* Resize back buffer */
        if (!Gs::Equals(prevVideoMode.resolution, videoModeDesc.resolution))
        {
            auto size = videoModeDesc.resolution.Cast<UINT>();
            ResizeBackBuffer(size.x, size.y);
        }

        /* Switch fullscreen mode */
        if (prevVideoMode.fullscreen != videoModeDesc.fullscreen)
            swapChain_->SetFullscreenState(videoModeDesc.fullscreen ? TRUE : FALSE, nullptr);
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
        
    if (dsvClearFlags)
        context_->ClearDepthStencilView(framebufferView_.dsv, dsvClearFlags, clearState_.depth, clearState_.stencil);
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

void D3D11RenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot, long shaderStageFlags)
{
    /* Set constant buffer resource to all shader stages */
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, constantBuffer);
    auto resource = constantBufferD3D.hwBuffer.Get();
    SetConstantBuffersOnStages(slot, 1, &resource, shaderStageFlags);
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

void D3D11RenderContext::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    /* Set texture resource to all shader stages */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    auto resource = textureD3D.GetSRV();
    SetShaderResourcesOnStages(slot, 1, &resource, shaderStageFlags);
}

void D3D11RenderContext::GenerateMips(Texture& texture)
{
    /* Generate MIP-maps for SRV of specified texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    context_->GenerateMips(textureD3D.GetSRV());
}

/* ----- Sampler States ----- */

void D3D11RenderContext::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    /* Set sampler state object to all shader stages */
    auto& samplerD3D = LLGL_CAST(D3D11Sampler&, sampler);
    auto resource = samplerD3D.GetSamplerState();
    SetSamplersOnStages(slot, 1, &resource, shaderStageFlags);
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
        swapChainDesc.BufferCount                           = (desc_.videoMode.swapChainMode == SwapChainMode::TripleBuffering ? 2 : 1);
        swapChainDesc.OutputWindow                          = wndHandle.window;
        swapChainDesc.Windowed                              = (desc_.videoMode.fullscreen ? FALSE : TRUE);
        swapChainDesc.SwapEffect                            = DXGI_SWAP_EFFECT_DISCARD;
    }
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc);
}

void D3D11RenderContext::CreateBackBuffer(UINT width, UINT height)
{
    /* Get back buffer from swap chain */
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer_.colorBuffer));
    DXThrowIfFailed(hr, "failed to get back buffer from D3D11 swap chain");

    /* Create back buffer RTV */
    hr = renderSystem_.GetDevice()->CreateRenderTargetView(backBuffer_.colorBuffer.Get(), nullptr, &backBuffer_.rtv);
    DXThrowIfFailed(hr, "failed to create render-target-view (RTV) for D3D11 back buffer");

    /* Create depth-stencil and DSV */
    renderSystem_.CreateDXDepthStencilAndDSV(
        width,
        height,
        (desc_.antiAliasing.enabled ? std::max(1u, desc_.antiAliasing.samples) : 1),
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        backBuffer_.depthStencil,
        backBuffer_.dsv
    );
}

void D3D11RenderContext::ResizeBackBuffer(UINT width, UINT height)
{
    /* Unset render targets */
    context_->OMSetRenderTargets(0, nullptr, nullptr);

    /* Release buffers */
    backBuffer_.colorBuffer.Reset();
    backBuffer_.rtv.Reset();
    backBuffer_.depthStencil.Reset();
    backBuffer_.dsv.Reset();

    /* Resize swap-chain buffers, let DXGI find out the client area, and preserve buffer count and format */
    auto hr = swapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    DXThrowIfFailed(hr, "failed to resize DXGI swap-chain buffers");

    /* Recreate back buffer and reset default render target */
    CreateBackBuffer(width, height);
    SetDefaultRenderTargets();
}

void D3D11RenderContext::SetDefaultRenderTargets()
{
    framebufferView_.rtvList    = { backBuffer_.rtv.Get() };
    framebufferView_.dsv        = backBuffer_.dsv.Get();
    SubmitFramebufferView();
}

void D3D11RenderContext::SubmitFramebufferView()
{
    context_->OMSetRenderTargets(
        static_cast<UINT>(framebufferView_.rtvList.size()),
        framebufferView_.rtvList.data(),
        framebufferView_.dsv
    );
}

#define SHADERSTAGE_VS(FLAG) ( ((FLAG) & ShaderStageFlags::VertexStage        ) != 0 )
#define SHADERSTAGE_HS(FLAG) ( ((FLAG) & ShaderStageFlags::TessControlStage   ) != 0 )
#define SHADERSTAGE_DS(FLAG) ( ((FLAG) & ShaderStageFlags::TessEvaluationStage) != 0 )
#define SHADERSTAGE_GS(FLAG) ( ((FLAG) & ShaderStageFlags::GeometryStage      ) != 0 )
#define SHADERSTAGE_PS(FLAG) ( ((FLAG) & ShaderStageFlags::FragmentStage      ) != 0 )
#define SHADERSTAGE_CS(FLAG) ( ((FLAG) & ShaderStageFlags::ComputeStage       ) != 0 )

void D3D11RenderContext::SetConstantBuffersOnStages(UINT startSlot, UINT count, ID3D11Buffer* const* buffers, long flags)
{
    if (SHADERSTAGE_VS(flags)) { context_->VSSetConstantBuffers(startSlot, count, buffers); }
    if (SHADERSTAGE_HS(flags)) { context_->HSSetConstantBuffers(startSlot, count, buffers); }
    if (SHADERSTAGE_DS(flags)) { context_->DSSetConstantBuffers(startSlot, count, buffers); }
    if (SHADERSTAGE_GS(flags)) { context_->GSSetConstantBuffers(startSlot, count, buffers); }
    if (SHADERSTAGE_PS(flags)) { context_->PSSetConstantBuffers(startSlot, count, buffers); }
    if (SHADERSTAGE_CS(flags)) { context_->CSSetConstantBuffers(startSlot, count, buffers); }
}

void D3D11RenderContext::SetShaderResourcesOnStages(UINT startSlot, UINT count, ID3D11ShaderResourceView* const* views, long flags)
{
    if (SHADERSTAGE_VS(flags)) { context_->VSSetShaderResources(startSlot, count, views); }
    if (SHADERSTAGE_HS(flags)) { context_->HSSetShaderResources(startSlot, count, views); }
    if (SHADERSTAGE_DS(flags)) { context_->DSSetShaderResources(startSlot, count, views); }
    if (SHADERSTAGE_GS(flags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (SHADERSTAGE_PS(flags)) { context_->GSSetShaderResources(startSlot, count, views); }
    if (SHADERSTAGE_CS(flags)) { context_->CSSetShaderResources(startSlot, count, views); }
}

void D3D11RenderContext::SetSamplersOnStages(UINT startSlot, UINT count, ID3D11SamplerState* const* samplers, long flags)
{
    if (SHADERSTAGE_VS(flags)) { context_->VSSetSamplers(startSlot, count, samplers); }
    if (SHADERSTAGE_HS(flags)) { context_->HSSetSamplers(startSlot, count, samplers); }
    if (SHADERSTAGE_DS(flags)) { context_->DSSetSamplers(startSlot, count, samplers); }
    if (SHADERSTAGE_GS(flags)) { context_->GSSetSamplers(startSlot, count, samplers); }
    if (SHADERSTAGE_PS(flags)) { context_->PSSetSamplers(startSlot, count, samplers); }
    if (SHADERSTAGE_CS(flags)) { context_->CSSetSamplers(startSlot, count, samplers); }
}

#undef SHADERSTAGE_VS
#undef SHADERSTAGE_HS
#undef SHADERSTAGE_DS
#undef SHADERSTAGE_GS
#undef SHADERSTAGE_PS
#undef SHADERSTAGE_CS


} // /namespace LLGL



// ================================================================================
