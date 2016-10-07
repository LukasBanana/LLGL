/*
 * D3D12RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
#include "D3DX12/d3dx12.h"
//#include "RenderState/D3D12StateManager.h"

#include "Buffer/D3D12VertexBuffer.h"
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

    /* Create command queue, command allocator, and graphics command list */
    commandQueue_   = CreateDXCommandQueue();
    commandAlloc_   = CreateDXCommandAllocator();
    commandList_    = CreateDXCommandList(commandAlloc_.Get());

    /* Initialize renderer information */
    QueryRendererInfo();
    QueryRenderingCaps();
}

D3D12RenderSystem::~D3D12RenderSystem()
{
    CloseHandle(fenceEvent_);
}

/* ----- Render Context ----- */

RenderContext* D3D12RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return TakeOwnership(renderContexts_, MakeUnique<D3D12RenderContext>(*this, desc, window));
}

void D3D12RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command buffers ----- */

CommandBuffer* D3D12RenderSystem::CreateCommandBuffer()
{
    return TakeOwnership(commandBuffers_, MakeUnique<D3D12CommandBuffer>(*this));
}

void D3D12RenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Hardware Buffers ------ */

// private
std::unique_ptr<D3D12Buffer> D3D12RenderSystem::MakeBufferAndInitialize(const BufferDescriptor& desc, const void* initialData)
{
    std::unique_ptr<D3D12Buffer> buffer;
    ComPtr<ID3D12Resource> bufferUpload;

    /* Create buffer and upload data to GPU */
    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            auto vertexBufferD3D = MakeUnique<D3D12VertexBuffer>(device_.Get(), desc);
            vertexBufferD3D->UpdateSubresource(device_.Get(), commandList_.Get(), bufferUpload, initialData, desc.size);
            buffer = std::move(vertexBufferD3D);
        }
        break;

        case BufferType::Index:
        {
            auto indexBufferD3D = MakeUnique<D3D12IndexBuffer>(device_.Get(), desc);
            indexBufferD3D->UpdateSubresource(device_.Get(), commandList_.Get(), bufferUpload, initialData, desc.size);
            buffer = std::move(indexBufferD3D);
        }
        break;

        case BufferType::Constant:
        {
            auto constantBufferD3D = MakeUnique<D3D12ConstantBuffer>(device_.Get(), desc);
            constantBufferD3D->UpdateSubresource(initialData, desc.size);
            buffer = std::move(constantBufferD3D);
        }
        break;

        case BufferType::Storage:
        {
            auto storageBufferD3D = MakeUnique<D3D12StorageBuffer>(device_.Get(), desc);
            //todo...
            buffer = std::move(storageBufferD3D);
        }
        break;
    }

    /* Execute upload commands and wait for GPU to finish execution */
    CloseAndExecuteCommandList(commandList_.Get());
    SyncGPU();

    return buffer;
}

Buffer* D3D12RenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc);
    return TakeOwnership(buffers_, MakeBufferAndInitialize(desc, initialData));
}

BufferArray* D3D12RenderSystem::CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    return nullptr;//todo
}

void D3D12RenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D12RenderSystem::Release(BufferArray& bufferArray)
{
    //RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void D3D12RenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo...
}

void* D3D12RenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    return nullptr;//todo...
}

void D3D12RenderSystem::UnmapBuffer(Buffer& buffer)
{
    //todo...
}

/* ----- Textures ----- */

Texture* D3D12RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    return nullptr;//TakeOwnership(textures_, MakeUnique<D3D12Texture>());
}

TextureArray* D3D12RenderSystem::CreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
{
    return nullptr;//todo...
}

void D3D12RenderSystem::Release(Texture& texture)
{
    //RemoveFromUniqueSet(textures_, &texture);
}

void D3D12RenderSystem::Release(TextureArray& textureArray)
{
    //RemoveFromUniqueSet(textureArrays_, &textureArray);
}

TextureDescriptor D3D12RenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Setup texture descriptor */
    TextureDescriptor desc;

    //todo

    return desc;
}

void D3D12RenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    LLGL_ASSERT_PTR(buffer);

    //todo
}

void D3D12RenderSystem::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ---- */

Sampler* D3D12RenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return nullptr;//todo
}

void D3D12RenderSystem::Release(Sampler& sampler)
{
    //RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Render Targets ----- */

RenderTarget* D3D12RenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    return nullptr;//TakeOwnership(renderTargets_, MakeUnique<D3D12RenderTarget>(multiSamples));
}

void D3D12RenderSystem::Release(RenderTarget& renderTarget)
{
    //RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* D3D12RenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<D3D12Shader>(type));
}

ShaderProgram* D3D12RenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<D3D12ShaderProgram>());
}

void D3D12RenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void D3D12RenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D12RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<D3D12GraphicsPipeline>(*this, desc));
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


/* ----- Extended internal functions ----- */

ComPtr<IDXGISwapChain1> D3D12RenderSystem::CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& desc, HWND wnd)
{
    ComPtr<IDXGISwapChain1> swapChain;

    auto hr = factory_->CreateSwapChainForHwnd(commandQueue_.Get(), wnd, &desc, nullptr, nullptr, &swapChain);
    DXThrowIfFailed(hr, "failed to create D3D12 swap chain");

    return swapChain;
}

ComPtr<ID3D12CommandQueue> D3D12RenderSystem::CreateDXCommandQueue()
{
    ComPtr<ID3D12CommandQueue> cmdQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    {
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
    auto hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
    DXThrowIfFailed(hr, "failed to create D3D12 command queue");

    return cmdQueue;
}

ComPtr<ID3D12CommandAllocator> D3D12RenderSystem::CreateDXCommandAllocator()
{
    ComPtr<ID3D12CommandAllocator> commandAlloc;

    auto hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAlloc));
    DXThrowIfFailed(hr, "failed to create D3D12 command allocator");

    return commandAlloc;
}

ComPtr<ID3D12GraphicsCommandList> D3D12RenderSystem::CreateDXCommandList(ID3D12CommandAllocator* commandAlloc)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;

    auto hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc, nullptr, IID_PPV_ARGS(&commandList));
    DXThrowIfFailed(hr, "failed to create D3D12 graphics command list");

    return commandList;
}

ComPtr<ID3D12PipelineState> D3D12RenderSystem::CreateDXGfxPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    ComPtr<ID3D12PipelineState> pipelineState;

    auto hr = device_->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
    DXThrowIfFailed(hr, "failed to create D3D12 graphics pipeline state");

    return pipelineState;
}

ComPtr<ID3D12DescriptorHeap> D3D12RenderSystem::CreateDXDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    ComPtr<ID3D12DescriptorHeap> descHeap;

    auto hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap");

    return descHeap;
}

void D3D12RenderSystem::CloseAndExecuteCommandList(ID3D12GraphicsCommandList* commandList)
{
    /* Close graphics command list */
    auto hr = commandList->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");

    /* Execute command list */
    ID3D12CommandList* cmdLists[] = { commandList };
    commandQueue_->ExecuteCommandLists(1, cmdLists);
}

void D3D12RenderSystem::SyncGPU(UINT64& fenceValue)
{
    /* Schedule signal command into the qeue */
    SignalFenceValue(fenceValue);

    /* Wait until the fence has been crossed */
    auto hr = fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
    DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
    WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);

    /* Increment fence value */
    ++fenceValue;
}

void D3D12RenderSystem::SyncGPU()
{
    SyncGPU(fenceValue_);
}

void D3D12RenderSystem::SignalFenceValue(UINT64 fenceValue)
{
    /* Schedule signal command into the qeue */
    auto hr = commandQueue_->Signal(fence_.Get(), fenceValue);
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


/*
 * ======= Private: =======
 */

#ifdef LLGL_DEBUG

void D3D12RenderSystem::EnableDebugLayer()
{
    ComPtr<ID3D12Debug> debugController0;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController0))))
    {
        debugController0->EnableDebugLayer();

        ComPtr<ID3D12Debug1> debugController1;
        if (SUCCEEDED(debugController0->QueryInterface(IID_PPV_ARGS(&debugController1))))
            debugController1->SetEnableGPUBasedValidation(TRUE);
    }
}

#endif

void D3D12RenderSystem::CreateFactory()
{
    /* Create DXGI factory 1.4 */
    #ifdef LLGL_DEBUG
    auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory_));
    #else
    auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory_));
    #endif

    DXThrowIfFailed(hr, "failed to create DXGI factor 1.4");
}

void D3D12RenderSystem::QueryVideoAdapters()
{
    /* Enumerate over all video adapters */
    ComPtr<IDXGIAdapter> adapter;

    for (UINT i = 0; factory_->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        /* Add adapter to the list and release handle */
        videoAdatperDescs_.push_back(DXGetVideoAdapterDesc(adapter.Get()));
        adapter.Reset();
    }
}

void D3D12RenderSystem::CreateDevice()
{
    /* Use default adapter (null) and try all feature levels */
    IDXGIAdapter* adapter = nullptr;
    auto featureLevels = DXGetFeatureLevels(D3D_FEATURE_LEVEL_12_1);

    /* Try to create a feature level with an hardware adapter */
    HRESULT hr = 0;
    if (!CreateDevice(hr, adapter, featureLevels))
    {
        /* Use software adapter as fallback */
        factory_->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
        if (!CreateDevice(hr, adapter, featureLevels))
            DXThrowIfFailed(hr, "failed to create D3D12 device");
    }
}

bool D3D12RenderSystem::CreateDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels)
{
    for (auto level : featureLevels)
    {
        /* Try to create D3D12 device with current feature level */
        hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr))
        {
            /* Store final feature level */
            featureLevel_ = level;
            return true;
        }
    }
    return false;
}

void D3D12RenderSystem::CreateGPUSynchObjects()
{
    /* Create D3D12 fence */
    UINT64 initialFenceValue = 0;
    auto hr = device_->CreateFence(initialFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    DXThrowIfFailed(hr, "failed to create D3D12 fence");
    
    /* Create Win32 event */
    fenceEvent_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
}

void D3D12RenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    info.rendererName           = "Direct3D " + DXFeatureLevelToVersion(GetFeatureLevel());
    info.shadingLanguageName    = "HLSL " + DXFeatureLevelToShaderModel(GetFeatureLevel());
    info.rendererID             = RendererID::Direct3D12;

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
    RenderingCaps caps;
    DXGetRenderingCaps(caps, GetFeatureLevel());
    SetRenderingCaps(caps);
}


} // /namespace LLGL



// ================================================================================
