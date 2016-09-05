/*
 * D3D12RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "DXCore.h"
#include "../CheckedCast.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"
#include <algorithm>


namespace LLGL
{


D3D12RenderContext::D3D12RenderContext(
    D3D12RenderSystem& renderSystem,
    RenderContextDescriptor desc,
    const std::shared_ptr<Window>& window) :
        renderSystem_   ( renderSystem ),
        desc_           ( desc         )
{
    /* Setup window for the render context */
    SetWindow(window, desc_.videoMode, nullptr);
    CreateWindowSizeDependentResources();
}

D3D12RenderContext::~D3D12RenderContext()
{
    /* Release D3D12 objects */
    for (UINT i = 0; i < maxNumBuffers; ++i)
    {
        SafeRelease(renderTargets_[i]);
        SafeRelease(cmdAllocs_[i]);
    }
    SafeRelease(descHeap_);
    SafeRelease(swapChain_);
}

std::map<RendererInfo, std::string> D3D12RenderContext::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    //todo

    return info;
}

static int GetMaxTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 2048;
}

static int GetMaxCubeTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384;
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 512;
}

static unsigned int GetMaxRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4;
    else                                        return 1;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
RenderingCaps D3D12RenderContext::QueryRenderingCaps() const
{
    RenderingCaps caps;

    auto            featureLevel        = renderSystem_.GetFeatureLevel();
    unsigned int    maxThreadGroups     = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    caps.screenOrigin                   = ScreenOrigin::UpperLeft;
    caps.clippingRange                  = ClippingRange::ZeroToOne;
    caps.hasRenderTargets               = true;
    caps.has3DTextures                  = true;
    caps.hasCubeTextures                = true;
    caps.hasTextureArrays               = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasCubeTextureArrays           = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    caps.hasSamplers                    = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasConstantBuffers             = true;
    caps.hasStorageBuffers              = true;
    caps.hasUniforms                    = false;
    caps.hasGeometryShaders             = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasTessellationShaders         = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    caps.hasComputeShaders              = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.hasInstancing                  = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasOffsetInstancing            = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.hasViewportArrays              = true;
    caps.hasConservativeRasterization   = (featureLevel >= D3D_FEATURE_LEVEL_11_1);
    caps.maxNumTextureArrayLayers       = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048 : 256);
    caps.maxNumRenderTargetAttachments  = GetMaxRenderTargets(featureLevel);
    caps.maxConstantBufferSize          = 16384;
    caps.max1DTextureSize               = GetMaxTextureDimension(featureLevel);
    caps.max2DTextureSize               = GetMaxTextureDimension(featureLevel);
    caps.max3DTextureSize               = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048 : 256);
    caps.maxCubeTextureSize             = GetMaxCubeTextureDimension(featureLevel);
    caps.maxAnisotropy                  = (featureLevel >= D3D_FEATURE_LEVEL_9_2 ? 16 : 2);
    caps.maxNumComputeShaderWorkGroups  = { maxThreadGroups, maxThreadGroups, (featureLevel >= D3D_FEATURE_LEVEL_11_0 ? maxThreadGroups : 1u) };
    caps.maxComputeShaderWorkGroupSize  = { 1024, 1024, 1024 };

    return caps;
}

ShadingLanguage D3D12RenderContext::QueryShadingLanguage() const
{
    return ShadingLanguage::HLSL_5_0;
}

void D3D12RenderContext::Present()
{
    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present D3D12 swap-chain");
}

/* ----- Configuration ----- */

void D3D12RenderContext::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    // dummy
}

void D3D12RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        //todo

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);
    }
}

void D3D12RenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        //SetupVsyncInterval();
    }
}

void D3D12RenderContext::SetViewports(const std::vector<Viewport>& viewports)
{
    //todo
}

void D3D12RenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    //todo
}

void D3D12RenderContext::SetClearColor(const ColorRGBAf& color)
{
    //todo
}

void D3D12RenderContext::SetClearDepth(float depth)
{
    //todo
}

void D3D12RenderContext::SetClearStencil(int stencil)
{
    //todo
}

void D3D12RenderContext::ClearBuffers(long flags)
{
    //todo
}

void D3D12RenderContext::SetDrawMode(const DrawMode drawMode)
{
    //todo
}

/* ----- Hardware Buffers ------ */

void D3D12RenderContext::BindVertexBuffer(VertexBuffer& vertexBuffer)
{
    //todo
}

void D3D12RenderContext::UnbindVertexBuffer()
{
    //todo
}

void D3D12RenderContext::BindIndexBuffer(IndexBuffer& indexBuffer)
{
    //todo
}

void D3D12RenderContext::UnbindIndexBuffer()
{
    //todo
}

void D3D12RenderContext::BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index)
{
    //todo
}

void D3D12RenderContext::UnbindConstantBuffer(unsigned int index)
{
    //todo
}

/* ----- Textures ----- */

void D3D12RenderContext::BindTexture(unsigned int layer, Texture& texture)
{
    //todo
}

void D3D12RenderContext::UnbindTexture(unsigned int layer)
{
    //todo
}

void D3D12RenderContext::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ----- */

void D3D12RenderContext::BindSampler(unsigned int layer, Sampler& sampler)
{
    //todo
}

void D3D12RenderContext::UnbindSampler(unsigned int layer)
{
    //todo
}

/* ----- Render Targets ----- */

void D3D12RenderContext::BindRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D12RenderContext::UnbindRenderTarget()
{
    //todo
}

/* ----- Pipeline States ----- */

void D3D12RenderContext::BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    //todo
}

/* ----- Drawing ----- */

void D3D12RenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    //todo
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    //todo
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    //todo
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    //todo
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    //todo
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    //todo
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    //todo
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    //todo
}

/* ----- Compute ----- */

void D3D12RenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    //todo
}


/*
 * ======= Private: =======
 */

void D3D12RenderContext::SyncGPU()
{
    renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}

void D3D12RenderContext::CreateWindowSizeDependentResources()
{
    /* Wait until all previous GPU work is complete */
    SyncGPU();

    /* Setup swap chain meta data */
    numFrames_ = static_cast<UINT>(desc_.videoMode.swapChainMode);

    /* Create swap chain for window handle */
    NativeHandle wndHandle;
    GetWindow().GetNativeHandle(&wndHandle);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    InitMemory(swapChainDesc);
    {
        swapChainDesc.Width                 = static_cast<UINT>(desc_.videoMode.resolution.x);
        swapChainDesc.Height                = static_cast<UINT>(desc_.videoMode.resolution.y);
        swapChainDesc.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo                = false;
        swapChainDesc.SampleDesc.Count      = (desc_.antiAliasing.enabled ? desc_.antiAliasing.samples : 1);
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount           = numFrames_;
        swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Flags                 = 0;
        swapChainDesc.Scaling               = DXGI_SCALING_NONE;
        swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
    }
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc, wndHandle.window);

    /* Create descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    InitMemory(descHeapDesc);
    {
        descHeapDesc.NumDescriptors = numFrames_;
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }
    descHeap_ = renderSystem_.CreateDXDescriptorHeap(descHeapDesc);
    descHeap_->SetName(L"Render Target View Descriptor Heap");

    /* Update tracked fence values */
    for (UINT i = 0; i < numFrames_; ++i)
        fenceValues_[i] = fenceValues_[currentFrame_];

    /* Create render targets */
    //todo...


}

void D3D12RenderContext::SetupSwapChainInterval(const VsyncDescriptor& desc)
{
    swapChainInterval_ = (desc.enabled ? std::max(1u, std::min(desc.interval, 4u)) : 0);
}


} // /namespace LLGL



// ================================================================================
