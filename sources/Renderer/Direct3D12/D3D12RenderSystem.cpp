/*
 * D3D12RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../DXCommon/DXCore.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/Vendor.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include "D3DX12/d3dx12.h"
#include <limits>

#include "Buffer/D3D12Buffer.h"
#include "Buffer/D3D12BufferArray.h"


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
    CreateGPUSynchObjects();

    /* Create command queue interface */
    commandQueue_ = MakeUnique<D3D12CommandQueue>(device_.GetNative(), device_.GetQueue());

    /* Create command queue, command allocator, and graphics command list */
    graphicsCmdAlloc_   = device_.CreateDXCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
    graphicsCmdList_    = device_.CreateDXCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, graphicsCmdAlloc_.Get());

    computeCmdAlloc_    = device_.CreateDXCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    computeCmdList_     = device_.CreateDXCommandList(D3D12_COMMAND_LIST_TYPE_COMPUTE, computeCmdAlloc_.Get());

    /* Create default pipeline layout and command signature pool */
    defaultPipelineLayout_.CreateRootSignature(device_.GetNative(), {});
    commandSignaturePool_.CreateDefaultSignatures(device_.GetNative());
    commandContext_.SetCommandList(graphicsCmdList_.Get());
    mipGenerator_.CreateRootSignature(device_.GetNative());

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

    CloseHandle(fenceEvent_);
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
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

// private
std::unique_ptr<D3D12Buffer> D3D12RenderSystem::CreateGpuBuffer(const BufferDescriptor& desc, const void* initialData)
{
    /* Create buffer and upload data to GPU */
    ComPtr<ID3D12Resource> uploadBuffer;

    auto bufferD3D = MakeUnique<D3D12Buffer>(device_.GetNative(), desc);

    if (initialData)
    {
        bufferD3D->UpdateStaticSubresource(
            device_.GetNative(),
            commandContext_,
            uploadBuffer,
            initialData,
            desc.size
        );
    }

    /* Execute upload commands and wait for GPU to finish execution */
    ExecuteCommandList();
    SyncGPU();

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
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D12RenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

//TODO: execute command list only before the next call to D3D12CommandBuffer::Begin()
void D3D12RenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    dstBufferD3D.UpdateDynamicSubresource(commandContext_, data, dataSize, dstOffset);
    ExecuteCommandList();
}

void* D3D12RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    return nullptr;//todo...
}

void D3D12RenderSystem::UnmapBuffer(Buffer& buffer)
{
    //todo...
}

/* ----- Textures ----- */

Texture* D3D12RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    auto textureD3D = MakeUnique<D3D12Texture>(device_.GetNative(), textureDesc);

    if (imageDesc != nullptr)
    {
        /* Upload image data */
        ComPtr<ID3D12Resource> uploadBuffer;

        /* Get texture dimensions */
        auto texWidth   = textureDesc.extent.width;
        auto texHeight  = textureDesc.extent.height;

        if (textureDesc.type == TextureType::Texture1D || textureDesc.type == TextureType::Texture1DArray)
            texHeight = 1u;

        /* Check if image data conversion is necessary */
        auto initialData    = imageDesc->data;
        auto dstTexFormat   = DXGetTextureFormatDesc(textureD3D->GetFormat());

        ByteBuffer tempImageData;

        if (dstTexFormat.format != imageDesc->format || dstTexFormat.dataType != imageDesc->dataType)
        {
            /* Convert image data (e.g. from RGB to RGBA) */
            tempImageData = ConvertImageBuffer(*imageDesc, dstTexFormat.format, dstTexFormat.dataType, GetConfiguration().threadCount);
            initialData = tempImageData.get();
        }

        /* Upload image data to subresource */
        D3D12_SUBRESOURCE_DATA subresourceData;
        {
            subresourceData.pData       = initialData;
            subresourceData.RowPitch    = ImageFormatSize(dstTexFormat.format) * DataTypeSize(dstTexFormat.dataType) * texWidth;
            subresourceData.SlicePitch  = subresourceData.RowPitch * texHeight;
        }
        textureD3D->UpdateSubresource(device_.GetNative(), graphicsCmdList_.Get(), uploadBuffer, subresourceData);

        #if 0//TODO: GenerateMips
        /* Generate MIP-maps if enabled */
        if (FlagsRequireGenerateMips(textureDesc.miscFlags))
        {
            //...
        }
        #endif

        /* Execute upload commands and wait for GPU to finish execution */
        ExecuteCommandList();
        SyncGPU();
    }

    return TakeOwnership(textures_, std::move(textureD3D));
}

void D3D12RenderSystem::Release(Texture& texture)
{
    //RemoveFromUniqueSet(textures_, &texture);
}

void D3D12RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    //todo
}

#if 1//TODO: remove this
void D3D12RenderSystem::GenerateMips(Texture& texture){}
void D3D12RenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers){}
#endif

/* ----- Sampler States ---- */

Sampler* D3D12RenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<D3D12Sampler>(desc));
}

void D3D12RenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D12RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<D3D12ResourceHeap>(device_.GetNative(), desc));
}

void D3D12RenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* D3D12RenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return TakeOwnership(renderPasses_, MakeUnique<D3D12RenderPass>(desc));
}

void D3D12RenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D12RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<D3D12RenderTarget>(device_, desc));
}

void D3D12RenderSystem::Release(RenderTarget& renderTarget)
{
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
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D12RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(
        graphicsPipelines_,
        MakeUnique<D3D12GraphicsPipeline>(device_, defaultPipelineLayout_.GetRootSignature(), desc)
    );
}

ComputePipeline* D3D12RenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return TakeOwnership(
        computePipelines_,
        MakeUnique<D3D12ComputePipeline>(device_, defaultPipelineLayout_.GetRootSignature(), desc)
    );
}

void D3D12RenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void D3D12RenderSystem::Release(ComputePipeline& computePipeline)
{
    //RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

QueryHeap* D3D12RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<D3D12QueryHeap>(device_, desc));
}

void D3D12RenderSystem::Release(QueryHeap& queryHeap)
{
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* D3D12RenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<D3D12Fence>(device_.GetNative(), 0));
}

void D3D12RenderSystem::Release(Fence& fence)
{
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

void D3D12RenderSystem::SignalFenceValue(UINT64 fenceValue)
{
    /* Schedule signal command into the qeue */
    auto hr = commandQueue_->GetNative()->Signal(fence_.Get(), fenceValue);
    DXThrowIfFailed(hr, "failed to signal D3D12 fence into command queue");
}

void D3D12RenderSystem::WaitForFenceValue(UINT64 fenceValue)
{
    /* Wait until the fence has been crossed */
    if (fence_->GetCompletedValue() < fenceValue)
    {
        auto hr = fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
        DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
        WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
    }
}

void D3D12RenderSystem::SyncGPU(UINT64& fenceValue)
{
    /* Increment fence value */
    ++fenceValue;
    fenceValue_ = fenceValue;

    /* Schedule signal command into the qeue */
    SignalFenceValue(fenceValue);

    /* Wait until the fence has been processed */
    auto hr = fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
    DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
    WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
}

void D3D12RenderSystem::SyncGPU()
{
    SyncGPU(fenceValue_);
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

#endif

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

void D3D12RenderSystem::CreateGPUSynchObjects()
{
    /* Create D3D12 fence */
    UINT64 initialFenceValue = 0;
    auto hr = device_.GetNative()->CreateFence(initialFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 fence");

    /* Create Win32 event */
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!fenceEvent_)
        DXThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "failed to create Win32 event object");
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
        info.deviceName = std::string(videoAdapterDesc.name.begin(), videoAdapterDesc.name.end());
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

//TODO: also reset allocator!
void D3D12RenderSystem::ExecuteCommandList()
{
    device_.CloseAndExecuteCommandList(graphicsCmdList_.Get());

    /* Reset command list */
    auto hr = graphicsCmdList_->Reset(graphicsCmdAlloc_.Get(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");
}


} // /namespace LLGL



// ================================================================================
