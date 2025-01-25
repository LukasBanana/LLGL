/*
 * D3D12RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "D3D12SubresourceContext.h"
#include "../DXCommon/DXCore.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../RenderSystemUtils.h"
#include "../../Core/Vendor.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include "D3DX12/d3dx12.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Backend/Direct3D12/NativeHandle.h>
#include <limits.h>

#include "Shader/D3D12BuiltinShaderFactory.h"

#include "Buffer/D3D12Buffer.h"
#include "Buffer/D3D12BufferArray.h"
#include "Buffer/D3D12BufferConstantsPool.h"

#include "Texture/D3D12MipGenerator.h"

#include "RenderState/D3D12GraphicsPSO.h"
#include "RenderState/D3D12ComputePSO.h"

#include <LLGL/Backend/Direct3D12/NativeHandle.h>


namespace LLGL
{


D3D12RenderSystem::D3D12RenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    const bool isDebugDevice = ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0);
    if (isDebugDevice)
        EnableDebugLayer();

    if (auto* customNativeHandle = GetRendererNativeHandle<Direct3D12::RenderSystemNativeHandle>(renderSystemDesc))
    {
        /* Query all DXGI interfaces from native handle */
        HRESULT hr = QueryDXInterfacesFromNativeHandle(*customNativeHandle);
        DXThrowIfFailed(hr, "failed to query D3D12 device from custom native handle");
    }
    else
    {
        /* Create DXGU factory 1.4, query video adapters, and create D3D12 device */
        CreateFactory(isDebugDevice);

        ComPtr<IDXGIAdapter> preferredAdatper;
        QueryVideoAdapters(renderSystemDesc.flags, preferredAdatper);

        HRESULT hr = CreateDevice(preferredAdatper.Get(), isDebugDevice);
        DXThrowIfFailed(hr, "failed to create D3D12 device");
    }

    /* Query and cache DXGI factory feature support */
    tearingSupported_ = CheckFactoryFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING);

    /* Create command queue interface */
    commandQueue_   = MakeUnique<D3D12CommandQueue>(device_);
    commandContext_ = &(commandQueue_->GetContext());

    /* Create default pipeline layout and command signature pool */
    defaultPipelineLayout_.CreateRootSignature(device_.GetNative(), {});
    cmdSignatureFactory_.CreateDefaultSignatures(device_.GetNative());

    stagingBufferPool_.InitializeDevice(device_.GetNative(), 0);
    D3D12MipGenerator::Get().InitializeDevice(device_.GetNative());
    D3D12BufferConstantsPool::Get().InitializeDevice(device_.GetNative(), *commandContext_, *commandQueue_, stagingBufferPool_);
    D3D12BuiltinShaderFactory::Get().CreateBuiltinPSOs(device_.GetNative());
}

D3D12RenderSystem::~D3D12RenderSystem()
{
    SyncGPU();

    /*
    Release render targets first, to ensure the GPU is no longer
    referencing resources that are about to be released
    */
    swapChains_.clear();

    /* Clear shaders explicitly to release all ComPtr<ID3DBlob> objects */
    shaders_.clear();

    /* Clear resources of singletons */
    D3D12MipGenerator::Get().Clear();
    D3D12BufferConstantsPool::Get().Clear();
    D3D12BuiltinShaderFactory::Get().Clear();
}

/* ----- Swap-chain ----- */

SwapChain* D3D12RenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<D3D12SwapChain>(*this, swapChainDesc, surface);
}

void D3D12RenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* D3D12RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D12RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    return commandBuffers_.emplace<D3D12CommandBuffer>(*this, commandBufferDesc);
}

void D3D12RenderSystem::Release(CommandBuffer& commandBuffer)
{
    SyncGPU();
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

Buffer* D3D12RenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, ULLONG_MAX);
    D3D12Buffer* bufferD3D = buffers_.emplace<D3D12Buffer>(device_.GetNative(), bufferDesc);
    if (initialData != nullptr)
        UpdateBufferAndSync(*bufferD3D, 0, initialData, bufferDesc.size, bufferD3D->GetAlignment());
    return bufferD3D;
}

BufferArray* D3D12RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<D3D12BufferArray>(numBuffers, bufferArray);
}

void D3D12RenderSystem::Release(Buffer& buffer)
{
    SyncGPU();
    buffers_.erase(&buffer);
}

void D3D12RenderSystem::Release(BufferArray& bufferArray)
{
    SyncGPU();
    bufferArrays_.erase(&bufferArray);
}

void D3D12RenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    UpdateBufferAndSync(bufferD3D, offset, data, dataSize);
}

void D3D12RenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    stagingBufferPool_.ReadSubresourceRegion(*commandContext_, *commandQueue_, bufferD3D.GetResource(), offset, data, dataSize);
    /* No ExecuteCommandListAndSync() here as it has already been flushed by the staging buffer pool */
}

void* D3D12RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    return MapBufferRange(bufferD3D, access, 0, bufferD3D.GetBufferSize());
}

void* D3D12RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    return MapBufferRange(bufferD3D, access, offset, length);
}

void D3D12RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    bufferD3D.Unmap(*commandContext_, *commandQueue_, stagingBufferPool_);
}

/* ----- Textures ----- */

Texture* D3D12RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    auto* textureD3D = textures_.emplace<D3D12Texture>(device_.GetNative(), textureDesc);

    if (initialImage != nullptr)
    {
        /* Update base MIP-map */
        TextureRegion region;
        {
            region.subresource.numArrayLayers   = textureDesc.arrayLayers;
            region.extent                       = textureDesc.extent;
        }
        D3D12SubresourceContext subresourceContext{ *commandContext_, *commandQueue_ };
        UpdateTextureSubresourceFromImage(*textureD3D, region, *initialImage, subresourceContext);

        /* Generate MIP-maps if enabled */
        if (MustGenerateMipsOnCreate(textureDesc))
            D3D12MipGenerator::Get().GenerateMips(*commandContext_, *textureD3D, textureD3D->GetWholeSubresource());
    }

    return textureD3D;
}

void D3D12RenderSystem::Release(Texture& texture)
{
    SyncGPU();
    textures_.erase(&texture);
}

void D3D12RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Execute upload commands and wait for GPU to finish execution */
    D3D12SubresourceContext subresourceContext{ *commandContext_, *commandQueue_ };
    UpdateTextureSubresourceFromImage(textureD3D, textureRegion, srcImageView, subresourceContext);
}

void D3D12RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Determine what plane to read from */
    const bool isStencilOnlyFormat  = (dstImageView.format == ImageFormat::Stencil);
    const bool isDepthOnlyFormat    = (dstImageView.format == ImageFormat::Depth);
    const UINT texturePlane         = (isStencilOnlyFormat ? 1 : 0);

    /* Create CPU accessible readback buffer for texture and execute command list */
    ComPtr<ID3D12Resource> readbackBuffer;
    UINT rowStride = 0, layerSize = 0, layerStride = 0;
    {
        D3D12SubresourceContext subresourceContext{ *commandContext_, *commandQueue_ };
        textureD3D.CreateSubresourceCopyAsReadbackBuffer(subresourceContext, textureRegion, texturePlane, rowStride, layerSize, layerStride);
        readbackBuffer = subresourceContext.TakeResource();
    }

    /* Map readback buffer to CPU memory space */
    MutableImageView        intermediateDstView = dstImageView;
    const Format            format              = textureD3D.GetFormat();
    const FormatAttributes& formatAttribs       = GetFormatAttribs(format);
    const Extent3D          extent              = CalcTextureExtent(textureD3D.GetType(), textureRegion.extent);
    const std::uint32_t     numTexelsPerLayer   = extent.width * extent.height * extent.depth;

    void* mappedData = nullptr;
    HRESULT hr = readbackBuffer->Map(0, nullptr, &mappedData);
    DXThrowIfFailed(hr, "failed to map D3D12 texture copy resource");

    const char* srcData = static_cast<const char*>(mappedData);
    ImageView intermediateSrcView{ formatAttribs.format, formatAttribs.dataType, srcData, layerStride };

    if (isStencilOnlyFormat)
    {
        intermediateSrcView.format      = ImageFormat::Stencil;
        intermediateSrcView.dataType    = DataType::UInt8;
    }
    else if (isDepthOnlyFormat)
    {
        intermediateSrcView.format      = ImageFormat::Depth;
        intermediateSrcView.dataType    = DataType::Float32;
    }

    for_range(arrayLayer, textureRegion.subresource.numArrayLayers)
    {
        /* Copy CPU accessible buffer to output data */
        RenderSystem::CopyTextureImageData(intermediateDstView, intermediateSrcView, numTexelsPerLayer, extent.width, rowStride);

        /* Move destination image pointer to next layer */
        intermediateDstView.data = static_cast<char*>(intermediateDstView.data) + layerSize;
        intermediateSrcView.data = static_cast<const char*>(intermediateSrcView.data) + layerStride;
    }

    /* Unmap buffer */
    const D3D12_RANGE writtenRange = { 0, 0 };
    readbackBuffer->Unmap(0, &writtenRange);
}

/* ----- Sampler States ---- */

Sampler* D3D12RenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<D3D12Sampler>(samplerDesc);
}

void D3D12RenderSystem::Release(Sampler& sampler)
{
    SyncGPU();
    samplers_.erase(&sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D12RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<D3D12ResourceHeap>(device_.GetNative(), resourceHeapDesc, initialResourceViews);
}

void D3D12RenderSystem::Release(ResourceHeap& resourceHeap)
{
    SyncGPU();
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t D3D12RenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D12ResourceHeap&, resourceHeap);
    return resourceHeapD3D.CreateResourceViewHandles(device_.GetNative(), firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* D3D12RenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<D3D12RenderPass>(device_, renderPassDesc);
}

void D3D12RenderSystem::Release(RenderPass& renderPass)
{
    SyncGPU();
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D12RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<D3D12RenderTarget>(device_, renderTargetDesc);
}

void D3D12RenderSystem::Release(RenderTarget& renderTarget)
{
    SyncGPU();
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* D3D12RenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    RenderSystem::AssertCreateShader(shaderDesc);
    return shaders_.emplace<D3D12Shader>(*this, shaderDesc);
}

void D3D12RenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D12RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<D3D12PipelineLayout>(device_.GetNative(), pipelineLayoutDesc);
}

void D3D12RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    SyncGPU();
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* D3D12RenderSystem::CreatePipelineCache(const Blob& initialBlob)
{
    return pipelineCaches_.emplace<D3D12PipelineCache>(initialBlob);
}

void D3D12RenderSystem::Release(PipelineCache& pipelineCache)
{
    /* No GPU sync necessary for PSO caches; they only store an ID3DBlob that is used synchronously */
    pipelineCaches_.erase(&pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* D3D12RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<D3D12GraphicsPSO>(device_.GetNative(), defaultPipelineLayout_, pipelineStateDesc, GetDefaultRenderPass(), pipelineCache);
}

PipelineState* D3D12RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    return pipelineStates_.emplace<D3D12ComputePSO>(device_.GetNative(), defaultPipelineLayout_, pipelineStateDesc, pipelineCache);
}

void D3D12RenderSystem::Release(PipelineState& pipelineState)
{
    SyncGPU();
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D12RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<D3D12QueryHeap>(device_, queryHeapDesc);
}

void D3D12RenderSystem::Release(QueryHeap& queryHeap)
{
    SyncGPU();
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* D3D12RenderSystem::CreateFence()
{
    return fences_.emplace<D3D12Fence>(device_.GetNative(), 0);
}

void D3D12RenderSystem::Release(Fence& fence)
{
    SyncGPU();
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool D3D12RenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D12::RenderSystemNativeHandle))
    {
        auto* nativeHandleD3D = static_cast<Direct3D12::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleD3D->factory = factory_.Get();
        nativeHandleD3D->factory->AddRef();
        nativeHandleD3D->device = device_.GetNative();
        nativeHandleD3D->device->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Internal: =======
 */

ComPtr<IDXGISwapChain1> D3D12RenderSystem::CreateDXSwapChain(
    const DXGI_SWAP_CHAIN_DESC1&    swapChainDescDXGI,
    const void*                     nativeWindowHandle,
    std::size_t                     nativeWindowHandleSize)
{
    LLGL_ASSERT(nativeWindowHandleSize == sizeof(NativeHandle));
    auto* nativeWindowHandlePtr = static_cast<const NativeHandle*>(nativeWindowHandle);

    ComPtr<IDXGISwapChain1> swapChain;

    #ifdef LLGL_OS_UWP
    HRESULT hr = factory_->CreateSwapChainForCoreWindow(commandQueue_->GetNative(), nativeWindowHandlePtr->window, &swapChainDescDXGI, nullptr, &swapChain);
    #else
    HRESULT hr = factory_->CreateSwapChainForHwnd(commandQueue_->GetNative(), nativeWindowHandlePtr->window, &swapChainDescDXGI, nullptr, nullptr, &swapChain);
    #endif
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");

    return swapChain;
}

void D3D12RenderSystem::SyncGPU()
{
    commandQueue_->WaitIdle();
}


/*
 * ======= Private: =======
 */

bool D3D12RenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
        QueryRendererInfo(*outInfo);
    if (outCaps != nullptr)
        QueryRenderingCaps(*outCaps);
    return true;
}

void D3D12RenderSystem::EnableDebugLayer()
{
    ComPtr<ID3D12Debug> debugController0;
    if (SUCCEEDED( D3D12GetDebugInterface(IID_PPV_ARGS(debugController0.ReleaseAndGetAddressOf())) ))
    {
        debugController0->EnableDebugLayer();

        ComPtr<ID3D12Debug1> debugController1;
        if (SUCCEEDED( debugController0->QueryInterface(IID_PPV_ARGS(debugController1.ReleaseAndGetAddressOf())) ))
            debugController1->SetEnableGPUBasedValidation(TRUE);
    }
}

void D3D12RenderSystem::CreateFactory(bool debugDevice)
{
    /* Create DXGI factory 1.4 */
    HRESULT hr = S_OK;
    if (debugDevice)
        hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory_.ReleaseAndGetAddressOf()));
    else
        hr = CreateDXGIFactory1(IID_PPV_ARGS(factory_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create DXGI factor 1.4");
}

void D3D12RenderSystem::QueryVideoAdapters(long flags, ComPtr<IDXGIAdapter>& outPreferredAdatper)
{
    videoAdatperInfo_ = DXGetVideoAdapterInfo(factory_.Get(), flags, outPreferredAdatper.ReleaseAndGetAddressOf());
}

HRESULT D3D12RenderSystem::CreateDevice(IDXGIAdapter* preferredAdapter, bool isDebugLayerEnabled)
{
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 2
        D3D_FEATURE_LEVEL_12_2,
        #endif
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1
        D3D_FEATURE_LEVEL_12_1,
        #endif
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    HRESULT hr = S_OK;

    if (preferredAdapter != nullptr)
    {
        /* Try to create device with perferred adatper */
        hr = device_.CreateDXDevice(featureLevels, isDebugLayerEnabled, preferredAdapter);
        if (SUCCEEDED(hr))
            return hr;
    }

    /* Try to create device with default adapter */
    hr = device_.CreateDXDevice(featureLevels, isDebugLayerEnabled);
    if (SUCCEEDED(hr))
    {
        /* Update video adapter info with default adapter */
        videoAdatperInfo_ = DXGetVideoAdapterInfo(factory_.Get());
        return hr;
    }

    /* Use software adapter as fallback */
    ComPtr<IDXGIAdapter> adapter;
    factory_->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
    return device_.CreateDXDevice(featureLevels, isDebugLayerEnabled, adapter.Get());
}

HRESULT D3D12RenderSystem::QueryDXInterfacesFromNativeHandle(const Direct3D12::RenderSystemNativeHandle& nativeHandle)
{
    LLGL_ASSERT_PTR(nativeHandle.factory);
    LLGL_ASSERT_PTR(nativeHandle.device);

    factory_ = nativeHandle.factory;
    const LUID adapterLUID = nativeHandle.device->GetAdapterLuid();

    ComPtr<IDXGIAdapter> dxgiAdapter;
    HRESULT hr = factory_->EnumAdapterByLuid(adapterLUID, IID_PPV_ARGS(&dxgiAdapter));
    DXThrowIfFailed(hr, "failed to get adapter from DXGI factory");

    DXGI_ADAPTER_DESC dxgiAdapterDesc;
    hr = dxgiAdapter->GetDesc(&dxgiAdapterDesc);
    DXThrowIfFailed(hr, "failed to get descriptor from DXGI adapter");

    DXConvertVideoAdapterInfo(dxgiAdapter.Get(), dxgiAdapterDesc, videoAdatperInfo_);

    return device_.ShareDXDevice(nativeHandle.device);
}

static D3D_SHADER_MODEL FindHighestShaderModel(ID3D12Device* device)
{
    D3D12_FEATURE_DATA_SHADER_MODEL feature;

    const D3D_SHADER_MODEL shaderModles[] =
    {
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1
        D3D_SHADER_MODEL_6_7,
        D3D_SHADER_MODEL_6_6,
        D3D_SHADER_MODEL_6_5,
        D3D_SHADER_MODEL_6_4,
        D3D_SHADER_MODEL_6_3,
        D3D_SHADER_MODEL_6_2,
        D3D_SHADER_MODEL_6_1,
        #endif
        D3D_SHADER_MODEL_6_0,
        D3D_SHADER_MODEL_5_1,
    };

    for (D3D_SHADER_MODEL model : shaderModles)
    {
        feature.HighestShaderModel = model;
        HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &feature, sizeof(feature));
        if (SUCCEEDED(hr))
            return model;
    }

    return D3D_SHADER_MODEL_5_1;
}

static const char* DXShaderModelToString(D3D_SHADER_MODEL shaderModel)
{
    switch (shaderModel)
    {
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1
        case D3D_SHADER_MODEL_6_7: return "6.7";
        case D3D_SHADER_MODEL_6_6: return "6.6";
        case D3D_SHADER_MODEL_6_5: return "6.5";
        case D3D_SHADER_MODEL_6_4: return "6.4";
        case D3D_SHADER_MODEL_6_3: return "6.3";
        case D3D_SHADER_MODEL_6_2: return "6.2";
        case D3D_SHADER_MODEL_6_1: return "6.1";
        #endif
        case D3D_SHADER_MODEL_6_0: return "6.0";
        case D3D_SHADER_MODEL_5_1: return "5.1";
    }
    return "";
}

static const char* DXFeatureLevelToVersion(D3D_FEATURE_LEVEL featureLevel)
{
    #if LLGL_D3D12_ENABLE_FEATURELEVEL > 0
    switch (featureLevel)
    {
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 2
        case D3D_FEATURE_LEVEL_12_2:    return "12.2";
        #endif
        #if LLGL_D3D12_ENABLE_FEATURELEVEL >= 1
        case D3D_FEATURE_LEVEL_12_1:    return "12.1";
        #endif
        default:                        break;
    }
    #endif
    return "12.0";
}

int D3D12RenderSystem::GetMinorVersion() const
{
    return 0;
}

void D3D12RenderSystem::QueryRendererInfo(RendererInfo& info)
{
    /* Get D3D version */
    info.rendererName = "Direct3D " + std::string(DXFeatureLevelToVersion(GetFeatureLevel()));

    /* Get shading language support */
    info.shadingLanguageName = "HLSL ";

    const D3D_SHADER_MODEL shaderModel = FindHighestShaderModel(device_.GetNative());
    info.shadingLanguageName += DXShaderModelToString(shaderModel);

    /* Get device and vendor name from adapter */
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

static std::vector<Format> GetDefaultSupportedDXTextureFormats()
{
    std::vector<Format> formats;

    std::size_t numFormats = 0;
    DXGetDefaultSupportedTextureFormats(nullptr, &numFormats);

    formats.resize(numFormats, Format::Undefined);
    DXGetDefaultSupportedTextureFormats(formats.data(), nullptr);

    formats.insert(
        formats.end(),
        { Format::BC4UNorm, Format::BC4SNorm, Format::BC5UNorm, Format::BC5SNorm }
    );

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

void D3D12RenderSystem::QueryRenderingCaps(RenderingCapabilities& caps)
{
    const D3D_FEATURE_LEVEL featureLevel = GetFeatureLevel();
    //const int minorVersion = GetMinorVersion();

    const std::uint32_t maxThreadGroups = 65535u;

    /* Query common attributes */
    caps.screenOrigin                               = ScreenOrigin::UpperLeft;
    caps.clippingRange                              = ClippingRange::ZeroToOne;
    caps.shadingLanguages                           = DXGetHLSLVersions(featureLevel);
    caps.textureFormats                             = GetDefaultSupportedDXTextureFormats();

    caps.features.hasRenderTargets                  = true;
    caps.features.has3DTextures                     = true;
    caps.features.hasCubeTextures                   = true;
    caps.features.hasArrayTextures                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasCubeArrayTextures              = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
    caps.features.hasMultiSampleTextures            = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasMultiSampleArrayTextures       = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasTextureViews                   = true;
    caps.features.hasTextureViewSwizzle             = true;
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
    caps.features.hasConservativeRasterization      = (GetFeatureLevel() >= D3D_FEATURE_LEVEL_12_0);
    caps.features.hasStreamOutputs                  = (featureLevel >= D3D_FEATURE_LEVEL_10_0);
    caps.features.hasLogicOp                        = (featureLevel >= D3D_FEATURE_LEVEL_11_1);
    caps.features.hasPipelineCaching                = true;
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
    caps.limits.maxViewports                        = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    caps.limits.maxViewportSize[0]                  = D3D12_VIEWPORT_BOUNDS_MAX;
    caps.limits.maxViewportSize[1]                  = D3D12_VIEWPORT_BOUNDS_MAX;
    caps.limits.maxBufferSize                       = ULLONG_MAX;
    caps.limits.maxConstantBufferSize               = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    caps.limits.maxStreamOutputs                    = 4u;
    caps.limits.maxTessFactor                       = 64u;
    caps.limits.minConstantBufferAlignment          = 256u;
    caps.limits.minSampledBufferAlignment           = 32u;
    caps.limits.minStorageBufferAlignment           = 32u;
    caps.limits.maxColorBufferSamples               = device_.FindSuitableSampleDesc(DXGI_FORMAT_R8G8B8A8_UNORM).Count;
    caps.limits.maxDepthBufferSamples               = device_.FindSuitableSampleDesc(DXGI_FORMAT_D32_FLOAT).Count;
    caps.limits.maxStencilBufferSamples             = device_.FindSuitableSampleDesc(DXGI_FORMAT_D32_FLOAT_S8X24_UINT).Count;
    caps.limits.maxNoAttachmentSamples              = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
}

void D3D12RenderSystem::ExecuteCommandListAndSync()
{
    commandQueue_->FinishAndSubmitCommandContext(*commandContext_, true);
}

void D3D12RenderSystem::UpdateBufferAndSync(
    D3D12Buffer&    bufferD3D,
    std::uint64_t   offset,
    const void*     data,
    std::uint64_t   dataSize,
    std::uint64_t   alignment)
{
    stagingBufferPool_.WriteImmediate(*commandContext_, bufferD3D.GetResource(), offset, data, dataSize, alignment);
    ExecuteCommandListAndSync();
}

void* D3D12RenderSystem::MapBufferRange(D3D12Buffer& bufferD3D, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    void* mappedData = nullptr;
    const D3D12_RANGE range{ static_cast<SIZE_T>(offset), static_cast<SIZE_T>(offset + length) };

    if (SUCCEEDED(bufferD3D.Map(*commandContext_, *commandQueue_, stagingBufferPool_, range, &mappedData, access)))
        return mappedData;

    return nullptr;
}

HRESULT D3D12RenderSystem::UpdateTextureSubresourceFromImage(
    D3D12Texture&               textureD3D,
    const TextureRegion&        region,
    const ImageView&            imageView,
    D3D12SubresourceContext&    subresourceContext)
{
    /* Validate subresource range */
    const auto& subresource = region.subresource;
    if (subresource.baseMipLevel + subresource.numMipLevels     > textureD3D.GetNumMipLevels() ||
        subresource.baseArrayLayer + subresource.numArrayLayers > textureD3D.GetNumArrayLayers() ||
        subresource.numMipLevels != 1)
    {
        return E_INVALIDARG;
    }

    /* Check if image data conversion is necessary */
    const Format            format          = textureD3D.GetFormat();
    const FormatAttributes& formatAttribs   = GetFormatAttribs(format);

    const Extent3D                      srcExtent   = CalcTextureExtent(textureD3D.GetType(), region.extent, subresource.numArrayLayers);
    const SubresourceCPUMappingLayout   dataLayout  = CalcSubresourceCPUMappingLayout(format, region.extent, subresource.numArrayLayers, imageView.format, imageView.dataType);

    if (imageView.dataSize < dataLayout.imageSize)
    {
        Errorf(
            "image data size (%zu) is too small to update subresource of D3D12 texture (%zu is required)",
            imageView.dataSize, dataLayout.imageSize
        );
        return E_INVALIDARG;
    }

    const Extent3D mipExtent = textureD3D.GetMipExtent(region.subresource.baseMipLevel);

    DynamicByteArray intermediateData;
    const void* srcData = imageView.data;

    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != imageView.format || formatAttribs.dataType != imageView.dataType))
    {
        /* Convert image data (e.g. from RGB to RGBA), and redirect initial data to new buffer */
        intermediateData    = ConvertImageBuffer(imageView, formatAttribs.format, formatAttribs.dataType, LLGL_MAX_THREAD_COUNT);
        srcData             = intermediateData.get();
        LLGL_ASSERT(intermediateData.size() == dataLayout.subresourceSize);
    }

    /* Upload image data to subresource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = srcData;
        subresourceData.RowPitch    = dataLayout.rowStride;
        subresourceData.SlicePitch  = dataLayout.layerStride;
    }

    const bool isFullRegion = (region.offset == Offset3D{} && srcExtent == mipExtent);
    if (isFullRegion)
        textureD3D.UpdateSubresource(subresourceContext, subresourceData, region.subresource);
    else
        textureD3D.UpdateSubresourceRegion(subresourceContext, subresourceData, region);

    return S_OK;
}

const D3D12RenderPass* D3D12RenderSystem::GetDefaultRenderPass() const
{
    if (!swapChains_.empty())
    {
        if (auto renderPass = (*swapChains_.begin())->GetRenderPass())
            return LLGL_CAST(const D3D12RenderPass*, renderPass);
    }
    return nullptr;
}

bool D3D12RenderSystem::CheckFactoryFeatureSupport(DXGI_FEATURE feature) const
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


} // /namespace LLGL



// ================================================================================
