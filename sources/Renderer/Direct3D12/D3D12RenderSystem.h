/*
 * D3D12RenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RENDER_SYSTEM_H
#define LLGL_D3D12_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>

#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "Command/D3D12CommandQueue.h"
#include "Command/D3D12CommandBuffer.h"
#include "Command/D3D12CommandContext.h"
#include "Command/D3D12SignatureFactory.h"

#include "Buffer/D3D12Buffer.h"
#include "Buffer/D3D12BufferArray.h"
#include "Buffer/D3D12StagingBufferPool.h"

#include "Texture/D3D12Texture.h"
#include "Texture/D3D12Sampler.h"
#include "Texture/D3D12RenderTarget.h"

#include "RenderState/D3D12Fence.h"
#include "RenderState/D3D12PipelineCache.h"
#include "RenderState/D3D12PipelineState.h"
#include "RenderState/D3D12PipelineLayout.h"
#include "RenderState/D3D12ResourceHeap.h"
#include "RenderState/D3D12RenderPass.h"
#include "RenderState/D3D12QueryHeap.h"

#include "Shader/D3D12Shader.h"

#include "../VideoAdapter.h"
#include "../ContainerTypes.h"
#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <dxgi1_5.h>


namespace LLGL
{

namespace Direct3D12
{

struct RenderSystemNativeHandle;

} // /namespace Direct3D12

class D3D12SubresourceContext;

class D3D12RenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        D3D12RenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~D3D12RenderSystem();

    public:

        ComPtr<IDXGISwapChain1> CreateDXSwapChain(
            const DXGI_SWAP_CHAIN_DESC1&    swapChainDescDXGI,
            const void*                     nativeWindowHandle,
            std::size_t                     nativeWindowHandleSize
        );

        // Internal fence
        void SignalFenceValue(UINT64& fenceValue);
        void WaitForFenceValue(UINT64 fenceValue);

        // Waits until the GPU has done all previous work.
        void SyncGPU(UINT64& fenceValue);
        void SyncGPU();

        // Returns the feature level of the D3D device.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return device_.GetFeatureLevel();
        }

        // Returns the native ID3D12Device object.
        inline ID3D12Device* GetDXDevice() const
        {
            return device_.GetNative();
        }

        // Returns the device object.
        inline D3D12Device& GetDevice()
        {
            return device_;
        }

        // Returns the device object.
        inline const D3D12Device& GetDevice() const
        {
            return device_;
        }

        // Returns the command signmature factory.
        inline const D3D12SignatureFactory& GetSignatureFactory() const
        {
            return cmdSignatureFactory_;
        }

        // Returns whether the D3D12 device supports tearing (DXGI_FEATURE_PRESENT_ALLOW_TEARING).
        inline bool IsTearingSupported() const
        {
            return tearingSupported_;
        }

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void EnableDebugLayer();

        void CreateFactory(bool debugDevice = false);
        void QueryVideoAdapters(long flags, ComPtr<IDXGIAdapter>& outPreferredAdatper);

        HRESULT CreateDevice(IDXGIAdapter* preferredAdapter, long flags);
        HRESULT QueryDXInterfacesFromNativeHandle(const Direct3D12::RenderSystemNativeHandle& nativeHandle, long flags);

        // Returns the minor version of Direct3D 12.X.
        int GetMinorVersion() const;

        void QueryRendererInfo(RendererInfo& outInfo);
        void QueryRenderingCaps(RenderingCapabilities& outCaps);

        // Close, execute, and reset command list.
        void ExecuteCommandListAndSync();

        void UpdateBufferAndSync(
            D3D12Buffer&    bufferD3D,
            std::uint64_t   offset,
            const void*     data,
            std::uint64_t   dataSize,
            std::uint64_t   alignment   = 256u
        );

        // Maps the range of the specified D3D buffer between GPU and CPU memory space.
        void* MapBufferRange(D3D12Buffer& bufferD3D, const CPUAccess access, std::uint64_t offset, std::uint64_t length);

        // Updates the image data of the specified texture region and converts the source image on-the-fly.
        HRESULT UpdateTextureSubresourceFromImage(
            D3D12Texture&               textureD3D,
            const TextureRegion&        region,
            const ImageView&            imageView,
            D3D12SubresourceContext&    subresourceContext
        );

        const D3D12RenderPass* GetDefaultRenderPass() const;

        bool CheckFactoryFeatureSupport(DXGI_FEATURE feature) const;

    private:

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory4>                   factory_;
        D3D12Device                             device_;
        D3D12CommandContext*                    commandContext_         = nullptr;
        D3D12PipelineLayout                     defaultPipelineLayout_;
        D3D12SignatureFactory                   cmdSignatureFactory_;
        D3D12StagingBufferPool                  stagingBufferPool_;
        bool                                    tearingSupported_       = false;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D12SwapChain>       swapChains_;
        HWObjectInstance<D3D12CommandQueue>     commandQueue_;
        HWObjectContainer<D3D12CommandBuffer>   commandBuffers_;
        HWObjectContainer<D3D12Buffer>          buffers_;
        HWObjectContainer<D3D12BufferArray>     bufferArrays_;
        HWObjectContainer<D3D12Texture>         textures_;
        HWObjectContainer<D3D12Sampler>         samplers_;
        HWObjectContainer<D3D12RenderPass>      renderPasses_;
        HWObjectContainer<D3D12RenderTarget>    renderTargets_;
        HWObjectContainer<D3D12Shader>          shaders_;
        HWObjectContainer<D3D12PipelineLayout>  pipelineLayouts_;
        HWObjectContainer<D3D12PipelineCache>   pipelineCaches_;
        HWObjectContainer<D3D12PipelineState>   pipelineStates_;
        HWObjectContainer<D3D12ResourceHeap>    resourceHeaps_;
        HWObjectContainer<D3D12QueryHeap>       queryHeaps_;
        HWObjectContainer<D3D12Fence>           fences_;

        /* ----- Other members ----- */

        VideoAdapterInfo                        videoAdatperInfo_;

};


} // /namespace LLGL


#endif



// ================================================================================
