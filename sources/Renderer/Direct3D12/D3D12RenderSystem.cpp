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


namespace LLGL
{


D3D12RenderSystem::D3D12RenderSystem()
{
    #ifdef LLGL_DEBUG
    EnableDebugLayer();
    #endif

    /* Create DXGU factory 1.4, query video adapters, and create D3D12 device */
    CreateFactory();
    //QueryVideoAdapters();
    CreateDevice();
    CreateGPUSynchObjects();

    /* Create command queue, command allocator, and graphics command list */
    commandQueue_   = CreateDXCommandQueue();
    commandAlloc_   = CreateDXCommandAllocator();
    commandList_    = CreateDXCommandList(commandAlloc_.Get());
}

D3D12RenderSystem::~D3D12RenderSystem()
{
    CloseHandle(fenceEvent_);
}

std::map<RendererInfo, std::string> D3D12RenderSystem::QueryRendererInfo() const
{
    std::map<RendererInfo, std::string> info;

    //todo

    return info;
}

RenderingCaps D3D12RenderSystem::QueryRenderingCaps() const
{
    RenderingCaps caps;
    DXGetRenderingCaps(caps, GetFeatureLevel());
    return caps;
}

ShadingLanguage D3D12RenderSystem::QueryShadingLanguage() const
{
    return DXGetHLSLVersion(GetFeatureLevel());
}

/* ----- Render Context ----- */

RenderContext* D3D12RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    /* Create new render context and make it the current one */
    auto renderContext = MakeUnique<D3D12RenderContext>(*this, desc, window);
    MakeCurrent(renderContext.get());

    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    /* Take ownership and return new render context */
    return TakeOwnership(renderContexts_, std::move(renderContext));
}

void D3D12RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

VertexBuffer* D3D12RenderSystem::CreateVertexBuffer()
{
    return TakeOwnership(vertexBuffers_, MakeUnique<D3D12VertexBuffer>());
}

IndexBuffer* D3D12RenderSystem::CreateIndexBuffer()
{
    return TakeOwnership(indexBuffers_, MakeUnique<D3D12IndexBuffer>());
}

ConstantBuffer* D3D12RenderSystem::CreateConstantBuffer()
{
    return TakeOwnership(constantBuffers_, MakeUnique<D3D12ConstantBuffer>(device_.Get()));
}

StorageBuffer* D3D12RenderSystem::CreateStorageBuffer()
{
    return TakeOwnership(storageBuffers_, MakeUnique<D3D12StorageBuffer>());
}

void D3D12RenderSystem::Release(VertexBuffer& vertexBuffer)
{
    RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void D3D12RenderSystem::Release(IndexBuffer& indexBuffer)
{
    RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void D3D12RenderSystem::Release(ConstantBuffer& constantBuffer)
{
    RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void D3D12RenderSystem::Release(StorageBuffer& storageBuffer)
{
    RemoveFromUniqueSet(storageBuffers_, &storageBuffer);
}

void D3D12RenderSystem::SetupVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D12VertexBuffer&, vertexBuffer);

    /* Create hardware buffer resource */
    vertexBufferD3D.hwBuffer.CreateResource(device_.Get(), dataSize);
    vertexBufferD3D.PutView(vertexFormat.GetFormatSize());

    /* Upload buffer data to GPU */
    ComPtr<ID3D12Resource> bufferUpload;
    vertexBufferD3D.UpdateSubResource(device_.Get(), commandList_.Get(), bufferUpload, data, dataSize);

    /* Execute upload commands and wait for GPU to finish execution */
    CloseAndExecuteCommandList(commandList_.Get());
    SyncGPU();
}

void D3D12RenderSystem::SetupIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    auto& indexBufferD3D = LLGL_CAST(D3D12IndexBuffer&, indexBuffer);

    /* Create hardware buffer resource */
    indexBufferD3D.hwBuffer.CreateResource(device_.Get(), dataSize);
    indexBufferD3D.PutView(D3D12Types::Map(indexFormat.GetDataType()));

    /* Upload buffer data to GPU */
    ComPtr<ID3D12Resource> bufferUpload;
    indexBufferD3D.UpdateSubResource(device_.Get(), commandList_.Get(), bufferUpload, data, dataSize);

    /* Execute upload commands and wait for GPU to finish execution */
    CloseAndExecuteCommandList(commandList_.Get());
    SyncGPU();
}

void D3D12RenderSystem::SetupConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    auto& constantBufferD3D = LLGL_CAST(D3D12ConstantBuffer&, constantBuffer);

    /* Create hardware buffer resource */
    constantBufferD3D.CreateResourceAndPutView(device_.Get(), dataSize);

    /* Upload buffer data to GPU */
    //ComPtr<ID3D12Resource> bufferUpload;
    constantBufferD3D.UpdateSubResource(data, dataSize);

    /* Execute upload commands and wait for GPU to finish execution */
    //CloseAndExecuteCommandList(commandList_.Get());
    //SyncGPU();
}

void D3D12RenderSystem::SetupStorageBuffer(
    StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    //todo
}

void D3D12RenderSystem::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void D3D12RenderSystem::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void D3D12RenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void D3D12RenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

/* ----- Textures ----- */

Texture* D3D12RenderSystem::CreateTexture()
{
    return nullptr;//TakeOwnership(textures_, MakeUnique<D3D12Texture>());
}

void D3D12RenderSystem::Release(Texture& texture)
{
    //RemoveFromUniqueSet(textures_, &texture);
}

TextureDescriptor D3D12RenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Setup texture descriptor */
    TextureDescriptor desc;
    InitMemory(desc);

    //todo

    return desc;
}

void D3D12RenderSystem::SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture1D(
    Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture2D(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture3D(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTextureCube(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture1DArray(
    Texture& texture, int mipLevel, int position, unsigned int layerOffset,
    int size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture2DArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
    const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTextureCubeArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
    const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data)
{
    LLGL_ASSERT_PTR(data);

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

Query* D3D12RenderSystem::CreateQuery(const QueryType type)
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
    if (activeRenderContext_)
        activeRenderContext_->SyncGPU();
    else
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

bool D3D12RenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    activeRenderContext_ = LLGL_CAST(D3D12RenderContext*, renderContext);
    return true;
}

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
    ComPtr<IDXGIOutput> output;

    for (UINT i = 0; factory_->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        /* Query adapter description */
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        /* Setup adapter information */
        VideoAdapterDescriptor videoAdapterDesc;

        videoAdapterDesc.name           = std::wstring(desc.Description);
        videoAdapterDesc.vendor         = GetVendorByID(desc.VendorId);
        videoAdapterDesc.videoMemory    = desc.DedicatedVideoMemory;

        /* Enumerate over all adapter outputs */
        for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; ++j)
        {
            /* Get output description */
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);

            /* Query number of display modes */
            UINT numModes = 0;
            output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);

            /* Query display modes */
            std::vector<DXGI_MODE_DESC> modeDesc(numModes);

            auto hr = output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, modeDesc.data());
            DXThrowIfFailed(hr, "failed to get display mode list with format DXGI_FORMAT_R8G8B8A8_UNORM");

            /* Add output information to the current adapter */
            VideoOutput videoOutput;
            
            for (UINT i = 0; i < numModes; ++i)
            {
                VideoDisplayMode displayMode;
                {
                    displayMode.width       = modeDesc[i].Width;
                    displayMode.height      = modeDesc[i].Height;
                    displayMode.refreshRate = (modeDesc[i].RefreshRate.Denominator > 0 ? modeDesc[i].RefreshRate.Numerator / modeDesc[i].RefreshRate.Denominator : 0);
                }
                videoOutput.displayModes.push_back(displayMode);
            }
            
            /* Remove duplicate display modes */
            std::sort(videoOutput.displayModes.begin(), videoOutput.displayModes.end(), CompareSWO);

            videoOutput.displayModes.erase(
                std::unique(videoOutput.displayModes.begin(), videoOutput.displayModes.end()),
                videoOutput.displayModes.end()
            );

            /* Add output to the list and release handle */
            videoAdapterDesc.outputs.push_back(videoOutput);

            output.Reset();
        }

        /* Add adapter to the list and release handle */
        videoAdatperDescs_.push_back(videoAdapterDesc);

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


} // /namespace LLGL



// ================================================================================
