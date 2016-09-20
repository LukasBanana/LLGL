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
#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"


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
    auto& constantBufferD3D = LLGL_CAST(D3D11ConstantBuffer&, constantBuffer);

    ID3D11Buffer* buffers[] = { constantBufferD3D.hwBuffer.Get() };
    context_->VSSetConstantBuffers(slot, 1, buffers);
    context_->HSSetConstantBuffers(slot, 1, buffers);
    context_->DSSetConstantBuffers(slot, 1, buffers);
    context_->GSSetConstantBuffers(slot, 1, buffers);
    context_->PSSetConstantBuffers(slot, 1, buffers);
    context_->CSSetConstantBuffers(slot, 1, buffers);
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
    //todo
}

void D3D11RenderContext::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ----- */

void D3D11RenderContext::SetSampler(Sampler& sampler, unsigned int slot)
{
    //todo
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
    //todo
}

/* ----- Queries ----- */

void D3D11RenderContext::BeginQuery(Query& query)
{
    //todo
}

void D3D11RenderContext::EndQuery(Query& query)
{
    //todo
}

bool D3D11RenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    return false; //todo
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
    /* Create depth stencil texture */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width               = width;
        texDesc.Height              = height;
        texDesc.MipLevels           = 1;
        texDesc.ArraySize           = 1;
        texDesc.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
        texDesc.SampleDesc.Count    = (desc_.antiAliasing.enabled ? std::max(1u, desc_.antiAliasing.samples) : 1);
        texDesc.SampleDesc.Quality  = 0;
        texDesc.Usage               = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags           = D3D11_BIND_DEPTH_STENCIL;
        texDesc.CPUAccessFlags      = 0;
        texDesc.MiscFlags           = 0;
    }
    auto hr = renderSystem_.GetDevice()->CreateTexture2D(&texDesc, nullptr, &depthStencil_);
    DXThrowIfFailed(hr, "failed to create texture 2D for D3D11 depth-stencil");

    /* Create depth-stencil-view */
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    InitMemory(dsvDesc);
    {
        dsvDesc.Format          = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension   = (desc_.antiAliasing.enabled ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);
    }
    hr = renderSystem_.GetDevice()->CreateDepthStencilView(depthStencil_.Get(), &dsvDesc, &backBufferDSV_);
    DXThrowIfFailed(hr, "failed to create depth-stencil-view (DSV) for D3D11 depth-stencil");
}

void D3D11RenderContext::SetDefaultRenderTargets()
{
    ID3D11RenderTargetView* rtv[] = { backBufferRTV_.Get() };
    context_->OMSetRenderTargets(1, rtv, backBufferDSV_.Get());
}


} // /namespace LLGL



// ================================================================================
