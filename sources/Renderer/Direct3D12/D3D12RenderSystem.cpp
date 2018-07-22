/*
 * D3D12RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../../Core/Vendor.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include "D3DX12/d3dx12.h"
#include <limits>

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12VertexBufferArray.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"


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

    /* Create default pipeline layout */
    defaultPipelineLayout_.CreateRootSignature(device_.GetNative(), {});

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
    return TakeOwnership(commandBuffers_, MakeUnique<D3D12CommandBuffer>(*this));
}

CommandBufferExt* D3D12RenderSystem::CreateCommandBufferExt(const CommandBufferDescriptor& /*desc*/)
{
    /* Extended command buffers are not spported */
    return nullptr;
}

void D3D12RenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

static std::unique_ptr<D3D12Buffer> MakeD3D12VertexBuffer(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    const BufferDescriptor&     desc,
    const void*                 initialData)
{
    auto bufferD3D = MakeUnique<D3D12VertexBuffer>(device, desc);

    if (initialData)
        bufferD3D->UpdateSubresource(device, commandList, uploadBuffer, initialData, static_cast<UINT>(desc.size));

    return std::move(bufferD3D);
}

static std::unique_ptr<D3D12Buffer> MakeD3D12IndexBuffer(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    const BufferDescriptor&     desc,
    const void*                 initialData)
{
    auto bufferD3D = MakeUnique<D3D12IndexBuffer>(device, desc);

    if (initialData)
        bufferD3D->UpdateSubresource(device, commandList, uploadBuffer, initialData, static_cast<UINT>(desc.size));

    return std::move(bufferD3D);
}

static std::unique_ptr<D3D12Buffer> MakeD3D12ConstantBuffer(
    ID3D12Device*           device,
    const BufferDescriptor& desc,
    const void*             initialData)
{
    auto bufferD3D = MakeUnique<D3D12ConstantBuffer>(device, desc);

    if (initialData)
        bufferD3D->UpdateSubresource(initialData, static_cast<UINT>(desc.size));

    return std::move(bufferD3D);
}

static std::unique_ptr<D3D12Buffer> MakeD3D12StorageBuffer(
    ID3D12Device*           device,
    const BufferDescriptor& desc,
    const void*             /*initialData*/)
{
    auto bufferD3D = MakeUnique<D3D12StorageBuffer>(device, desc);

    //TODO...

    return std::move(bufferD3D);
}

static std::unique_ptr<D3D12Buffer> MakeD3D12Buffer(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    const BufferDescriptor&     desc,
    const void*                 initialData)
{
    switch (desc.type)
    {
        case BufferType::Vertex:    return MakeD3D12VertexBuffer(device, commandList, uploadBuffer, desc, initialData);
        case BufferType::Index:     return MakeD3D12IndexBuffer(device, commandList, uploadBuffer, desc, initialData);
        case BufferType::Constant:  return MakeD3D12ConstantBuffer(device, desc, initialData);
        case BufferType::Storage:   return MakeD3D12StorageBuffer(device, desc, initialData);
        default:                    return nullptr;
    }
}

// private
std::unique_ptr<D3D12Buffer> D3D12RenderSystem::MakeBufferAndInitialize(const BufferDescriptor& desc, const void* initialData)
{
    /* Create buffer and upload data to GPU */
    ComPtr<ID3D12Resource> uploadBuffer;

    auto buffer = MakeD3D12Buffer(device_.GetNative(), graphicsCmdList_.Get(), uploadBuffer, desc, initialData);

    /* Execute upload commands and wait for GPU to finish execution */
    ExecuteCommandList();
    SyncGPU();

    return buffer;
}

Buffer* D3D12RenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<UINT64>::max()));
    return TakeOwnership(buffers_, MakeBufferAndInitialize(desc, initialData));
}

static std::unique_ptr<BufferArray> MakeD3D12BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    auto type = (*bufferArray)->GetType();
    switch (type)
    {
        case BufferType::Vertex:        return MakeUnique<D3D12VertexBufferArray>(numBuffers, bufferArray);
      /*case BufferType::Constant:      return MakeUnique<D3D12BufferArray>(type, numBuffers, bufferArray);
        case BufferType::Storage:       return MakeUnique<D3D12StorageBufferArray>(numBuffers, bufferArray);
        case BufferType::StreamOutput:  return MakeUnique<D3D12StreamOutputBufferArray>(numBuffers, bufferArray);*/
        default:                        return nullptr;
    }
}

BufferArray* D3D12RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    return TakeOwnership(bufferArrays_, MakeD3D12BufferArray(numBuffers, bufferArray));
}

void D3D12RenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D12RenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void D3D12RenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D12Buffer&, dstBuffer);
    dstBufferD3D.UpdateDynamicSubresource(data, dataSize, dstOffset);
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

    /* Upload image data */
    ComPtr<ID3D12Resource> uploadBuffer;

    if (imageDesc)
    {
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
    }

    /* Execute upload commands and wait for GPU to finish execution */
    ExecuteCommandList();
    SyncGPU();

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

void D3D12RenderSystem::GenerateMips(Texture& texture)
{
    //todo
}

void D3D12RenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    //todo
}

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
    return nullptr;//TakeOwnership(renderTargets_, MakeUnique<D3D12RenderTarget>(desc));
}

void D3D12RenderSystem::Release(RenderTarget& renderTarget)
{
    //RemoveFromUniqueSet(renderTargets_, &renderTarget);
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
    return nullptr;//todo...
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

Query* D3D12RenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return nullptr;//todo...
}

void D3D12RenderSystem::Release(Query& query)
{
    //todo...
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
    #if 0
    fenceEvent_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    #else
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    #endif
    if (!fenceEvent_)
        DXThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "failed to create Win32 event object");
}

void D3D12RenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    info.rendererName           = "Direct3D " + DXFeatureLevelToVersion(GetFeatureLevel());
    info.shadingLanguageName    = "HLSL " + DXFeatureLevelToShaderModel(GetFeatureLevel());

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

        caps.limits.maxNumViewports                 = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
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
