/* D3D12RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "D3D12Serialization.h"
#include "D3D12SubresourceContext.h"
#include "../DXCommon/DXCore.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/Vendor.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/Assertion.h"
#include "D3DX12/d3dx12.h"
#include <LLGL/Utils/ForRange.h>
#include <limits.h>
#include <codecvt>

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
    const bool debugDevice = ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0);
    if (debugDevice)
        EnableDebugLayer();

    /* Create DXGU factory 1.4, query video adapters, and create D3D12 device */
    CreateFactory(debugDevice);
    QueryVideoAdapters();
    CreateDevice();
    CreateDxcInterface();

    /* Create command queue interface */
    commandQueue_   = MakeUnique<D3D12CommandQueue>(device_);
    commandContext_ = &(commandQueue_->GetContext());

    /* Create default pipeline layout and command signature pool */
    defaultPipelineLayout_.CreateRootSignature(device_.GetNative(), {});
    cmdSignatureFactory_.CreateDefaultSignatures(device_.GetNative());

    stagingBufferPool_.InitializeDevice(device_.GetNative(), 0);
    D3D12MipGenerator::Get().InitializeDevice(device_.GetNative());
    D3D12BufferConstantsPool::Get().InitializeDevice(device_.GetNative(), *commandContext_, stagingBufferPool_);

    /* Initialize renderer information */
    QueryRendererInfo();
    QueryRenderingCaps();
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

    /* Ensure DXC gets unloaded, if we have an open handle to it. */
	if (dxcModule_ != nullptr)
	{
		FreeLibrary(dxcModule_);
	}
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
    stagingBufferPool_.ReadSubresourceRegion(*commandContext_, bufferD3D.GetResource(), offset, data, dataSize);
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
    bufferD3D.Unmap(*commandContext_);
}

/* ----- Textures ----- */

Texture* D3D12RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    auto* textureD3D = textures_.emplace<D3D12Texture>(device_.GetNative(), textureDesc);

    if (imageDesc != nullptr)
    {
        /* Update base MIP-map */
        TextureRegion region;
        {
            region.subresource.numArrayLayers   = textureDesc.arrayLayers;
            region.extent                       = textureDesc.extent;
        }
        D3D12SubresourceContext subresourceContext{ *commandContext_ };
        UpdateTextureSubresourceFromImage(*textureD3D, region, *imageDesc, subresourceContext);

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

void D3D12RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Execute upload commands and wait for GPU to finish execution */
    D3D12SubresourceContext subresourceContext{ *commandContext_ };
    UpdateTextureSubresourceFromImage(textureD3D, textureRegion, imageDesc, subresourceContext);
}

void D3D12RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Determine what plane to read from */
    const bool isStencilOnlyFormat  = (imageDesc.format == ImageFormat::Stencil);
    const bool isDepthOnlyFormat    = (imageDesc.format == ImageFormat::Depth);
    const UINT texturePlane         = (isStencilOnlyFormat ? 1 : 0);

    /* Create CPU accessible readback buffer for texture and execute command list */
    ComPtr<ID3D12Resource> readbackBuffer;
    UINT rowStride = 0, layerSize = 0, layerStride = 0;
    {
        D3D12SubresourceContext subresourceContext{ *commandContext_ };
        textureD3D.CreateSubresourceCopyAsReadbackBuffer(subresourceContext, textureRegion, texturePlane, rowStride, layerSize, layerStride);
        readbackBuffer = subresourceContext.TakeResource();
    }

    /* Map readback buffer to CPU memory space */
    DstImageDescriptor      dstImageDesc        = imageDesc;
    const Format            format              = textureD3D.GetFormat();
    const FormatAttributes& formatAttribs       = GetFormatAttribs(format);
    const Extent3D          extent              = CalcTextureExtent(textureD3D.GetType(), textureRegion.extent);
    const std::uint32_t     numTexelsPerLayer   = extent.width * extent.height * extent.depth;

    void* mappedData = nullptr;
    HRESULT hr = readbackBuffer->Map(0, nullptr, &mappedData);
    DXThrowIfFailed(hr, "failed to map D3D12 texture copy resource");

    const char* srcData = reinterpret_cast<const char*>(mappedData);
    SrcImageDescriptor srcImageDesc{ formatAttribs.format, formatAttribs.dataType, srcData, layerStride };

    if (isStencilOnlyFormat)
    {
        srcImageDesc.format     = ImageFormat::Stencil;
        srcImageDesc.dataType   = DataType::UInt8;
    }
    else if (isDepthOnlyFormat)
    {
        srcImageDesc.format     = ImageFormat::Depth;
        srcImageDesc.dataType   = DataType::Float32;
    }

    for_range(arrayLayer, textureRegion.subresource.numArrayLayers)
    {
        /* Copy CPU accessible buffer to output data */
        RenderSystem::CopyTextureImageData(dstImageDesc, srcImageDesc, numTexelsPerLayer, extent.width, rowStride);

        /* Move destination image pointer to next layer */
        dstImageDesc.data = reinterpret_cast<char*>(dstImageDesc.data) + layerSize;
        srcImageDesc.data = reinterpret_cast<const char*>(srcImageDesc.data) + layerStride;
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

/* ----- Pipeline States ----- */

PipelineState* D3D12RenderSystem::CreatePipelineState(const Blob& serializedCache)
{
    Serialization::Deserializer reader{ serializedCache };

    /* Read type of PSO */
    auto seg = reader.ReadSegment();
    if (seg.ident == Serialization::D3D12Ident_GraphicsPSOIdent)
    {
        /* Create graphics PSO from cache */
        return pipelineStates_.emplace<D3D12GraphicsPSO>(device_, reader);
    }
    #if 0//TODO
    else if (seg.ident == Serialization::D3D12Ident_ComputePSOIdent)
    {
        /* Create compute PSO from cache */
        return pipelineStates_.emplace<D3D12ComputePSO>(device_, reader);
    }
    #endif

    LLGL_TRAP("serialized cache does not denote a D3D12 graphics or compute PSO");
}

PipelineState* D3D12RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, Blob* serializedCache)
{
    Serialization::Serializer writer;

    D3D12GraphicsPSO* pipelineState = pipelineStates_.emplace<D3D12GraphicsPSO>(
        device_,
        defaultPipelineLayout_,
        pipelineStateDesc,
        GetDefaultRenderPass(),
        (serializedCache != nullptr ? &writer : nullptr)
    );

    if (serializedCache != nullptr)
        *serializedCache = writer.Finalize();

    return pipelineState;
}

PipelineState* D3D12RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, Blob* /*serializedCache*/)
{
    return pipelineStates_.emplace<D3D12ComputePSO>(device_, defaultPipelineLayout_, pipelineStateDesc);
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
        auto* nativeHandleD3D = reinterpret_cast<Direct3D12::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleD3D->device = device_.GetNative();
        nativeHandleD3D->device->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Internal: =======
 */

ComPtr<IDXGISwapChain1> D3D12RenderSystem::CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& swapChainDescDXGI, HWND wnd)
{
    ComPtr<IDXGISwapChain1> swapChain;

    auto hr = factory_->CreateSwapChainForHwnd(commandQueue_->GetNative(), wnd, &swapChainDescDXGI, nullptr, nullptr, &swapChain);
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

void D3D12RenderSystem::QueryVideoAdapters()
{
    /* Enumerate over all video adapters */
    ComPtr<IDXGIAdapter> adapter;
    for (UINT i = 0; factory_->EnumAdapters(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        /* Add adapter to the list and release handle */
        videoAdatperDescs_.push_back(DXGetVideoAdapterDesc(adapter.Get()));
        adapter.Reset();
    }
}

void D3D12RenderSystem::CreateDevice()
{
    /* Use default adapter (null) and try all feature levels */
    auto featureLevels = DXGetFeatureLevels(D3D_FEATURE_LEVEL_12_1);

    /* Try to create a feature level with an hardware adapter */
    HRESULT hr = 0;
    if (!device_.CreateDXDevice(hr, nullptr, featureLevels))
    {
        /* Use software adapter as fallback */
        ComPtr<IDXGIAdapter> adapter;
        factory_->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
        if (!device_.CreateDXDevice(hr, adapter.Get(), featureLevels))
            DXThrowIfFailed(hr, "failed to create D3D12 device");
    }
}

void D3D12RenderSystem::CreateDxcInterface()
{
    dxcModule_ = LoadLibrary(TEXT("dxcompiler.dll"));
    if (dxcModule_ == nullptr)
        return;

    dxcCreateInstanceFn_ = (DxcCreateInstanceProc)GetProcAddress(dxcModule_, "DxcCreateInstance");
    if (dxcCreateInstanceFn_ == nullptr)
    {
        FreeLibrary(dxcModule_);
        dxcModule_ = nullptr;
    }
}

static bool FindHighestShaderModel(ID3D12Device* device, D3D_SHADER_MODEL& shaderModel)
{
    D3D12_FEATURE_DATA_SHADER_MODEL feature;

    for (auto model : { D3D_SHADER_MODEL_6_0, D3D_SHADER_MODEL_5_1 })
    {
        feature.HighestShaderModel = model;
        auto hr = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &feature, sizeof(feature));
        if (SUCCEEDED(hr))
        {
            shaderModel = model;
            return true;
        }
    }

    return false;
}

static const char* DXShaderModelToString(D3D_SHADER_MODEL shaderModel)
{
    switch (shaderModel)
    {
        case D3D_SHADER_MODEL_5_1: return "5.1";
        case D3D_SHADER_MODEL_6_0: return "6.0";
    }
    return "";
}

void D3D12RenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    /* Get D3D version */
    info.rendererName = "Direct3D " + std::string(DXFeatureLevelToVersion(GetFeatureLevel()));

    /* Get shading language support */
    info.shadingLanguageName = "HLSL ";

    D3D_SHADER_MODEL shaderModel = D3D_SHADER_MODEL_5_1;
    if (FindHighestShaderModel(device_.GetNative(), shaderModel))
        info.shadingLanguageName += DXShaderModelToString(shaderModel);
    else
        info.shadingLanguageName += DXFeatureLevelToShaderModel(GetFeatureLevel());

    /* Get device and vendor name from adapter */
    if (!videoAdatperDescs_.empty())
    {
        const auto& videoAdapterDesc = videoAdatperDescs_.front();
        info.deviceName = ToUTF8String(videoAdapterDesc.name);
        info.vendorName = videoAdapterDesc.vendor;
    }
    else
        info.deviceName = info.vendorName = "<no adapter found>";

    SetRendererInfo(info);
}

void D3D12RenderSystem::QueryRenderingCaps()
{
    RenderingCapabilities caps;
    {
        /* Query common DX rendering capabilities */
        DXGetRenderingCaps(caps, GetFeatureLevel());

        /* Set extended attributes */
        caps.features.hasConservativeRasterization  = (GetFeatureLevel() >= D3D_FEATURE_LEVEL_12_0);
        caps.features.hasTextureViewSwizzle         = true;

        caps.limits.maxViewports                    = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        caps.limits.maxViewportSize[0]              = D3D12_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxViewportSize[1]              = D3D12_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxBufferSize                   = ULLONG_MAX;
        caps.limits.maxConstantBufferSize           = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;

        /* Determine maximum number of samples for various formats */
        caps.limits.maxColorBufferSamples           = device_.FindSuitableSampleDesc(DXGI_FORMAT_R8G8B8A8_UNORM).Count;
        caps.limits.maxDepthBufferSamples           = device_.FindSuitableSampleDesc(DXGI_FORMAT_D32_FLOAT).Count;
        caps.limits.maxStencilBufferSamples         = device_.FindSuitableSampleDesc(DXGI_FORMAT_D32_FLOAT_S8X24_UINT).Count;
        caps.limits.maxNoAttachmentSamples          = D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT;
    }
    SetRenderingCaps(caps);
}

void D3D12RenderSystem::ExecuteCommandList()
{
    commandContext_->Finish();
}

void D3D12RenderSystem::ExecuteCommandListAndSync()
{
    commandContext_->Finish(true);
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

    if (SUCCEEDED(bufferD3D.Map(*commandContext_, range, &mappedData, access)))
        return mappedData;

    return nullptr;
}

HRESULT D3D12RenderSystem::UpdateTextureSubresourceFromImage(
    D3D12Texture&               textureD3D,
    const TextureRegion&        region,
    const SrcImageDescriptor&   imageDesc,
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
    const auto  format          = textureD3D.GetFormat();
    const auto& formatAttribs   = GetFormatAttribs(format);

    const auto  texExtent       = textureD3D.GetMipExtent(region.subresource.baseMipLevel);
    const auto  srcExtent       = CalcTextureExtent(textureD3D.GetType(), region.extent, region.subresource.numArrayLayers);

    const auto  dataLayout      = CalcSubresourceLayout(format, srcExtent);

    ByteBuffer intermediateData;
    const void* srcData = imageDesc.data;

    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != imageDesc.format || formatAttribs.dataType != imageDesc.dataType))
    {
        /* Convert image data (e.g. from RGB to RGBA), and redirect initial data to new buffer */
        intermediateData    = ConvertImageBuffer(imageDesc, formatAttribs.format, formatAttribs.dataType, Constants::maxThreadCount);
        srcData             = intermediateData.get();
    }
    else
    {
        /* Validate input data is large enough */
        if (imageDesc.dataSize < dataLayout.dataSize)
            return E_INVALIDARG;
    }

    /* Upload image data to subresource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = srcData;
        subresourceData.RowPitch    = dataLayout.rowStride;
        subresourceData.SlicePitch  = dataLayout.layerStride;
    }

    const bool isFullRegion = (region.offset == Offset3D{} && srcExtent == texExtent);
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


} // /namespace LLGL



// ================================================================================
