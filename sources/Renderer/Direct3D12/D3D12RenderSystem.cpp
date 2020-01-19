/*
 * D3D12RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "D3D12Serialization.h"
#include "../DXCommon/DXCore.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/Vendor.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include "D3DX12/d3dx12.h"
#include <limits>
#include <codecvt>

#include "Buffer/D3D12Buffer.h"
#include "Buffer/D3D12BufferArray.h"
#include "Buffer/D3D12BufferConstantsPool.h"

#include "Texture/D3D12MipGenerator.h"

#include "RenderState/D3D12GraphicsPSO.h"
#include "RenderState/D3D12ComputePSO.h"


namespace LLGL
{


D3D12RenderSystem::D3D12RenderSystem()
{
    #ifdef LLGL_DEBUG
    EnableDebugLayer();
    #endif

    /* Create DXGU factory 1.4, query video adapters, and create D3D12 device */
    CreateFactory();
    QueryVideoAdapters();
    CreateDevice();

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
    renderContexts_.clear();

    /* Clear shaders explicitly to release all ComPtr<ID3DBlob> objects */
    shaders_.clear();
    shaderPrograms_.clear();

    /* Clear resources of singletons */
    D3D12MipGenerator::Get().Clear();
    D3D12BufferConstantsPool::Get().Clear();
}

/* ----- Render Context ----- */

RenderContext* D3D12RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(renderContexts_, MakeUnique<D3D12RenderContext>(*this, desc, surface));
}

void D3D12RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* D3D12RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D12RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    return TakeOwnership(commandBuffers_, MakeUnique<D3D12CommandBuffer>(*this, desc));
}

void D3D12RenderSystem::Release(CommandBuffer& commandBuffer)
{
    SyncGPU();
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

// private
std::unique_ptr<D3D12Buffer> D3D12RenderSystem::CreateGpuBuffer(const BufferDescriptor& desc, const void* initialData)
{
    auto bufferD3D = MakeUnique<D3D12Buffer>(device_.GetNative(), desc);

    if (initialData)
    {
        /* Write initial data to GPU buffer */
        stagingBufferPool_.WriteImmediate(
            *commandContext_,
            bufferD3D->GetResource(),
            0,
            initialData,
            desc.size,
            bufferD3D->GetAlignment()
        );

        /* Execute upload commands and wait for GPU to finish execution */
        ExecuteCommandListAndSync();
    }

    return bufferD3D;
}

Buffer* D3D12RenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<UINT64>::max()));
    return TakeOwnership(buffers_, CreateGpuBuffer(desc, initialData));
}

static std::unique_ptr<BufferArray> MakeD3D12BufferArray(long bindFlags, std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    return MakeUnique<D3D12BufferArray>(bindFlags, numBuffers, bufferArray);
}

BufferArray* D3D12RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    auto refBindFlags = bufferArray[0]->GetBindFlags();
    return TakeOwnership(bufferArrays_, MakeD3D12BufferArray(refBindFlags, numBuffers, bufferArray));
}

void D3D12RenderSystem::Release(Buffer& buffer)
{
    SyncGPU();
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D12RenderSystem::Release(BufferArray& bufferArray)
{
    SyncGPU();
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void D3D12RenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    stagingBufferPool_.WriteImmediate(*commandContext_, dstBufferD3D.GetResource(), dstOffset, data, dataSize);
    ExecuteCommandListAndSync();
}

void* D3D12RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);

    void* mappedData = nullptr;
    const D3D12_RANGE range{ 0, static_cast<SIZE_T>(bufferD3D.GetBufferSize()) };

    if (SUCCEEDED(bufferD3D.Map(*commandContext_, range, &mappedData, access)))
        return mappedData;

    return nullptr;
}

void D3D12RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
    bufferD3D.Unmap(*commandContext_);
}

/* ----- Textures ----- */

//private
void D3D12RenderSystem::UpdateGpuTexture(
    D3D12Texture&               textureD3D,
    const TextureRegion&        region,
    const SrcImageDescriptor&   imageDesc,
    ComPtr<ID3D12Resource>&     uploadBuffer)
{
    /* Validate subresource range */
    const auto& subresource = region.subresource;
    if (subresource.baseMipLevel + subresource.numMipLevels     > textureD3D.GetNumMipLevels() ||
        subresource.baseArrayLayer + subresource.numArrayLayers > textureD3D.GetNumArrayLayers())
    {
        throw std::invalid_argument("texture subresource out of range for image upload");
    }

    /* Check if image data conversion is necessary */
    auto format = textureD3D.GetFormat();
    const auto& formatAttribs = GetFormatAttribs(format);
    auto dataLayout = CalcSubresourceLayout(format, region.extent);

    ByteBuffer intermediateData;
    const void* initialData = imageDesc.data;

    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != imageDesc.format || formatAttribs.dataType != imageDesc.dataType))
    {
        /* Convert image data (e.g. from RGB to RGBA), and redirect initial data to new buffer */
        intermediateData    = ConvertImageBuffer(imageDesc, formatAttribs.format, formatAttribs.dataType, GetConfiguration().threadCount);
        initialData         = intermediateData.get();
    }
    else
    {
        /* Validate input data is large enough */
        if (imageDesc.dataSize < dataLayout.dataSize)
        {
            throw std::invalid_argument(
                "image data size is too small to update subresource of D3D12 texture (" +
                std::to_string(dataLayout.dataSize) + " is required but only " + std::to_string(imageDesc.dataSize) + " was specified)"
            );
        }
    }

    /* Upload image data to subresource */
    D3D12_SUBRESOURCE_DATA subresourceData;
    {
        subresourceData.pData       = initialData;
        subresourceData.RowPitch    = dataLayout.rowStride;
        subresourceData.SlicePitch  = dataLayout.layerStride;
    }
    textureD3D.UpdateSubresource(
        device_.GetNative(),
        commandContext_->GetCommandList(),
        uploadBuffer,
        subresourceData,
        region.subresource.baseMipLevel,
        region.subresource.baseArrayLayer,
        region.subresource.numArrayLayers
    );
}

Texture* D3D12RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    auto textureD3D = MakeUnique<D3D12Texture>(device_.GetNative(), textureDesc);

    if (imageDesc != nullptr)
    {
        ComPtr<ID3D12Resource> uploadBuffer;

        /* Update first MIP-map */
        TextureRegion region;
        {
            region.subresource.numArrayLayers   = textureDesc.arrayLayers;
            region.extent                       = textureDesc.extent;
        }
        UpdateGpuTexture(*textureD3D, region, *imageDesc, uploadBuffer);

        /* Generate MIP-maps if enabled */
        if (MustGenerateMipsOnCreate(textureDesc))
            D3D12MipGenerator::Get().GenerateMips(*commandContext_, *textureD3D, textureD3D->GetWholeSubresource());

        /* Execute upload commands and wait for GPU to finish execution */
        ExecuteCommandListAndSync();
    }

    return TakeOwnership(textures_, std::move(textureD3D));
}

void D3D12RenderSystem::Release(Texture& texture)
{
    SyncGPU();
    RemoveFromUniqueSet(textures_, &texture);
}

void D3D12RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Execute upload commands and wait for GPU to finish execution */
    ComPtr<ID3D12Resource> uploadBuffer;
    UpdateGpuTexture(textureD3D, textureRegion, imageDesc, uploadBuffer);

    /* Execute upload commands and wait for GPU to finish execution */
    ExecuteCommandListAndSync();
}

void D3D12RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Create CPU accessible readback buffer for texture and execute command list */
    ComPtr<ID3D12Resource> readbackBuffer;
    UINT rowStride = 0;

    textureD3D.CreateSubresourceCopyAsReadbackBuffer(device_.GetNative(), *commandContext_, textureRegion, readbackBuffer, rowStride);
    ExecuteCommandListAndSync();

    /* Map readback buffer to CPU memory space */
    void* mappedData = nullptr;

    auto hr = readbackBuffer->Map(0, nullptr, &mappedData);
    DXThrowIfFailed(hr, "failed to map D3D12 texture copy resource");

    /* Copy CPU accessible buffer to output data */
    auto format = textureD3D.GetFormat();
    auto extent = CalcTextureExtent(textureD3D.GetType(), textureRegion.extent, textureRegion.subresource.numArrayLayers);

    CopyTextureImageData(imageDesc, extent, format, mappedData, rowStride);

    /* Unmap buffer */
    const D3D12_RANGE writtenRange{ 0, 0 };
    readbackBuffer->Unmap(0, &writtenRange);
}

/* ----- Sampler States ---- */

Sampler* D3D12RenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<D3D12Sampler>(desc));
}

void D3D12RenderSystem::Release(Sampler& sampler)
{
    SyncGPU();
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D12RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<D3D12ResourceHeap>(device_.GetNative(), desc));
}

void D3D12RenderSystem::Release(ResourceHeap& resourceHeap)
{
    SyncGPU();
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* D3D12RenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return TakeOwnership(renderPasses_, MakeUnique<D3D12RenderPass>(device_, desc));
}

void D3D12RenderSystem::Release(RenderPass& renderPass)
{
    SyncGPU();
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D12RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<D3D12RenderTarget>(device_, desc));
}

void D3D12RenderSystem::Release(RenderTarget& renderTarget)
{
    SyncGPU();
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* D3D12RenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    AssertCreateShader(desc);
    return TakeOwnership(shaders_, MakeUnique<D3D12Shader>(desc));
}

ShaderProgram* D3D12RenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    AssertCreateShaderProgram(desc);
    return TakeOwnership(shaderPrograms_, MakeUnique<D3D12ShaderProgram>(desc));
}

void D3D12RenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void D3D12RenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D12RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<D3D12PipelineLayout>(device_.GetNative(), desc));
}

void D3D12RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    SyncGPU();
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
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
        return TakeOwnership(pipelineStates_, MakeUnique<D3D12GraphicsPSO>(device_, reader));
    }
    #if 0//TODO
    else if (seg.ident == Serialization::D3D12Ident_ComputePSOIdent)
    {
        /* Create compute PSO from cache */
        return TakeOwnership(pipelineStates_, MakeUnique<D3D12ComputePSO>(device_, reader));
    }
    #endif

    throw std::runtime_error("serialized cache does not denote a D3D12 graphics or compute PSO");
}

PipelineState* D3D12RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache)
{
    Serialization::Serializer writer;

    auto pipelineState = TakeOwnership(
        pipelineStates_,
        MakeUnique<D3D12GraphicsPSO>(
            device_,
            defaultPipelineLayout_,
            desc,
            GetDefaultRenderPass(),
            (serializedCache != nullptr ? &writer : nullptr)
        )
    );

    if (serializedCache != nullptr)
        *serializedCache = writer.Finalize();

    return pipelineState;
}

PipelineState* D3D12RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return TakeOwnership(
        pipelineStates_,
        MakeUnique<D3D12ComputePSO>(device_, defaultPipelineLayout_, desc)
    );
}

void D3D12RenderSystem::Release(PipelineState& pipelineState)
{
    SyncGPU();
    RemoveFromUniqueSet(pipelineStates_, &pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D12RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<D3D12QueryHeap>(device_, desc));
}

void D3D12RenderSystem::Release(QueryHeap& queryHeap)
{
    SyncGPU();
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* D3D12RenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<D3D12Fence>(device_.GetNative(), 0));
}

void D3D12RenderSystem::Release(Fence& fence)
{
    SyncGPU();
    RemoveFromUniqueSet(fences_, &fence);
}

/* ----- Extended internal functions ----- */

ComPtr<IDXGISwapChain1> D3D12RenderSystem::CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& desc, HWND wnd)
{
    ComPtr<IDXGISwapChain1> swapChain;

    auto hr = factory_->CreateSwapChainForHwnd(commandQueue_->GetNative(), wnd, &desc, nullptr, nullptr, &swapChain);
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");

    return swapChain;
}

void D3D12RenderSystem::SignalFenceValue(UINT64& fenceValue)
{
    auto& fence = commandQueue_->GetGlobalFence();
    fenceValue = fence.GetNextValue();
    commandQueue_->SignalFence(fence, fenceValue);
}

void D3D12RenderSystem::WaitForFenceValue(UINT64 fenceValue)
{
    commandQueue_->GetGlobalFence().WaitForValue(fenceValue);
}

void D3D12RenderSystem::SyncGPU(UINT64& fenceValue)
{
    SignalFenceValue(fenceValue);
    WaitForFenceValue(fenceValue);
}

void D3D12RenderSystem::SyncGPU()
{
    commandQueue_->WaitIdle();
}


/*
 * ======= Private: =======
 */

#ifdef LLGL_DEBUG

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

#endif // /LLGL_DEBUG

void D3D12RenderSystem::CreateFactory()
{
    /* Create DXGI factory 1.4 */
    #ifdef LLGL_DEBUG
    auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory_.ReleaseAndGetAddressOf()));
    #else
    auto hr = CreateDXGIFactory1(IID_PPV_ARGS(factory_.ReleaseAndGetAddressOf()));
    #endif
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
    ComPtr<IDXGIAdapter> adapter;
    auto featureLevels = DXGetFeatureLevels(D3D_FEATURE_LEVEL_12_1);

    /* Try to create a feature level with an hardware adapter */
    HRESULT hr = 0;
    if (!device_.CreateDXDevice(hr, adapter.Get(), featureLevels))
    {
        /* Use software adapter as fallback */
        factory_->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
        if (!device_.CreateDXDevice(hr, adapter.Get(), featureLevels))
            DXThrowIfFailed(hr, "failed to create D3D12 device");
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
        caps.limits.maxBufferSize                   = std::numeric_limits<UINT64>::max();
        caps.limits.maxConstantBufferSize           = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
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

const D3D12RenderPass* D3D12RenderSystem::GetDefaultRenderPass() const
{
    if (!renderContexts_.empty())
    {
        if (auto renderPass = (*renderContexts_.begin())->GetRenderPass())
            return LLGL_CAST(const D3D12RenderPass*, renderPass);
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
