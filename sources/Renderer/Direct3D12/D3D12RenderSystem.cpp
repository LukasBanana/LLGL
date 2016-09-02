/*
 * D3D12RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderSystem.h"
#include "D3D12Core.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
//#include "RenderState/D3D12StateManager.h"


namespace LLGL
{


D3D12RenderSystem::D3D12RenderSystem()
{
    /* Create DXGU factory 1.4, query video adapters, and create D3D12 device */
    CreateFactory();
    QueryVideoAdapters();
    CreateDevice();
    CreateGPUSynchObjects();

    /* Create main command queue */
    cmdQueue_ = CreateDXCommandQueue();
}

D3D12RenderSystem::~D3D12RenderSystem()
{
    SafeRelease(cmdQueue_);
    SafeRelease(device_);
    SafeRelease(factory_);
}

RenderContext* D3D12RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    auto renderContext = MakeUnique<D3D12RenderContext>(*this, desc, window);

    /*
    If render context created it's own window then show it after creation,
    since anti-aliasing may force the window to be recreated several times
    */
    if (!window)
        renderContext->GetWindow().Show();

    return TakeOwnership(renderContexts_, std::move(renderContext));
}

void D3D12RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

VertexBuffer* D3D12RenderSystem::CreateVertexBuffer()
{
    return nullptr;//TakeOwnership(vertexBuffers_, MakeUnique<D3D12VertexBuffer>());
}

IndexBuffer* D3D12RenderSystem::CreateIndexBuffer()
{
    return nullptr;//TakeOwnership(indexBuffers_, MakeUnique<D3D12IndexBuffer>());
}

ConstantBuffer* D3D12RenderSystem::CreateConstantBuffer()
{
    return nullptr;//TakeOwnership(constantBuffers_, MakeUnique<D3D12ConstantBuffer>());
}

void D3D12RenderSystem::Release(VertexBuffer& vertexBuffer)
{
    //RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void D3D12RenderSystem::Release(IndexBuffer& indexBuffer)
{
    //RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void D3D12RenderSystem::Release(ConstantBuffer& constantBuffer)
{
    //RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void D3D12RenderSystem::WriteVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    //todo
}

void D3D12RenderSystem::WriteIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    //todo
}

void D3D12RenderSystem::WriteConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    //todo
}

void D3D12RenderSystem::WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void D3D12RenderSystem::WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void D3D12RenderSystem::WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
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

void D3D12RenderSystem::WriteTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    //todo
}

void D3D12RenderSystem::WriteTexture1DSub(
    Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture2DSub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture3DSub(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTextureCubeSub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture1DArraySub(
    Texture& texture, int mipLevel, int position, unsigned int layers, int size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTexture2DArraySub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    //todo...
}

void D3D12RenderSystem::WriteTextureCubeArraySub(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
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
    return nullptr;//TakeOwnership(shaders_, MakeUnique<D3D12Shader>(type));
}

ShaderProgram* D3D12RenderSystem::CreateShaderProgram()
{
    return nullptr;//TakeOwnership(shaderPrograms_, MakeUnique<D3D12ShaderProgram>());
}

void D3D12RenderSystem::Release(Shader& shader)
{
    //RemoveFromUniqueSet(shaders_, &shader);
}

void D3D12RenderSystem::Release(ShaderProgram& shaderProgram)
{
    //RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D12RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return nullptr;//TakeOwnership(graphicsPipelines_, MakeUnique<D3D12GraphicsPipeline>(desc));
}

void D3D12RenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    //RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

/* ----- Extended internal functions ----- */

ID3D12CommandQueue* D3D12RenderSystem::CreateDXCommandQueue()
{
    ID3D12CommandQueue* cmdQueue = nullptr;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    {
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
    auto hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
    DXThrowIfFailed(hr, "failed to create D3D12 command queue");

    return cmdQueue;
}

ID3D12CommandAllocator* D3D12RenderSystem::CreateDXCommandAllocator()
{
    ID3D12CommandAllocator* cmdAlloc = nullptr;

    auto hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
    DXThrowIfFailed(hr, "failed to create D3D12 command allocator");

    return cmdAlloc;
}

ID3D12DescriptorHeap* D3D12RenderSystem::CreateDXDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    ID3D12DescriptorHeap* descHeap = nullptr;

    auto hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap");

    return descHeap;
}

IDXGISwapChain1* D3D12RenderSystem::CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& desc, HWND wnd)
{
    IDXGISwapChain1* swapChain = nullptr;

    auto hr = factory_->CreateSwapChainForHwnd(cmdQueue_, wnd, &desc, nullptr, nullptr, &swapChain);
    DXThrowIfFailed(hr, "failed to create D3D12 swap chain");

    return swapChain;
}

void D3D12RenderSystem::SyncGPU(UINT64& fenceValue)
{
    HRESULT hr = 0;

    /* Schedule signal command into the qeue */
    hr = cmdQueue_->Signal(fence_, fenceValue);
    DXThrowIfFailed(hr, "failed to signal D3D12 fence into command queue");

    /* Wait until the fence has been crossed */
    hr = fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
    DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
    WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);

    /* Increment fence value */
    ++fenceValue;
}


/*
 * ======= Private: =======
 */

void D3D12RenderSystem::CreateFactory()
{
    /* Create DXGI factory 1.4 */
    auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory_));
    DXThrowIfFailed(hr, "failed to create DXGI factor 1.4");
}

void D3D12RenderSystem::QueryVideoAdapters()
{
    /* Enumerate over all video adapters */
    IDXGIAdapter* adapter = nullptr;
    IDXGIOutput* output = nullptr;

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

            output->Release();
        }

        /* Add adapter to the list and release handle */
        videoAdatperDescs_.push_back(videoAdapterDesc);

        adapter->Release();
    }
}

static std::vector<D3D_FEATURE_LEVEL> GetFeatureLevels()
{
    return
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
}

void D3D12RenderSystem::CreateDevice()
{
    /* Use default adapter (null) and try all feature levels */
    IDXGIAdapter* adapter = nullptr;
    auto featureLevels = GetFeatureLevels();

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
        hr = D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&device_));
        if (!FAILED(hr))
            return true;
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
