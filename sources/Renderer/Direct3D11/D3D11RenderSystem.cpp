/*
 * D3D11RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "D3D11ResourceFlags.h"
#include "Texture/D3D11MipGenerator.h"
#include "Shader/D3D11BuiltinShaderFactory.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../TextureUtils.h"
#include "../../Core/Vendor.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"
#include <sstream>
#include <iomanip>
#include <limits>

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11BufferWithRV.h"

#include "RenderState/D3D11GraphicsPSO.h"
#include "RenderState/D3D11GraphicsPSO1.h"
#include "RenderState/D3D11GraphicsPSO3.h"
#include "RenderState/D3D11ComputePSO.h"


namespace LLGL
{


#if 0 //WIP
/*
Returns true if the D3D runtime supports command lists natively.
Otherwise, they will be emulated by the D3D runtime.
See https://docs.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1#remarks
*/
static bool D3DSupportsDriverCommandLists(ID3D11Device* device, ID3D11DeviceContext* context)
{
    D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };
    HRESULT hr = device->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));
    return (SUCCEEDED(hr) && threadingCaps.DriverCommandLists != FALSE);
}
#endif

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

    /* Initialize MIP-map generator singleton */
    D3D11MipGenerator::Get().InitializeDevice(device_);
    D3D11BuiltinShaderFactory::Get().CreateBuiltinShaders(device_.Get());

    //D3DSupportsDriverCommandLists(device_.Get(), context_.Get());
}

D3D11RenderSystem::~D3D11RenderSystem()
{
    /* Release resource of singletons first */
    D3D11MipGenerator::Get().Clear();
    D3D11BuiltinShaderFactory::Get().Clear();
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

CommandBuffer* D3D11RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    if ((desc.flags & (CommandBufferFlags::DeferredSubmit | CommandBufferFlags::MultiSubmit)) != 0)
    {
        /* Create deferred D3D11 device context */
        ComPtr<ID3D11DeviceContext> deferredContext;
        auto hr = device_->CreateDeferredContext(0, deferredContext.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11DeviceContext", "for deferred command buffer");

        /* Create command buffer with deferred context and dedicated state manager */
        return TakeOwnership(
            commandBuffers_,
            MakeUnique<D3D11CommandBuffer>(device_.Get(), deferredContext, std::make_shared<D3D11StateManager>(device_.Get(), deferredContext), desc)
        );
    }
    else
    {
        /* Create command buffer with immediate context */
        return TakeOwnership(
            commandBuffers_,
            MakeUnique<D3D11CommandBuffer>(device_.Get(), context_, stateMngr_, desc)
        );
    }
}

void D3D11RenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

static std::unique_ptr<D3D11Buffer> MakeD3D11Buffer(ID3D11Device* device, const BufferDescriptor& desc, const void* initialData)
{
    /* Make respective buffer type */
    if (DXBindFlagsNeedBufferWithRV(desc.bindFlags))
        return MakeUnique<D3D11BufferWithRV>(device, desc, initialData);
    else
        return MakeUnique<D3D11Buffer>(device, desc, initialData);
}

Buffer* D3D11RenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    AssertCreateBuffer(desc, static_cast<uint64_t>(std::numeric_limits<UINT>::max()));
    return TakeOwnership(buffers_, MakeD3D11Buffer(device_.Get(), desc, initialData));
}

BufferArray* D3D11RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    auto bindFlags = bufferArray[0]->GetBindFlags();
    return TakeOwnership(bufferArrays_, MakeUnique<D3D11BufferArray>(bindFlags, numBuffers, bufferArray));
}

void D3D11RenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void D3D11RenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void D3D11RenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferD3D = LLGL_CAST(D3D11Buffer&, dstBuffer);
    dstBufferD3D.UpdateSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(dstOffset));
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

Texture* D3D11RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    /* Create texture object and store type */
    auto texture = MakeUnique<D3D11Texture>(textureDesc);

    /* Bulid generic texture */
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            CreateAndInitializeGpuTexture1D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            CreateAndInitializeGpuTexture2D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture3D:
            CreateAndInitializeGpuTexture3D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            CreateAndInitializeGpuTexture2DMS(*texture, textureDesc);
            break;
        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }

    /* Generate MIP-maps if enabled */
    if (imageDesc != nullptr && MustGenerateMipsOnCreate(textureDesc))
        D3D11MipGenerator::Get().GenerateMips(context_.Get(), *texture);

    return TakeOwnership(textures_, std::move(texture));
}

void D3D11RenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void D3D11RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    0,
                    0,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    static_cast<LONG>(textureRegion.subresource.numArrayLayers),
                    1
                ),
                imageDesc
            );
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    0,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    textureRegion.offset.y + static_cast<LONG>(textureRegion.extent.height),
                    static_cast<LONG>(textureRegion.subresource.numArrayLayers)
                ),
                imageDesc
            );
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            break;

        case TextureType::Texture3D:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                0,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.offset.z,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    textureRegion.offset.y + static_cast<LONG>(textureRegion.extent.height),
                    textureRegion.offset.z + static_cast<LONG>(textureRegion.extent.depth)
                ),
                imageDesc
            );
            break;
    }
}

void D3D11RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    LLGL_ASSERT_PTR(imageDesc.data);
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);

    /* Create a copy of the hardware texture with CPU read access */
    D3D11NativeTexture texCopy;
    textureD3D.CreateSubresourceCopyWithCPUAccess(device_.Get(), context_.Get(), texCopy, D3D11_CPU_ACCESS_READ, textureRegion);

    /* Map subresource for reading */
    const UINT subresource = 0;

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    auto hr = context_->Map(texCopy.resource.Get(), subresource, D3D11_MAP_READ, 0, &mappedSubresource);
    DXThrowIfFailed(hr, "failed to map D3D11 texture copy resource");

    /* Copy host visible resource to CPU accessible resource */
    const auto format = textureD3D.GetFormat();
    CopyTextureImageData(imageDesc, textureRegion.extent, format, mappedSubresource.pData, mappedSubresource.RowPitch);

    /* Unmap resource */
    context_->Unmap(texCopy.resource.Get(), subresource);
}

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
    const bool hasDeviceContextD3D11_1 = (GetMinorVersion() >= 1);
    return TakeOwnership(resourceHeaps_, MakeUnique<D3D11ResourceHeap>(desc, hasDeviceContextD3D11_1));
}

void D3D11RenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* D3D11RenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return TakeOwnership(renderPasses_, MakeUnique<D3D11RenderPass>(desc));
}

void D3D11RenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
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

PipelineState* D3D11RenderSystem::CreatePipelineState(const Blob& /*serializedCache*/)
{
    return nullptr;//TODO
}

PipelineState* D3D11RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    if (device3_)
    {
        /* Create graphics pipeline for Direct3D 11.3 */
        return TakeOwnership(pipelineStates_, MakeUnique<D3D11GraphicsPSO3>(device3_.Get(), desc));
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
    if (device2_)
    {
        /* Create graphics pipeline for Direct3D 11.1 (there is no dedicated class for 11.2) */
        return TakeOwnership(pipelineStates_, MakeUnique<D3D11GraphicsPSO1>(device2_.Get(), desc));
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (device1_)
    {
        /* Create graphics pipeline for Direct3D 11.1 */
        return TakeOwnership(pipelineStates_, MakeUnique<D3D11GraphicsPSO1>(device1_.Get(), desc));
    }
    #endif

    /* Create graphics pipeline for Direct3D 11.0 */
    return TakeOwnership(pipelineStates_, MakeUnique<D3D11GraphicsPSO>(device_.Get(), desc));
}

PipelineState* D3D11RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return TakeOwnership(pipelineStates_, MakeUnique<D3D11ComputePSO>(desc));
}

void D3D11RenderSystem::Release(PipelineState& pipelineState)
{
    RemoveFromUniqueSet(pipelineStates_, &pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D11RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<D3D11QueryHeap>(device_.Get(), desc));
}

void D3D11RenderSystem::Release(QueryHeap& queryHeap)
{
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* D3D11RenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<D3D11Fence>(device_.Get()));
}

void D3D11RenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
}

/* ----- Internal functions ----- */

DXGI_SAMPLE_DESC D3D11RenderSystem::FindSuitableSampleDesc(ID3D11Device* device, DXGI_FORMAT format, UINT maxSampleCount)
{
    for (; maxSampleCount > 1; --maxSampleCount)
    {
        UINT numQualityLevels = 0;
        if (device->CheckMultisampleQualityLevels(format, maxSampleCount, &numQualityLevels) == S_OK)
        {
            if (numQualityLevels > 0)
                return { maxSampleCount, numQualityLevels - 1 };
        }
    }
    return { 1u, 0u };
}

DXGI_SAMPLE_DESC D3D11RenderSystem::FindSuitableSampleDesc(ID3D11Device* device, std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount)
{
    DXGI_SAMPLE_DESC sampleDesc = { maxSampleCount, 0 };

    for (std::size_t i = 0; i < numFormats; ++i)
    {
        if (formats[i] != DXGI_FORMAT_UNKNOWN)
            sampleDesc = D3D11RenderSystem::FindSuitableSampleDesc(device, formats[i], sampleDesc.Count);
    }

    return sampleDesc;
}


/*
 * ======= Private: =======
 */

void D3D11RenderSystem::CreateFactory()
{
    /* Create DXGI factory */
    auto hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
    DXThrowIfCreateFailed(hr, "IDXGIFactory");
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
    /* Find list of feature levels to select from, and statically determine maximal feature level */
    auto featureLevels = DXGetFeatureLevels(
        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        D3D_FEATURE_LEVEL_11_1
        #else
        D3D_FEATURE_LEVEL_11_0
        #endif
    );

    HRESULT hr = 0;

    #ifdef LLGL_DEBUG

    /* Try to create device with debug layer (only supported if Windows 8.1 SDK is installed) */
    if (!CreateDeviceWithFlags(adapter, featureLevels, D3D11_CREATE_DEVICE_DEBUG, hr))
        CreateDeviceWithFlags(adapter, featureLevels, 0, hr);

    #else

    /* Create device without debug layer */
    CreateDeviceWithFlags(adapter, featureLevels, 0, hr);

    #endif

    DXThrowIfCreateFailed(hr, "ID3D11Device");

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
    stateMngr_ = std::make_shared<D3D11StateManager>(device_.Get(), context_);
    commandQueue_ = MakeUnique<D3D11CommandQueue>(device_.Get(), context_);
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
            info.rendererName = "Direct3D " + std::string(DXFeatureLevelToVersion(GetFeatureLevel()));
            break;
    }

    /* Initialize HLSL version string */
    info.shadingLanguageName = "HLSL " + std::string(DXFeatureLevelToShaderModel(GetFeatureLevel()));

    /* Initialize video adapter strings */
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

void D3D11RenderSystem::QueryRenderingCaps()
{
    RenderingCapabilities caps;
    {
        /* Query common DX rendering capabilities */
        DXGetRenderingCaps(caps, GetFeatureLevel());

        /* Set extended attributes */
        const auto minorVersion = GetMinorVersion();

        caps.features.hasDirectResourceBinding      = true;
        caps.features.hasConservativeRasterization  = (minorVersion >= 3);

        caps.limits.maxViewports                    = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
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

void D3D11RenderSystem::CreateAndInitializeGpuTexture1D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture1D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc, imageDesc);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture2D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture2D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc, imageDesc);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture3D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture3D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc, imageDesc);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& desc)
{
    /* Create native texture */
    textureD3D.CreateTexture2D(device_.Get(), desc);
}

void D3D11RenderSystem::UpdateGenericTexture(
    Texture&                    texture,
    std::uint32_t               mipLevel,
    std::uint32_t               arrayLayer,
    const D3D11_BOX&            region,
    const SrcImageDescriptor&   imageDesc)
{
    /* Get D3D texture and update subresource */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.UpdateSubresource(
        context_.Get(),
        mipLevel,
        arrayLayer,
        region,
        imageDesc,
        GetConfiguration().threadCount
    );
}

void D3D11RenderSystem::InitializeGpuTexture(
    D3D11Texture&               textureD3D,
    const TextureDescriptor&    textureDesc,
    const SrcImageDescriptor*   imageDesc)
{
    if (imageDesc)
    {
        /* Initialize texture with specified image descriptor */
        InitializeGpuTextureWithImage(
            textureD3D,
            textureDesc.format,
            textureDesc.extent,
            textureDesc.arrayLayers,
            *imageDesc
        );
    }
    else if ((textureDesc.miscFlags & MiscFlags::NoInitialData) == 0)
    {
        /* Initialize texture with default image data */
        InitializeGpuTextureWithClearValue(
            textureD3D,
            textureDesc.format,
            textureDesc.extent,
            textureDesc.arrayLayers,
            textureDesc.clearValue
        );
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithImage(
    D3D11Texture&       textureD3D,
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       arrayLayers,
    SrcImageDescriptor  imageDesc)
{
    /* Update only the first MIP-map level for each array layer */
    const auto bytesPerLayer =
    (
        extent.width                        *
        extent.height                       *
        extent.depth                        *
        ImageFormatSize(imageDesc.format)   *
        DataTypeSize(imageDesc.dataType)
    );

    /* Remap image data size for a single array layer to update each subresource individually */
    if (imageDesc.dataSize % arrayLayers != 0)
        throw std::invalid_argument("image data size is not a multiple of the layer count for D3D11 texture");

    imageDesc.dataSize /= arrayLayers;

    for (std::uint32_t layer = 0; layer < arrayLayers; ++layer)
    {
        /* Update subresource of current array layer */
        textureD3D.UpdateSubresource(
            context_.Get(),
            0, // mipLevel
            layer,
            CD3D11_BOX(0, 0, 0, extent.width, extent.height, extent.depth),
            imageDesc,
            GetConfiguration().threadCount
        );

        /* Move to next region of initial data */
        imageDesc.data = (reinterpret_cast<const std::int8_t*>(imageDesc.data) + bytesPerLayer);
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithClearValue(
    D3D11Texture&       textureD3D,
    const Format        format,
    const Extent3D&     extent,
    std::uint32_t       arrayLayers,
    const ClearValue&   clearValue)
{
    if (IsDepthStencilFormat(format))
    {
        //TODO
    }
    else
    {
        /* Find suitable image format for texture hardware format */
        SrcImageDescriptor imageDescDefault;

        const auto& formatDesc = GetFormatAttribs(format);
        if (formatDesc.bitSize > 0)
        {
            /* Copy image format and data type from descriptor */
            imageDescDefault.format     = formatDesc.format;
            imageDescDefault.dataType   = formatDesc.dataType;

            /* Generate default image buffer */
            const auto fillColor = clearValue.color.Cast<double>();
            const auto imageSize = extent.width * extent.height * extent.depth;

            auto imageBuffer = GenerateImageBuffer(imageDescDefault.format, imageDescDefault.dataType, imageSize, fillColor);

            /* Update only the first MIP-map level for each array slice */
            imageDescDefault.data       = imageBuffer.get();
            imageDescDefault.dataSize   = GetMemoryFootprint(imageDescDefault.format, imageDescDefault.dataType, imageSize);

            for (std::uint32_t layer = 0; layer < arrayLayers; ++layer)
            {
                textureD3D.UpdateSubresource(
                    context_.Get(),
                    0,
                    layer,
                    CD3D11_BOX(0, 0, 0, extent.width, extent.height, extent.depth),
                    imageDescDefault,
                    GetConfiguration().threadCount
                );
            }
        }
    }
}


} // /namespace LLGL



// ================================================================================
