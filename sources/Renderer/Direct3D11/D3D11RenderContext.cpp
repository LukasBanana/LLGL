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
    CreateSwapChain();
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
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
    }
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
    //context_->ClearDepthStencilView();

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
    //todo
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
    //todo
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
        swapChainDesc.BufferCount                           = (desc_.videoMode.swapChainMode == SwapChainMode::TripleBuffering ? 2 : 1);
        swapChainDesc.OutputWindow                          = wndHandle.window;
        swapChainDesc.Windowed                              = (desc_.videoMode.fullscreen ? FALSE : TRUE);
    }
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc);
}


} // /namespace LLGL



// ================================================================================
