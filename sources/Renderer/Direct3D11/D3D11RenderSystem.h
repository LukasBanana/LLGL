/*
 * D3D11RenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RENDER_SYSTEM_H
#define LLGL_D3D11_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/Container/ArrayView.h>

#include "Command/D3D11CommandQueue.h"
#include "Command/D3D11CommandBuffer.h"
#include "D3D11SwapChain.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"

#include "RenderState/D3D11PipelineState.h"
#include "RenderState/D3D11StateManager.h"
#include "RenderState/D3D11QueryHeap.h"
#include "RenderState/D3D11Fence.h"
#include "RenderState/D3D11ResourceHeap.h"
#include "RenderState/D3D11RenderPass.h"
#include "RenderState/D3D11PipelineLayout.h"

#include "Shader/D3D11Shader.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"

#include "../VideoAdapter.h"
#include "../ContainerTypes.h"
#include "../DXCommon/ComPtr.h"
#include "../ProxyPipelineCache.h"

#include <dxgi.h>

#if LLGL_DEBUG
#include <dxgidebug.h>
#endif

#include "Direct3D11.h"

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
#   include <dxgi1_5.h>
#elif defined LLGL_OS_UWP
#   include <dxgi1_3.h>
#endif


namespace LLGL
{

namespace Direct3D11
{

struct RenderSystemNativeHandle;

} // /namespace Direct3D11


class Module;

class D3D11RenderSystem final : public RenderSystem
{

    public:

        #include <LLGL/Backend/RenderSystem.inl>

    public:

        D3D11RenderSystem(const RenderSystemDescriptor& renderSystemDesc);
        ~D3D11RenderSystem();

    public:

        // Returns a sample descriptor for the specified format.
        static DXGI_SAMPLE_DESC FindSuitableSampleDesc(ID3D11Device* device, DXGI_FORMAT format, UINT maxSampleCount = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT);

        // Returns the least common denominator of a suitable sample descriptor for all formats.
        static DXGI_SAMPLE_DESC FindSuitableSampleDesc(ID3D11Device* device, std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT);

        // Calls ClearState() on all ID3D11DeviceContext objects.
        void ClearStateForAllContexts();

        // Returns the ID3D11Device object.
        inline ID3D11Device* GetDevice() const
        {
            return device_.Get();
        }

        // Returns the selected device feature level.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

        // Returns whether the D3D11 device supports tearing (DXGI_FEATURE_PRESENT_ALLOW_TEARING).
        inline bool IsTearingSupported() const
        {
            return tearingSupported_;
        }

    private:

        #include <LLGL/Backend/RenderSystem.Internal.inl>

    private:

        void CreateFactory();
        void QueryVideoAdapters(long flags, ComPtr<IDXGIAdapter>& outPreferredAdatper);
        HRESULT CreateDevice(IDXGIAdapter* adapter, bool isDebugDevice = false, bool isSoftwareDevice = false);
        HRESULT CreateDeviceWithFlags(IDXGIAdapter* adapter, const ArrayView<D3D_FEATURE_LEVEL>& featureLevels, bool isSoftwareDevice = false, UINT flags = 0);
        HRESULT CreateDeviceWithFlagsAndDriverType(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType, const ArrayView<D3D_FEATURE_LEVEL>& featureLevels, UINT flags);
        HRESULT QueryDXInterfacesFromNativeHandle(const Direct3D11::RenderSystemNativeHandle& nativeHandle);
        void QueryDXDeviceVersion();
        void CreateStateManagerAndCommandQueue();

        void QueryRendererInfo(RendererInfo& outInfo);
        void QueryRenderingCaps(RenderingCapabilities& outCaps);

        // Returns the minor version of Direct3D 11.X.
        int GetMinorVersion() const;

        void InitializeGpuTexture(
            D3D11Texture&               textureD3D,
            const TextureDescriptor&    textureDesc,
            const ImageView*            initialImage
        );

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
        bool CheckFactoryFeatureSupport(DXGI_FEATURE feature) const;
        #endif

        void NotifyBindingTablesOnRelease(D3D11BindingLocator* locator);

    private:

        #if LLGL_DEBUG

        struct LiveObjectReporter
        {
            LiveObjectReporter();
            ~LiveObjectReporter();

            std::unique_ptr<Module> debugModule;
            ComPtr<IDXGIDebug>      debugDevice;
        };

        // Must be declared first, to ensure its destructor is called after all D3D objects are cleared.
        std::unique_ptr<LiveObjectReporter>     liveObjectReporter_;

        #endif

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory>                    factory_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1 || defined LLGL_OS_UWP
        ComPtr<IDXGIFactory1>                   factory1_;
        #endif

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2 || defined LLGL_OS_UWP
        ComPtr<IDXGIFactory2>                   factory2_;
        #endif

        ComPtr<ID3D11Device>                    device_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3D11Device1>                   device1_;
        #endif

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
        ComPtr<ID3D11Device2>                   device2_;
        #endif

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
        ComPtr<ID3D11Device3>                   device3_;
        #endif

        ComPtr<ID3D11DeviceContext>             context_;

        D3D_FEATURE_LEVEL                       featureLevel_           = D3D_FEATURE_LEVEL_9_1;
        bool                                    tearingSupported_       = false;

        std::shared_ptr<D3D11StateManager>      stateMngr_;
        std::vector<D3D11StateManager*>         deferredStateMngrRefs_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D11SwapChain>       swapChains_;
        HWObjectInstance<D3D11CommandQueue>     commandQueue_;
        HWObjectContainer<D3D11CommandBuffer>   commandBuffers_;
        HWObjectContainer<D3D11Buffer>          buffers_;
        HWObjectContainer<D3D11BufferArray>     bufferArrays_;
        HWObjectContainer<D3D11Texture>         textures_;
        HWObjectContainer<D3D11Sampler>         samplers_;
        HWObjectContainer<D3D11RenderPass>      renderPasses_;
        HWObjectContainer<D3D11RenderTarget>    renderTargets_;
        HWObjectContainer<D3D11Shader>          shaders_;
        HWObjectContainer<D3D11PipelineLayout>  pipelineLayouts_;
        HWObjectInstance<ProxyPipelineCache>    pipelineCacheProxy_;
        HWObjectContainer<D3D11PipelineState>   pipelineStates_;
        HWObjectContainer<D3D11ResourceHeap>    resourceHeaps_;
        HWObjectContainer<D3D11QueryHeap>       queryHeaps_;
        HWObjectContainer<D3D11Fence>           fences_;

        /* ----- Other members ----- */

        VideoAdapterInfo                        videoAdatperInfo_;

};


} // /namespace LLGL


#endif



// ================================================================================
