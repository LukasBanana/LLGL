/*
 * D3D11RenderSystem_Common.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../../Core/Vendor.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include <sstream>
#include <iomanip>
#include <limits>

#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11VertexBufferArray.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"
#include "Buffer/D3D11StorageBuffer.h"
#include "Buffer/D3D11StorageBufferArray.h"
#include "Buffer/D3D11StreamOutputBuffer.h"
#include "Buffer/D3D11StreamOutputBufferArray.h"

#include "RenderState/D3D11GraphicsPipeline.h"
#include "RenderState/D3D11GraphicsPipeline1.h"
#include "RenderState/D3D11GraphicsPipeline3.h"


namespace LLGL
{


D3D11RenderSystem::D3D11RenderSystem()
{
    /* Create DXGU factory, query video adapters, and create D3D11 device */
    CreateFactory();
    QueryVideoAdapters();
    CreateDevice(nullptr);

    /* Initialize states and renderer information */
    CreateStateManagerAndCommandQueue();
    QueryRendererInfo();
    QueryRenderingCaps();
}

D3D11RenderSystem::~D3D11RenderSystem()
{
}

/* ----- Render Context ----- */

RenderContext* D3D11RenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(
        renderContexts_,
        MakeUnique<D3D11RenderContext>(factory_.Get(), device_, context_, desc, surface)
    );
}

void D3D11RenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* D3D11RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D11RenderSystem::CreateCommandBuffer()
{
    return CreateCommandBufferExt();
}

CommandBufferExt* D3D11RenderSystem::CreateCommandBufferExt()
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
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<UINT>::max()));
    return TakeOwnership(buffers_, MakeD3D11Buffer(device_.Get(), desc, initialData));
}

static std::unique_ptr<D3D11BufferArray> MakeD3D11BufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
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

BufferArray* D3D11RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
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

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
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

void D3D11RenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D11RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<D3D11ResourceHeap>(desc));
}

void D3D11RenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Targets ----- */

RenderTarget* D3D11RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<D3D11RenderTarget>(device_.Get(), desc));
}

void D3D11RenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* D3D11RenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    AssertCreateShader(desc);
    return TakeOwnership(shaders_, MakeUnique<D3D11Shader>(device_.Get(), desc));
}

ShaderProgram* D3D11RenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    AssertCreateShaderProgram(desc);
    return TakeOwnership(shaderPrograms_, MakeUnique<D3D11ShaderProgram>(device_.Get(), desc));
}

void D3D11RenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void D3D11RenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D11RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<D3D11PipelineLayout>(desc));
}

void D3D11RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* D3D11RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    if (device3_)
    {
        /* Create graphics pipeline for Direct3D 11.3 */
        return TakeOwnership(graphicsPipelines_, MakeUnique<D3D11GraphicsPipeline3>(device3_.Get(), desc));
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
    if (device2_)
    {
        /* Create graphics pipeline for Direct3D 11.1 (there is no dedicated class for 11.2) */
        return TakeOwnership(graphicsPipelines_, MakeUnique<D3D11GraphicsPipeline1>(device2_.Get(), desc));
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (device1_)
    {
        /* Create graphics pipeline for Direct3D 11.1 */
        return TakeOwnership(graphicsPipelines_, MakeUnique<D3D11GraphicsPipeline1>(device1_.Get(), desc));
    }
    #endif

    /* Create graphics pipeline for Direct3D 11.0 */
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

/* ----- Fences ----- */

Fence* D3D11RenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<D3D11Fence>(/*device_.Get(), 0*/));
}

void D3D11RenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
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

    #ifdef LLGL_DEBUG

    /* Try to create device with debug layer (only supported if Windows 8.1 SDK is installed) */
    if (!CreateDeviceWithFlags(adapter, featureLevels, D3D11_CREATE_DEVICE_DEBUG, hr))
        CreateDeviceWithFlags(adapter, featureLevels, 0, hr);

    #else

    /* Create device without debug layer */
    CreateDeviceWithFlags(adapter, featureLevels, 0, hr);

    #endif

    DXThrowIfFailed(hr, "failed to create D3D11 device");

    /* Try to get an extended D3D11 device */
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    hr = device_->QueryInterface(IID_PPV_ARGS(&device3_));
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

bool D3D11RenderSystem::CreateDeviceWithFlags(IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels, UINT flags, HRESULT& hr)
{
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
            return true;
    }

    return false;
}

void D3D11RenderSystem::CreateStateManagerAndCommandQueue()
{
    stateMngr_ = MakeUnique<D3D11StateManager>(context_);
    commandQueue_ = MakeUnique<D3D11CommandQueue>(context_);
}

void D3D11RenderSystem::QueryRendererInfo()
{
    RendererInfo info;

    /* Initialize Direct3D version string */
    const auto minorVersion = GetMinorVersion();
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
            info.rendererName = "Direct3D " + DXFeatureLevelToVersion(GetFeatureLevel());
            break;
    }

    /* Initialize HLSL version string */
    info.shadingLanguageName = "HLSL " + DXFeatureLevelToShaderModel(GetFeatureLevel());

    /* Initialize video adapter strings */
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
    RenderingCapabilities caps;
    {
        /* Query common DX rendering capabilities */
        DXGetRenderingCaps(caps, GetFeatureLevel());

        /* Set extended attributes */
        const auto minorVersion = GetMinorVersion();

        caps.features.hasCommandBufferExt           = true;
        caps.features.hasConservativeRasterization  = (minorVersion >= 3);

        caps.limits.maxNumViewports                 = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        caps.limits.maxViewportSize[0]              = D3D11_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxViewportSize[1]              = D3D11_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxBufferSize                   = std::numeric_limits<UINT>::max();
        caps.limits.maxConstantBufferSize           = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    }
    SetRenderingCaps(caps);
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


} // /namespace LLGL



// ================================================================================
