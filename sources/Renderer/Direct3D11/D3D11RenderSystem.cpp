/*
 * D3D11RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "D3D11ResourceFlags.h"
#include "Texture/D3D11MipGenerator.h"
#include "Shader/D3D11BuiltinShaderFactory.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../TextureUtils.h"
#include "../RenderSystemUtils.h"
#include "../../Core/Vendor.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/StringUtils.h"
#include "../../Core/Assertion.h"
#include "../../Platform/Module.h"
#include <limits.h>

#include "Command/D3D11PrimaryCommandBuffer.h"
#include "Command/D3D11SecondaryCommandBuffer.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11BufferWithRV.h"

#include "RenderState/D3D11GraphicsPSO.h"
#include "RenderState/D3D11GraphicsPSO1.h"
#include "RenderState/D3D11GraphicsPSO3.h"
#include "RenderState/D3D11ComputePSO.h"

#include "Shader/D3D11CommonShader.h"
#include "Shader/D3D11DomainShader.h"
#include "Shader/D3D11VertexShader.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>


namespace LLGL
{


D3D11RenderSystem::D3D11RenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    const bool debugDevice = ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0);

    if (auto* customNativeHandle = GetRendererNativeHandle<Direct3D11::RenderSystemNativeHandle>(renderSystemDesc))
    {
        /* Query all DXGI interfaces from native handle */
        HRESULT hr = QueryDXInterfacesFromNativeHandle(*customNativeHandle);
        DXThrowIfFailed(hr, "failed to query D3D11 device from custom native handle");
    }
    else
    {
        /* Create DXGU factory, query video adapters, and create D3D11 device */
        CreateFactory();

        ComPtr<IDXGIAdapter> preferredAdatper;
        QueryVideoAdapters(renderSystemDesc.flags, preferredAdatper);

        HRESULT hr = CreateDevice(preferredAdatper.Get(), debugDevice);
        DXThrowIfFailed(hr, "failed to create D3D11 device");
    }

    #if LLGL_DEBUG
    if (debugDevice)
        liveObjectReporter_ = std::unique_ptr<LiveObjectReporter>(new LiveObjectReporter());
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    /* Query tearing feature support */
    tearingSupported_ = CheckFactoryFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING);
    #endif

    /* Initialize states and renderer information */
    CreateStateManagerAndCommandQueue();

    /* Initialize MIP-map generator singleton */
    D3D11MipGenerator::Get().InitializeDevice(device_);
    D3D11BuiltinShaderFactory::Get().CreateBuiltinShaders(device_.Get());
}

D3D11RenderSystem::~D3D11RenderSystem()
{
    /* Release resource of singletons first */
    D3D11MipGenerator::Get().Clear();
    D3D11BuiltinShaderFactory::Get().Clear();
}

/* ----- Swap-chain ----- */

SwapChain* D3D11RenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<D3D11SwapChain>(factory_.Get(), device_, *this, swapChainDesc, surface);
}

void D3D11RenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* D3D11RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D11RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    if ((commandBufferDesc.flags & (CommandBufferFlags::ImmediateSubmit)) != 0)
    {
        /* Create command buffer with immediate context */
        return commandBuffers_.emplace<D3D11PrimaryCommandBuffer>(device_.Get(), context_, stateMngr_, commandBufferDesc);
    }
    else if ((commandBufferDesc.flags & (CommandBufferFlags::Secondary)) != 0)
    {
        /* Create secondary command buffer with virtual buffer */
        return commandBuffers_.emplace<D3D11SecondaryCommandBuffer>(commandBufferDesc);
    }
    else
    {
        /* Create deferred D3D11 device context */
        ComPtr<ID3D11DeviceContext> deferredContext;
        HRESULT hr = device_->CreateDeferredContext(0, deferredContext.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11DeviceContext", "for deferred command buffer");

        /* Create state manager dedicated to deferred context */
        std::shared_ptr<D3D11StateManager> deferredStateMngr = std::make_shared<D3D11StateManager>(device_.Get(), deferredContext);

        /* Store references to unique state manager - we need to notify all binding tables on resource release */
        deferredStateMngrRefs_.push_back(deferredStateMngr.get());

        /* Create command buffer with deferred context and dedicated state manager */
        return commandBuffers_.emplace<D3D11PrimaryCommandBuffer>(device_.Get(), deferredContext, std::move(deferredStateMngr), commandBufferDesc);
    }
}

void D3D11RenderSystem::Release(CommandBuffer& commandBuffer)
{
    auto& commandBufferD3D = LLGL_CAST(D3D11CommandBuffer&, commandBuffer);
    if (!commandBufferD3D.IsSecondaryCmdBuffer())
    {
        /* If this is command buffer has a unique state manager, remove it from the list of deferred state managers */
        auto& primaryCmdBufferD3D = LLGL_CAST(D3D11PrimaryCommandBuffer&, commandBufferD3D);
        if (primaryCmdBufferD3D.GetStateManagerPtr() != stateMngr_.get())
        {
            RemoveFromListIf(
                deferredStateMngrRefs_,
                [stateMngr = primaryCmdBufferD3D.GetStateManagerPtr()](D3D11StateManager* entry) -> bool
                {
                    return (entry == stateMngr);
                }
            );
        }
    }
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

Buffer* D3D11RenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, UINT_MAX);
    if (DXBindFlagsNeedBufferWithRV(bufferDesc.bindFlags))
        return buffers_.emplace<D3D11BufferWithRV>(device_.Get(), bufferDesc, initialData);
    else
        return buffers_.emplace<D3D11Buffer>(device_.Get(), bufferDesc, initialData);
}

BufferArray* D3D11RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<D3D11BufferArray>(numBuffers, bufferArray);
}

void D3D11RenderSystem::Release(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    NotifyBindingTablesOnRelease(bufferD3D.GetBindingLocator());
    buffers_.erase(&buffer);
}

void D3D11RenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void D3D11RenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.WriteSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void D3D11RenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.ReadSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    return bufferD3D.Map(context_.Get(), access, 0, bufferD3D.GetSize());
}

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    return bufferD3D.Map(context_.Get(), access, static_cast<UINT>(offset), static_cast<UINT>(length));
}

void D3D11RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.Unmap(context_.Get());
}

/* ----- Textures ----- */

Texture* D3D11RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    /* Create texture object */
    auto* textureD3D = textures_.emplace<D3D11Texture>(device_.Get(), textureDesc);

    /* Initialize texture data with or without initial image data */
    InitializeGpuTexture(*textureD3D, textureDesc, initialImage);

    /* Generate MIP-maps if enabled */
    if (initialImage != nullptr && MustGenerateMipsOnCreate(textureDesc))
        D3D11MipGenerator::Get().GenerateMips(context_.Get(), *textureD3D);

    return textureD3D;
}

void D3D11RenderSystem::Release(Texture& texture)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    NotifyBindingTablesOnRelease(textureD3D.GetBindingLocator());
    textures_.erase(&texture);
}

void D3D11RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                textureRegion.subresource.numArrayLayers,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.extent.width
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                textureRegion.subresource.numArrayLayers,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.extent.width,
                    textureRegion.extent.height
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            /* Multi-sampled textures cannot be written by CPU */
            break;

        case TextureType::Texture3D:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                0,
                1,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.offset.z,
                    textureRegion.extent.width,
                    textureRegion.extent.height,
                    textureRegion.extent.depth
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;
    }
}

void D3D11RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    if (dstImageView.data == nullptr)
        return /*E_INVALIDARG*/;

    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);

    /* Map subresource for reading */
    const Format            format              = textureD3D.GetFormat();
    const Extent3D          extent              = CalcTextureExtent(textureD3D.GetType(), textureRegion.extent);
    const std::uint32_t     numTexelsPerLayer   = extent.width * extent.height * extent.depth;
    const std::uint32_t     numTexelsTotal      = numTexelsPerLayer * textureRegion.subresource.numArrayLayers;
    const std::size_t       requiredImageSize   = GetMemoryFootprint(dstImageView.format, dstImageView.dataType, numTexelsTotal);

    if (dstImageView.dataSize < requiredImageSize)
        return /*E_BOUNDS*/;

    /* Create a copy of the hardware texture with CPU read access */
    ComPtr<ID3D11Resource> texCopy;
    textureD3D.CreateSubresourceCopyWithCPUAccess(device_.Get(), context_.Get(), texCopy, D3D11_CPU_ACCESS_READ, textureRegion);

    MutableImageView        intermediateDstView = dstImageView;
    const FormatAttributes& formatAttribs       = GetFormatAttribs(format);

    for_range(arrayLayer, textureRegion.subresource.numArrayLayers)
    {
        const UINT subresource = D3D11CalcSubresource(0, arrayLayer, 1);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        HRESULT hr = context_->Map(texCopy.Get(), subresource, D3D11_MAP_READ, 0, &mappedSubresource);
        DXThrowIfFailed(hr, "failed to map D3D11 texture copy resource");

        /* Copy host visible resource to CPU accessible resource */
        const ImageView intermediateSrcView{ formatAttribs.format, formatAttribs.dataType, mappedSubresource.pData, mappedSubresource.DepthPitch };
        const std::size_t bytesWritten = RenderSystem::CopyTextureImageData(intermediateDstView, intermediateSrcView, numTexelsPerLayer, extent.width, mappedSubresource.RowPitch);

        /* Unmap resource */
        context_->Unmap(texCopy.Get(), subresource);

        /* Move destination image pointer to next layer */
        intermediateDstView.data = reinterpret_cast<char*>(intermediateDstView.data) + bytesWritten;
    }
}

/* ----- Sampler States ---- */

Sampler* D3D11RenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<D3D11Sampler>(device_.Get(), samplerDesc);
}

void D3D11RenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D11RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<D3D11ResourceHeap>(resourceHeapDesc, initialResourceViews);
}

void D3D11RenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t D3D11RenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);
    return resourceHeapD3D.WriteResourceViews(firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* D3D11RenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<D3D11RenderPass>(renderPassDesc);
}

void D3D11RenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D11RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<D3D11RenderTarget>(device_.Get(), renderTargetDesc);
}

void D3D11RenderSystem::Release(RenderTarget& renderTarget)
{
    auto& renderTargetD3D = LLGL_CAST(D3D11RenderTarget&, renderTarget);
    const D3D11RenderTargetHandles& rtHandles = renderTargetD3D.GetRenderTargetHandles();
    NotifyBindingTablesOnRelease(rtHandles.GetDepthStencilLocator());
    for_range(i, rtHandles.GetNumRenderTargetViews())
        NotifyBindingTablesOnRelease(rtHandles.GetRenderTargetLocators()[i]);
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* D3D11RenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    RenderSystem::AssertCreateShader(shaderDesc);
    switch (shaderDesc.type)
    {
        case ShaderType::Vertex:
            return shaders_.emplace<D3D11VertexShader>(device_.Get(), shaderDesc);
        case ShaderType::TessEvaluation:
            return shaders_.emplace<D3D11DomainShader>(device_.Get(), shaderDesc);
        default:
            return shaders_.emplace<D3D11CommonShader>(device_.Get(), shaderDesc);
    }
}

void D3D11RenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D11RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<D3D11PipelineLayout>(device_.Get(), pipelineLayoutDesc);
}

void D3D11RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* D3D11RenderSystem::CreatePipelineCache(const Blob& /*initialBlob*/)
{
    return ProxyPipelineCache::CreateInstance(pipelineCacheProxy_);
}

void D3D11RenderSystem::Release(PipelineCache& pipelineCache)
{
    ProxyPipelineCache::ReleaseInstance(pipelineCacheProxy_, pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* D3D11RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    if (device3_)
    {
        /* Create graphics pipeline for Direct3D 11.3 */
        return pipelineStates_.emplace<D3D11GraphicsPSO3>(device3_.Get(), pipelineStateDesc);
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
    if (device2_)
    {
        /* Create graphics pipeline for Direct3D 11.1 (there is no dedicated class for 11.2) */
        return pipelineStates_.emplace<D3D11GraphicsPSO1>(device2_.Get(), pipelineStateDesc);
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (device1_)
    {
        /* Create graphics pipeline for Direct3D 11.1 */
        return pipelineStates_.emplace<D3D11GraphicsPSO1>(device1_.Get(), pipelineStateDesc);
    }
    #endif

    /* Create graphics pipeline for Direct3D 11.0 */
    return pipelineStates_.emplace<D3D11GraphicsPSO>(device_.Get(), pipelineStateDesc);
}

PipelineState* D3D11RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    return pipelineStates_.emplace<D3D11ComputePSO>(pipelineStateDesc);
}

void D3D11RenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D11RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<D3D11QueryHeap>(device_.Get(), queryHeapDesc);
}

void D3D11RenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* D3D11RenderSystem::CreateFence()
{
    return fences_.emplace<D3D11Fence>(device_.Get());
}

void D3D11RenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool D3D11RenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D11::RenderSystemNativeHandle))
    {
        auto* nativeHandleD3D = reinterpret_cast<Direct3D11::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleD3D->device = device_.Get();
        nativeHandleD3D->device->AddRef();
        nativeHandleD3D->deviceContext = context_.Get();
        nativeHandleD3D->deviceContext->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Internal: =======
 */

DXGI_SAMPLE_DESC D3D11RenderSystem::FindSuitableSampleDesc(ID3D11Device* device, DXGI_FORMAT format, UINT maxSampleCount)
{
    for (; maxSampleCount > 1; --maxSampleCount)
    {
        UINT numQualityLevels = 0;
        if (device->CheckMultisampleQualityLevels(format, maxSampleCount, &numQualityLevels) == S_OK)
        {
            if (numQualityLevels > 0)
                return { maxSampleCount, numQualityLevels - 1 };
        }
    }
    return { 1u, 0u };
}

DXGI_SAMPLE_DESC D3D11RenderSystem::FindSuitableSampleDesc(ID3D11Device* device, std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount)
{
    DXGI_SAMPLE_DESC sampleDesc = { maxSampleCount, 0 };

    for_range(i, numFormats)
    {
        if (formats[i] != DXGI_FORMAT_UNKNOWN)
            sampleDesc = D3D11RenderSystem::FindSuitableSampleDesc(device, formats[i], sampleDesc.Count);
    }

    return sampleDesc;
}

void D3D11RenderSystem::ClearStateForAllContexts()
{
    stateMngr_->ClearState();
    for (const auto& cmdBuffer : commandBuffers_)
    {
        if (!cmdBuffer->IsSecondaryCmdBuffer())
        {
            auto& primaryCmdBufferD3D = LLGL_CAST(D3D11PrimaryCommandBuffer&, *cmdBuffer);
            primaryCmdBufferD3D.ClearStateAndResetDeferredCommandList();
        }
    }
}


/*
 * ======= Private: =======
 */

bool D3D11RenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
        QueryRendererInfo(*outInfo);
    if (outCaps != nullptr)
        QueryRenderingCaps(*outCaps);
    return true;
}

void D3D11RenderSystem::CreateFactory()
{
    /* Create DXGI factory */
    HRESULT hr = S_OK;

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2 || defined LLGL_OS_UWP
    hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory2_));
    if (SUCCEEDED(hr))
    {
        factory2_.As(&factory_);
        factory2_.As(&factory1_);
        return;
    }
    #endif

    #ifdef LLGL_OS_UWP
    DXThrowIfCreateFailed(hr, "IDXGIFactory2");
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory1_));
    if (SUCCEEDED(hr))
    {
        factory1_.As(&factory_);
        return;
    }
    #endif

    #ifdef LLGL_OS_WIN32
    hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
    DXThrowIfCreateFailed(hr, "IDXGIFactory");
    #endif
}

void D3D11RenderSystem::QueryVideoAdapters(long flags, ComPtr<IDXGIAdapter>& outPreferredAdatper)
{
    videoAdatperInfo_ = DXGetVideoAdapterInfo(factory_.Get(), flags, outPreferredAdatper.ReleaseAndGetAddressOf());
}

HRESULT D3D11RenderSystem::CreateDevice(IDXGIAdapter* adapter, bool debugDevice)
{
    /* Find list of feature levels to select from, and statically determine maximal feature level */
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        D3D_FEATURE_LEVEL_11_1,
        #endif
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    HRESULT hr = 0;

    if (debugDevice)
    {
        /* Try to create device with debug layer (only supported if Windows 8.1 SDK is installed) */
        hr = CreateDeviceWithFlags(adapter, featureLevels, D3D11_CREATE_DEVICE_DEBUG);
        if (FAILED(hr))
            hr = CreateDeviceWithFlags(adapter, featureLevels, 0);
    }
    else
    {
        /* Create device without debug layer */
        hr = CreateDeviceWithFlags(adapter, featureLevels, 0);
    }

    /* Try to create device with default adapter if preferred one failed */
    if (FAILED(hr) && adapter != nullptr)
    {
        /* Update video adapter info with default adapter */
        videoAdatperInfo_ = DXGetVideoAdapterInfo(factory_.Get());
        hr = CreateDeviceWithFlags(nullptr, featureLevels, 0);
    }

    if (FAILED(hr))
        return hr;

    QueryDXDeviceVersion();

    return S_OK;
}

HRESULT D3D11RenderSystem::CreateDeviceWithFlags(IDXGIAdapter* adapter, const ArrayView<D3D_FEATURE_LEVEL>& featureLevels, UINT flags)
{
    HRESULT hr = S_OK;

    for (D3D_DRIVER_TYPE driver : { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE })
    {
        hr = D3D11CreateDevice(
            adapter,                                    // Video adapter
            driver,                                     // Driver type
            0,                                          // Software rasterizer module (none)
            flags,                                      // Flags
            featureLevels.data(),                       // Feature level
            static_cast<UINT>(featureLevels.size()),    // Num feature levels
            D3D11_SDK_VERSION,                          // SDK version
            device_.ReleaseAndGetAddressOf(),           // Output device
            &featureLevel_,                             // Output feature level
            context_.ReleaseAndGetAddressOf()           // Output device context
        );
        if (SUCCEEDED(hr))
            break;
    }

    return hr;
}

HRESULT D3D11RenderSystem::QueryDXInterfacesFromNativeHandle(const Direct3D11::RenderSystemNativeHandle& nativeHandle)
{
    LLGL_ASSERT_PTR(nativeHandle.device);
    LLGL_ASSERT_PTR(nativeHandle.deviceContext);

    /* Adopt custom native handles */
    device_         = nativeHandle.device;
    context_        = nativeHandle.deviceContext;
    featureLevel_   = device_->GetFeatureLevel();

    QueryDXDeviceVersion();

    /* Query factory and video adapter information */
    ComPtr<IDXGIDevice> dxgiDevice;
    HRESULT hr = device_->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    DXThrowIfFailed(hr, "failed to query interface IDXGIDevice from custom native handle");

    /* Get DXGI adapter and get video adapter information */
    ComPtr<IDXGIAdapter> adapter;
    hr = dxgiDevice->GetAdapter(&adapter);
    DXThrowIfFailed(hr, "failed to get adapter from DXGI device");

    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    hr = adapter->GetDesc(&dxgiAdapterDesc);
    DXThrowIfFailed(hr, "failed to get descriptor from DXGI adapter");

    DXConvertVideoAdapterInfo(adapter.Get(), dxgiAdapterDesc, videoAdatperInfo_);

    /* Get DXGI factory */
    hr = adapter->GetParent(IID_PPV_ARGS(&factory_));
    DXThrowIfFailed(hr, "failed to get parent factory from DXGI adapter");

    return S_OK;
}

void D3D11RenderSystem::QueryDXDeviceVersion()
{
    LLGL_ASSERT_PTR(device_);

    /* Try to get an extended D3D11 device */
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    HRESULT hr = device_->QueryInterface(IID_PPV_ARGS(&device3_));
    if (FAILED(hr))
    #endif
    {
        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
        hr = device_->QueryInterface(IID_PPV_ARGS(&device2_));
        if (FAILED(hr))
        #endif
        {
            #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
            device_->QueryInterface(IID_PPV_ARGS(&device1_));
            #endif
        }
    }
}

void D3D11RenderSystem::CreateStateManagerAndCommandQueue()
{
    stateMngr_ = std::make_shared<D3D11StateManager>(device_.Get(), context_);
    commandQueue_ = MakeUnique<D3D11CommandQueue>(device_.Get(), context_, stateMngr_);
}

static const char* DXFeatureLevelToShaderModel(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
        case D3D_FEATURE_LEVEL_11_1:    /* pass - 5.1 only supported on D3D12 */
        case D3D_FEATURE_LEVEL_11_0:    return "5.0";
        case D3D_FEATURE_LEVEL_10_1:    return "4.1";
        case D3D_FEATURE_LEVEL_10_0:    return "4.0";
        case D3D_FEATURE_LEVEL_9_3:     return "3.0";
        case D3D_FEATURE_LEVEL_9_2:     return "2.0b";
        case D3D_FEATURE_LEVEL_9_1:     return "2.0a";
    }
    return "";
}

void D3D11RenderSystem::QueryRendererInfo(RendererInfo& info)
{
    /* Initialize Direct3D version string */
    const int minorVersion = GetMinorVersion();
    switch (minorVersion)
    {
        case 3:
            info.rendererName = "Direct3D 11.3";
            break;
        case 2:
            info.rendererName = "Direct3D 11.2";
            break;
        case 1:
            info.rendererName = "Direct3D 11.1";
            break;
        default:
            info.rendererName = "Direct3D 11.0";
            break;
    }

    /* Initialize HLSL version string */
    info.shadingLanguageName = "HLSL " + std::string(DXFeatureLevelToShaderModel(GetFeatureLevel()));

    /* Initialize video adapter strings */
    info.deviceName = videoAdatperInfo_.name.c_str();
    info.vendorName = GetVendorName(videoAdatperInfo_.vendor);
}

// Returns the HLSL version for the specified Direct3D feature level.
static std::vector<ShadingLanguage> DXGetHLSLVersions(D3D_FEATURE_LEVEL featureLevel)
{
    std::vector<ShadingLanguage> languages;

    languages.push_back(ShadingLanguage::HLSL);
    languages.push_back(ShadingLanguage::HLSL_2_0);

    if (featureLevel >= D3D_FEATURE_LEVEL_9_1 ) { languages.push_back(ShadingLanguage::HLSL_2_0a); }
    if (featureLevel >= D3D_FEATURE_LEVEL_9_2 ) { languages.push_back(ShadingLanguage::HLSL_2_0b); }
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) { languages.push_back(ShadingLanguage::HLSL_3_0);  }
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) { languages.push_back(ShadingLanguage::HLSL_4_0);  }
    if (featureLevel >= D3D_FEATURE_LEVEL_10_1) { languages.push_back(ShadingLanguage::HLSL_4_1);  }
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) { languages.push_back(ShadingLanguage::HLSL_5_0);  }
    if (featureLevel >= D3D_FEATURE_LEVEL_12_0) { languages.push_back(ShadingLanguage::HLSL_5_1);  }

    return languages;
}

static std::vector<Format> GetDefaultSupportedDXTextureFormats(D3D_FEATURE_LEVEL featureLevel)
{
    std::vector<Format> formats;

    std::size_t numFormats = 0;
    DXGetDefaultSupportedTextureFormats(nullptr, &numFormats);

    formats.resize(numFormats, Format::Undefined);
    DXGetDefaultSupportedTextureFormats(formats.data(), nullptr);

    if (featureLevel >= D3D_FEATURE_LEVEL_10_0)
    {
        formats.insert(
            formats.end(),
            { Format::BC4UNorm, Format::BC4SNorm, Format::BC5UNorm, Format::BC5SNorm }
        );
    }

    return formats;
}

static std::uint32_t GetMaxTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384; // D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;  // D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 2048;
}

static std::uint32_t GetMaxCubeTextureDimension(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_11_0) return 16384; // D3D11_REQ_TEXTURECUBE_DIMENSION
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8192;  // D3D10_REQ_TEXTURECUBE_DIMENSION
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4096;
    else                                        return 512;
}

static std::uint32_t GetMaxRenderTargets(D3D_FEATURE_LEVEL featureLevel)
{
    if (featureLevel >= D3D_FEATURE_LEVEL_10_0) return 8;
    if (featureLevel >= D3D_FEATURE_LEVEL_9_3 ) return 4;
    else                                        return 1;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476876(v=vs.85).aspx
void D3D11RenderSystem::QueryRenderingCaps(RenderingCapabilities& caps)
{
    const D3D_FEATURE_LEVEL featureLevel = GetFeatureLevel();
    const int minorVersion = GetMinorVersion();

    const std::uint32_t maxThreadGroups = 65535u;//D3D11_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    /* Query common attributes */
    caps.screenOrigin                               = ScreenOrigin::UpperLeft;
    caps.clippingRange                              = ClippingRange::ZeroToOne;
    caps.shadingLanguages                           = DXGetHLSLVersions(featureLevel);
    caps.textureFormats                             = GetDefaultSupportedDXTextureFormats(featureLevel);

    caps.features.hasRenderTargets                  = true;
    caps.features.has3DTextures                     = true;
    caps.features.hasCubeTextures                   = true;
    caps.features.hasArrayTextures                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasCubeArrayTextures              = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    caps.features.hasMultiSampleTextures            = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasMultiSampleArrayTextures       = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasTextureViews                   = true;
    caps.features.hasTextureViewSwizzle             = false; // not supported by D3D11
    caps.features.hasBufferViews                    = true;
    caps.features.hasConstantBuffers                = true;
    caps.features.hasStorageBuffers                 = true;
    caps.features.hasGeometryShaders                = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasTessellationShaders            = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    caps.features.hasTessellatorStage               = (featureLevel >= D3D_FEATURE_LEVEL_11_0);
    caps.features.hasComputeShaders                 = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasInstancing                     = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.features.hasOffsetInstancing               = (featureLevel >= D3D_FEATURE_LEVEL_9_3);
    caps.features.hasIndirectDrawing                = (featureLevel >= D3D_FEATURE_LEVEL_10_0);//???
    caps.features.hasViewportArrays                 = true;
    caps.features.hasConservativeRasterization      = (minorVersion >= 3);
    caps.features.hasStreamOutputs                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasLogicOp                        = (featureLevel >= D3D_FEATURE_LEVEL_11_1);
    caps.features.hasPipelineStatistics             = true;
    caps.features.hasRenderCondition                = true;

    /* Query limits */
    caps.limits.lineWidthRange[0]                   = 1.0f;
    caps.limits.lineWidthRange[1]                   = 1.0f;
    caps.limits.maxTextureArrayLayers               = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u);
    caps.limits.maxColorAttachments                 = GetMaxRenderTargets(featureLevel);
    caps.limits.maxPatchVertices                    = 32u;
    caps.limits.max1DTextureSize                    = GetMaxTextureDimension(featureLevel);
    caps.limits.max2DTextureSize                    = GetMaxTextureDimension(featureLevel);
    caps.limits.max3DTextureSize                    = (featureLevel >= D3D_FEATURE_LEVEL_10_0 ? 2048u : 256u);
    caps.limits.maxCubeTextureSize                  = GetMaxCubeTextureDimension(featureLevel);
    caps.limits.maxAnisotropy                       = (featureLevel >= D3D_FEATURE_LEVEL_9_2 ? 16u : 2u);
    caps.limits.maxComputeShaderWorkGroups[0]       = maxThreadGroups;
    caps.limits.maxComputeShaderWorkGroups[1]       = maxThreadGroups;
    caps.limits.maxComputeShaderWorkGroups[2]       = (featureLevel >= D3D_FEATURE_LEVEL_11_0 ? maxThreadGroups : 1u);
    caps.limits.maxComputeShaderWorkGroupSize[0]    = 1024u;
    caps.limits.maxComputeShaderWorkGroupSize[1]    = 1024u;
    caps.limits.maxComputeShaderWorkGroupSize[2]    = 1024u;
    caps.limits.maxViewports                        = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    caps.limits.maxViewportSize[0]                  = D3D11_VIEWPORT_BOUNDS_MAX;
    caps.limits.maxViewportSize[1]                  = D3D11_VIEWPORT_BOUNDS_MAX;
    caps.limits.maxBufferSize                       = UINT_MAX;
    caps.limits.maxConstantBufferSize               = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    caps.limits.maxStreamOutputs                    = 4u;
    caps.limits.maxTessFactor                       = 64u;
    caps.limits.minConstantBufferAlignment          = 256u;
    caps.limits.minSampledBufferAlignment           = 32u;
    caps.limits.minStorageBufferAlignment           = 32u;
    caps.limits.maxColorBufferSamples               = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM).Count;
    caps.limits.maxDepthBufferSamples               = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_D32_FLOAT).Count;
    caps.limits.maxStencilBufferSamples             = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_D32_FLOAT_S8X24_UINT).Count;
    caps.limits.maxNoAttachmentSamples              = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
}

int D3D11RenderSystem::GetMinorVersion() const
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    if (device3_) { return 3; }
    #endif
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
    if (device2_) { return 2; }
    #endif
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (device1_) { return 1; }
    #endif
    return 0;
}

static void InitializeD3DDepthStencilTextureWithDSV(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const ClearValue&       clearValue)
{
    /* Create intermediate depth-stencil view for texture */
    ComPtr<ID3D11DepthStencilView> dsv;
    D3D11RenderTarget::CreateSubresourceDSV(
        device,
        textureD3D.GetNative(),
        dsv.ReleaseAndGetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetDXFormat(),
        0,
        0,
        textureD3D.GetNumArrayLayers()
    );

    /* Clear view with depth-stencil values */
    context->ClearDepthStencilView(
        dsv.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        clearValue.depth,
        static_cast<UINT8>(clearValue.stencil)
    );
}

static void InitializeD3DColorTextureWithRTV(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const ClearValue&       clearValue)
{
    /* Create intermediate depth-stencil view for texture */
    ComPtr<ID3D11RenderTargetView> rtv;
    D3D11RenderTarget::CreateSubresourceRTV(
        device,
        textureD3D.GetNative(),
        rtv.ReleaseAndGetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetBaseDXFormat(),
        0,
        0,
        textureD3D.GetNumArrayLayers()
    );

    /* Clear view with depth-stencil values */
    context->ClearRenderTargetView(rtv.Get(), clearValue.color);
}

static void InitializeD3DColorTextureWithUploadBuffer(
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const Extent3D&         extent,
    const ClearValue&       clearValue)
{
    /* Find suitable image format for texture hardware format */
    ImageView imageViewDefault;

    const auto& formatDesc = GetFormatAttribs(textureD3D.GetBaseFormat());
    if (formatDesc.bitSize > 0)
    {
        /* Copy image format and data type from descriptor */
        imageViewDefault.format     = formatDesc.format;
        imageViewDefault.dataType   = formatDesc.dataType;

        /* Generate default image buffer */
        const std::size_t   imageSize   = extent.width * extent.height * extent.depth;
        DynamicByteArray    imageBuffer = GenerateImageBuffer(imageViewDefault.format, imageViewDefault.dataType, imageSize, clearValue.color);

        /* Update only the first MIP-map level for each array slice */
        imageViewDefault.data       = imageBuffer.data();
        imageViewDefault.dataSize   = GetMemoryFootprint(imageViewDefault.format, imageViewDefault.dataType, imageSize);

        for_range(layer, textureD3D.GetNumArrayLayers())
        {
            HRESULT hr = textureD3D.UpdateSubresource(
                /*context:*/        context,
                /*mipLevel:*/       0,
                /*baseArrayLayer:*/ layer,
                /*numArrayLayers:*/ 1,
                /*dstBox:*/         D3D11Types::MakeD3D11Box(0, 0, 0, extent.width, extent.height, extent.depth),
                /*imageView:*/      imageViewDefault
            );
            DXThrowIfFailed(hr, "in 'InitializeD3DColorTextureWithUploadBuffer': LLGL::D3D11Texture::UpdateSubresource failed");
        }
    }
}

void D3D11RenderSystem::InitializeGpuTexture(
    D3D11Texture&               textureD3D,
    const TextureDescriptor&    textureDesc,
    const ImageView*            initialImage)
{
    if (initialImage != nullptr)
    {
        /* Initialize texture with specified image descriptor */
        textureD3D.UpdateSubresource(
            /*context:*/        context_.Get(),
            /*mipLevel:*/       0,
            /*baseArrayLayer:*/ 0,
            /*numArrayLayers:*/ textureDesc.arrayLayers,
            /*dstBox:*/         D3D11Types::MakeD3D11Box(0, 0, 0, textureDesc.extent.width, textureDesc.extent.height, textureDesc.extent.depth),
            /*imageView:*/      *initialImage,
            /*report:*/         &(GetMutableReport())
        );
    }
    else if ((textureDesc.miscFlags & MiscFlags::NoInitialData) == 0 && !IsCompressedFormat(textureDesc.format))
    {
        /* Initialize texture with clear value using hardware accelerated clear function or CPU upload buffer */
        if (IsDepthOrStencilFormat(textureDesc.format))
        {
            const bool hasDSVBinding = ((textureDesc.bindFlags & BindFlags::DepthStencilAttachment) != 0);
            if (hasDSVBinding)
            {
                InitializeD3DDepthStencilTextureWithDSV(
                    device_.Get(),
                    context_.Get(),
                    textureD3D,
                    textureDesc.clearValue
                );
            }
            else
            {
                //LLGL_TRAP_NOT_IMPLEMENTED("initialize depth-stencil texture without DepthStencilAttachment binding"); //TODO
            }
        }
        else
        {
            const bool hasRTVBinding = ((textureDesc.bindFlags & BindFlags::ColorAttachment) != 0);
            if (hasRTVBinding)
            {
                InitializeD3DColorTextureWithRTV(
                    device_.Get(),
                    context_.Get(),
                    textureD3D,
                    textureDesc.clearValue
                );
            }
            else
            {
                InitializeD3DColorTextureWithUploadBuffer(
                    context_.Get(),
                    textureD3D,
                    textureDesc.extent,
                    textureDesc.clearValue
                );
            }
        }
    }
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3

bool D3D11RenderSystem::CheckFactoryFeatureSupport(DXGI_FEATURE feature) const
{
    ComPtr<IDXGIFactory5> factory5;
    HRESULT hr = factory_->QueryInterface(IID_PPV_ARGS(&factory5));

    if (SUCCEEDED(hr))
    {
        BOOL supported = FALSE;
        hr = factory5->CheckFeatureSupport(feature, &supported, sizeof(supported));
        return (SUCCEEDED(hr) && supported != FALSE);
    }

    return false;
}

#endif

void D3D11RenderSystem::NotifyBindingTablesOnRelease(D3D11BindingLocator* locator)
{
    if (locator != nullptr)
    {
        /* Notify state manager that is shared across the primary D3D device context */
        stateMngr_->GetBindingTable().NotifyResourceRelease(locator);

        /* Notify state managers for all deferred device contexts */
        for (D3D11StateManager* deferredStateMngr : deferredStateMngrRefs_)
            deferredStateMngr->GetBindingTable().NotifyResourceRelease(locator);
    }
}


#if LLGL_DEBUG

/*
 * LiveObjectReporter structure
 */

typedef HRESULT (WINAPI *DXGIGetDebugInterfacePfn)(REFIID riid, void** ppDebug);

D3D11RenderSystem::LiveObjectReporter::LiveObjectReporter() :
    debugModule { Module::Load("Dxgidebug.dll") }
{
    if (debugModule)
    {
        DXGIGetDebugInterfacePfn dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterfacePfn>(debugModule->LoadProcedure("DXGIGetDebugInterface"));
        if (dxgiGetDebugInterface)
            dxgiGetDebugInterface(IID_PPV_ARGS(&debugDevice));
    }
}

D3D11RenderSystem::LiveObjectReporter::~LiveObjectReporter()
{
    if (debugDevice)
        debugDevice->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_ALL);
}

#endif // /LLGL_DEBUG


} // /namespace LLGL



// ================================================================================
