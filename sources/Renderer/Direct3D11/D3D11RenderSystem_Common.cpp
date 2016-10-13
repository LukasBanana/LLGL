/*
 * D3D11RenderSystem_Common.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
#include <sstream>
#include <iomanip>

#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11VertexBufferArray.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"
#include "Buffer/D3D11StorageBuffer.h"
#include "Buffer/D3D11StorageBufferArray.h"
#include "Buffer/D3D11StreamOutputBuffer.h"
#include "Buffer/D3D11StreamOutputBufferArray.h"


namespace LLGL
{


D3D11RenderSystem::D3D11RenderSystem()
{
    /* Create DXGU factory, query video adapters, and create D3D11 device */
    CreateFactory();
    QueryVideoAdapters();
    CreateDevice(nullptr);

    /* Initialize states and renderer information */
    InitStateManager();
    QueryRendererInfo();
    QueryRenderingCaps();
}

D3D11RenderSystem::~D3D11RenderSystem()
{
}

/* ----- Render Context ----- */

RenderContext* D3D11RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return TakeOwnership(
        renderContexts_,
        MakeUnique<D3D11RenderContext>(factory_.Get(), device_, context_, desc, window)
    );
}

void D3D11RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command buffers ----- */

CommandBuffer* D3D11RenderSystem::CreateCommandBuffer()
{
    return TakeOwnership(commandBuffers_, MakeUnique<D3D11CommandBuffer>(*stateMngr_, context_));
}

void D3D11RenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

static std::unique_ptr<D3D11Buffer> MakeD3D11Buffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData)
{
    /* Make respective buffer type */
    switch (desc.type)
    {
        case BufferType::Vertex:        return MakeUnique< D3D11VertexBuffer       >(device, desc, initialData);
        case BufferType::Index:         return MakeUnique< D3D11IndexBuffer        >(device, desc, initialData);
        case BufferType::Constant:      return MakeUnique< D3D11ConstantBuffer     >(device, desc, initialData);
        case BufferType::Storage:       return MakeUnique< D3D11StorageBuffer      >(device, desc, initialData);
        case BufferType::StreamOutput:  return MakeUnique< D3D11StreamOutputBuffer >(device, desc, initialData);
    }
    return nullptr;
}

Buffer* D3D11RenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc);
    return TakeOwnership(buffers_, MakeD3D11Buffer(device_.Get(), desc, initialData));
}

static std::unique_ptr<D3D11BufferArray> MakeD3D11BufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    auto type = (*bufferArray)->GetType();
    switch (type)
    {
        case BufferType::Vertex:        return MakeUnique<D3D11VertexBufferArray>(numBuffers, bufferArray);
        case BufferType::Constant:      return MakeUnique<D3D11BufferArray>(type, numBuffers, bufferArray);
        case BufferType::Storage:       return MakeUnique<D3D11StorageBufferArray>(numBuffers, bufferArray);
        case BufferType::StreamOutput:  return MakeUnique<D3D11StreamOutputBufferArray>(numBuffers, bufferArray);
        default:                        break;
    }
    return nullptr;
}

BufferArray* D3D11RenderSystem::CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    return TakeOwnership(bufferArrays_, MakeD3D11BufferArray(numBuffers, bufferArray));
}

void D3D11RenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D11RenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void D3D11RenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    mappedBufferCPUAccess_ = access;
    return bufferD3D.Map(context_.Get(), mappedBufferCPUAccess_);
}

void D3D11RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.Unmap(context_.Get(), mappedBufferCPUAccess_);
}

/* ----- Textures ----- */

// --> see "D3D11RenderSystem_Textures.cpp" file

/* ----- Sampler States ---- */

Sampler* D3D11RenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<D3D11Sampler>(device_.Get(), desc));
}

SamplerArray* D3D11RenderSystem::CreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray)
{
    AssertCreateSamplerArray(numSamplers, samplerArray);
    return TakeOwnership(samplerArrays_, MakeUnique<D3D11SamplerArray>(numSamplers, samplerArray));
}

void D3D11RenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

void D3D11RenderSystem::Release(SamplerArray& samplerArray)
{
    RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Render Targets ----- */

RenderTarget* D3D11RenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    return TakeOwnership(renderTargets_, MakeUnique<D3D11RenderTarget>(device_.Get(), multiSamples));
}

void D3D11RenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* D3D11RenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<D3D11Shader>(device_.Get(), type));
}

ShaderProgram* D3D11RenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<D3D11ShaderProgram>(device_.Get()));
}

void D3D11RenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void D3D11RenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D11RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return TakeOwnership(graphicsPipelines_, MakeUnique<D3D11GraphicsPipeline>(device_.Get(), desc));
}

ComputePipeline* D3D11RenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return TakeOwnership(computePipelines_, MakeUnique<D3D11ComputePipeline>(desc));
}

void D3D11RenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void D3D11RenderSystem::Release(ComputePipeline& computePipeline)
{
    RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* D3D11RenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return TakeOwnership(queries_, MakeUnique<D3D11Query>(device_.Get(), desc));
}

void D3D11RenderSystem::Release(Query& query)
{
    RemoveFromUniqueSet(queries_, &query);
}


/*
 * ======= Private: =======
 */

void D3D11RenderSystem::CreateFactory()
{
    /* Create DXGI factory */
    auto hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
    DXThrowIfFailed(hr, "failed to create DXGI factor");
}

void D3D11RenderSystem::QueryVideoAdapters()
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

void D3D11RenderSystem::CreateDevice(IDXGIAdapter* adapter)
{
    /* Use default adapter (null) and try all feature levels */
    auto    featureLevels   = DXGetFeatureLevels(D3D_FEATURE_LEVEL_11_1);
    HRESULT hr              = 0;
    UINT    flags           = 0;

    #ifdef LLGL_DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    for (D3D_DRIVER_TYPE driver : { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE })
    {
        auto hr = D3D11CreateDevice(
            adapter,                            // Video adapter
            driver,                             // Driver type
            0,                                  // Software rasterizer module (none)
            flags,                              // Flags
            featureLevels.data(),               // Feature level
            featureLevels.size(),               // Num feature levels
            D3D11_SDK_VERSION,                  // SDK version
            device_.ReleaseAndGetAddressOf(),   // Output device
            &featureLevel_,                     // Output feature level
            context_.ReleaseAndGetAddressOf()   // Output device context
        );

        if (SUCCEEDED(hr))
            break;
    }

    DXThrowIfFailed(hr, "failed to create D3D11 device");
}

void D3D11RenderSystem::InitStateManager()
{
    /* Create state manager */
    stateMngr_ = MakeUnique<D3D11StateManager>(context_);
}

void D3D11RenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    info.rendererName           = "Direct3D " + DXFeatureLevelToVersion(GetFeatureLevel());
    info.shadingLanguageName    = "HLSL " + DXFeatureLevelToShaderModel(GetFeatureLevel());
    info.rendererID             = RendererID::Direct3D11;

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

void D3D11RenderSystem::QueryRenderingCaps()
{
    RenderingCaps caps;
    DXGetRenderingCaps(caps, GetFeatureLevel());
    SetRenderingCaps(caps);
}


} // /namespace LLGL



// ================================================================================
