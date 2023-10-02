/*
 * D3D11RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
#include "../../Core/CoreUtils.h"
#include "../../Core/StringUtils.h"
#include "../../Core/Assertion.h"
#include <sstream>
#include <iomanip>
#include <limits.h>

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"
#include "Buffer/D3D11BufferWithRV.h"

#include "RenderState/D3D11GraphicsPSO.h"
#include "RenderState/D3D11GraphicsPSO1.h"
#include "RenderState/D3D11GraphicsPSO3.h"
#include "RenderState/D3D11ComputePSO.h"

#include <LLGL/Backend/Direct3D11/NativeHandle.h>


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

D3D11RenderSystem::D3D11RenderSystem(const RenderSystemDescriptor& renderSystemDesc)
{
    const bool debugDevice = ((renderSystemDesc.flags & RenderSystemFlags::DebugDevice) != 0);

    /* Create DXGU factory, query video adapters, and create D3D11 device */
    CreateFactory();
    QueryVideoAdapters();
    CreateDevice(nullptr, debugDevice);

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

/* ----- Swap-chain ----- */

SwapChain* D3D11RenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<D3D11SwapChain>(factory_.Get(), device_, swapChainDesc, surface);
}

void D3D11RenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* D3D11RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D11RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    if ((commandBufferDesc.flags & (CommandBufferFlags::ImmediateSubmit)) != 0)
    {
        /* Create command buffer with immediate context */
        return commandBuffers_.emplace<D3D11CommandBuffer>(device_.Get(), context_, stateMngr_, commandBufferDesc);
    }
    else
    {
        /* Create deferred D3D11 device context */
        ComPtr<ID3D11DeviceContext> deferredContext;
        HRESULT hr = device_->CreateDeferredContext(0, deferredContext.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11DeviceContext", "for deferred command buffer");

        /* Create state manager dedicated to deferred context */
        auto deferredStateMngr = std::make_shared<D3D11StateManager>(device_.Get(), deferredContext);

        /* Create command buffer with deferred context and dedicated state manager */
        return commandBuffers_.emplace<D3D11CommandBuffer>(device_.Get(), deferredContext, deferredStateMngr, commandBufferDesc);
    }
}

void D3D11RenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

Buffer* D3D11RenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, UINT_MAX);
    if (DXBindFlagsNeedBufferWithRV(bufferDesc.bindFlags))
        return buffers_.emplace<D3D11BufferWithRV>(device_.Get(), bufferDesc, initialData);
    else
        return buffers_.emplace<D3D11Buffer>(device_.Get(), bufferDesc, initialData);
}

BufferArray* D3D11RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<D3D11BufferArray>(numBuffers, bufferArray);
}

void D3D11RenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void D3D11RenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void D3D11RenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.WriteSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void D3D11RenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.ReadSubresource(context_.Get(), data, static_cast<UINT>(dataSize), static_cast<UINT>(offset));
}

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    return bufferD3D.Map(context_.Get(), access, 0, bufferD3D.GetSize());
}

void* D3D11RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    return bufferD3D.Map(context_.Get(), access, static_cast<UINT>(offset), static_cast<UINT>(length));
}

void D3D11RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D = LLGL_CAST(D3D11Buffer&, buffer);
    bufferD3D.Unmap(context_.Get());
}

/* ----- Textures ----- */

Texture* D3D11RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    /* Create texture object */
    auto* textureD3D = textures_.emplace<D3D11Texture>(device_.Get(), textureDesc);

    /* Initialize texture data with or without initial image data */
    InitializeGpuTexture(*textureD3D, textureDesc, initialImage);

    /* Generate MIP-maps if enabled */
    if (initialImage != nullptr && MustGenerateMipsOnCreate(textureDesc))
        D3D11MipGenerator::Get().GenerateMips(context_.Get(), *textureD3D);

    return textureD3D;
}

void D3D11RenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void D3D11RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                textureRegion.subresource.numArrayLayers,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.extent.width
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                textureRegion.subresource.numArrayLayers,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.extent.width,
                    textureRegion.extent.height
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            /* Multi-sampled textures cannot be written by CPU */
            break;

        case TextureType::Texture3D:
            textureD3D.UpdateSubresource(
                context_.Get(),
                textureRegion.subresource.baseMipLevel,
                0,
                1,
                D3D11Types::MakeD3D11Box(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.offset.z,
                    textureRegion.extent.width,
                    textureRegion.extent.height,
                    textureRegion.extent.depth
                ),
                srcImageView,
                &(GetMutableReport())
            );
            break;
    }
}

void D3D11RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    if (dstImageView.data == nullptr)
        return /*E_INVALIDARG*/;

    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);

    /* Map subresource for reading */
    const Format            format              = textureD3D.GetFormat();
    const Extent3D          extent              = CalcTextureExtent(textureD3D.GetType(), textureRegion.extent);
    const std::uint32_t     numTexelsPerLayer   = extent.width * extent.height * extent.depth;
    const std::uint32_t     numTexelsTotal      = numTexelsPerLayer * textureRegion.subresource.numArrayLayers;
    const std::size_t       requiredImageSize   = GetMemoryFootprint(dstImageView.format, dstImageView.dataType, numTexelsTotal);

    if (dstImageView.dataSize < requiredImageSize)
        return /*E_BOUNDS*/;

    /* Create a copy of the hardware texture with CPU read access */
    D3D11NativeTexture texCopy;
    textureD3D.CreateSubresourceCopyWithCPUAccess(device_.Get(), context_.Get(), texCopy, D3D11_CPU_ACCESS_READ, textureRegion);

    MutableImageView        intermediateDstView = dstImageView;
    const FormatAttributes& formatAttribs       = GetFormatAttribs(format);

    for_range(arrayLayer, textureRegion.subresource.numArrayLayers)
    {
        const UINT subresource = D3D11CalcSubresource(0, arrayLayer, 1);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        HRESULT hr = context_->Map(texCopy.resource.Get(), subresource, D3D11_MAP_READ, 0, &mappedSubresource);
        DXThrowIfFailed(hr, "failed to map D3D11 texture copy resource");

        /* Copy host visible resource to CPU accessible resource */
        const ImageView intermediateSrcView{ formatAttribs.format, formatAttribs.dataType, mappedSubresource.pData, mappedSubresource.DepthPitch };
        const std::size_t bytesWritten = RenderSystem::CopyTextureImageData(intermediateDstView, intermediateSrcView, numTexelsPerLayer, extent.width, mappedSubresource.RowPitch);

        /* Unmap resource */
        context_->Unmap(texCopy.resource.Get(), subresource);

        /* Move destination image pointer to next layer */
        intermediateDstView.data = reinterpret_cast<char*>(intermediateDstView.data) + bytesWritten;
    }
}

/* ----- Sampler States ---- */

Sampler* D3D11RenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<D3D11Sampler>(device_.Get(), samplerDesc);
}

void D3D11RenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* D3D11RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<D3D11ResourceHeap>(resourceHeapDesc, initialResourceViews);
}

void D3D11RenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t D3D11RenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapD3D = LLGL_CAST(D3D11ResourceHeap&, resourceHeap);
    return resourceHeapD3D.WriteResourceViews(firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* D3D11RenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<D3D11RenderPass>(renderPassDesc);
}

void D3D11RenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D11RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<D3D11RenderTarget>(device_.Get(), renderTargetDesc);
}

void D3D11RenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* D3D11RenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    RenderSystem::AssertCreateShader(shaderDesc);
    return shaders_.emplace<D3D11Shader>(device_.Get(), shaderDesc);
}

void D3D11RenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D11RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<D3D11PipelineLayout>(device_.Get(), pipelineLayoutDesc);
}

void D3D11RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline States ----- */

PipelineState* D3D11RenderSystem::CreatePipelineState(const Blob& /*serializedCache*/)
{
    return nullptr;//TODO
}

PipelineState* D3D11RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, Blob* /*serializedCache*/)
{
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    if (device3_)
    {
        /* Create graphics pipeline for Direct3D 11.3 */
        return pipelineStates_.emplace<D3D11GraphicsPSO3>(device3_.Get(), pipelineStateDesc);
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
    if (device2_)
    {
        /* Create graphics pipeline for Direct3D 11.1 (there is no dedicated class for 11.2) */
        return pipelineStates_.emplace<D3D11GraphicsPSO1>(device2_.Get(), pipelineStateDesc);
    }
    #endif

    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
    if (device1_)
    {
        /* Create graphics pipeline for Direct3D 11.1 */
        return pipelineStates_.emplace<D3D11GraphicsPSO1>(device1_.Get(), pipelineStateDesc);
    }
    #endif

    /* Create graphics pipeline for Direct3D 11.0 */
    return pipelineStates_.emplace<D3D11GraphicsPSO>(device_.Get(), pipelineStateDesc);
}

PipelineState* D3D11RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, Blob* /*serializedCache*/)
{
    return pipelineStates_.emplace<D3D11ComputePSO>(pipelineStateDesc);
}

void D3D11RenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D11RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<D3D11QueryHeap>(device_.Get(), queryHeapDesc);
}

void D3D11RenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* D3D11RenderSystem::CreateFence()
{
    return fences_.emplace<D3D11Fence>(device_.Get());
}

void D3D11RenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool D3D11RenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(Direct3D11::RenderSystemNativeHandle))
    {
        auto* nativeHandleD3D = reinterpret_cast<Direct3D11::RenderSystemNativeHandle*>(nativeHandle);
        nativeHandleD3D->device = device_.Get();
        nativeHandleD3D->device->AddRef();
        return true;
    }
    return false;
}


/*
 * ======= Internal: =======
 */

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

    for_range(i, numFormats)
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
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory_));
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

void D3D11RenderSystem::CreateDevice(IDXGIAdapter* adapter, bool debugDevice)
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

    if (debugDevice)
    {
        /* Try to create device with debug layer (only supported if Windows 8.1 SDK is installed) */
        if (!CreateDeviceWithFlags(adapter, featureLevels, D3D11_CREATE_DEVICE_DEBUG, hr))
            CreateDeviceWithFlags(adapter, featureLevels, 0, hr);
    }
    else
    {
        /* Create device without debug layer */
        CreateDeviceWithFlags(adapter, featureLevels, 0, hr);
    }

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

        caps.features.hasConservativeRasterization  = (minorVersion >= 3);

        caps.limits.maxViewports                    = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        caps.limits.maxViewportSize[0]              = D3D11_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxViewportSize[1]              = D3D11_VIEWPORT_BOUNDS_MAX;
        caps.limits.maxBufferSize                   = UINT_MAX;
        caps.limits.maxConstantBufferSize           = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;

        /* Determine maximum number of samples for various formats */
        caps.limits.maxColorBufferSamples           = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM).Count;
        caps.limits.maxDepthBufferSamples           = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_D32_FLOAT).Count;
        caps.limits.maxStencilBufferSamples         = FindSuitableSampleDesc(device_.Get(), DXGI_FORMAT_D32_FLOAT_S8X24_UINT).Count;
        caps.limits.maxNoAttachmentSamples          = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
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

static void InitializeD3DDepthStencilTextureWithDSV(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const ClearValue&       clearValue)
{
    /* Create intermediate depth-stencil view for texture */
    ComPtr<ID3D11DepthStencilView> dsv;
    D3D11RenderTarget::CreateSubresourceDSV(
        device,
        textureD3D.GetNative().resource.Get(),
        dsv.ReleaseAndGetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetDXFormat(),
        0,
        0,
        textureD3D.GetNumArrayLayers()
    );

    /* Clear view with depth-stencil values */
    context->ClearDepthStencilView(
        dsv.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        clearValue.depth,
        static_cast<UINT8>(clearValue.stencil)
    );
}

static void InitializeD3DColorTextureWithRTV(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const ClearValue&       clearValue)
{
    /* Create intermediate depth-stencil view for texture */
    ComPtr<ID3D11RenderTargetView> rtv;
    D3D11RenderTarget::CreateSubresourceRTV(
        device,
        textureD3D.GetNative().resource.Get(),
        rtv.ReleaseAndGetAddressOf(),
        textureD3D.GetType(),
        textureD3D.GetBaseDXFormat(),
        0,
        0,
        textureD3D.GetNumArrayLayers()
    );

    /* Clear view with depth-stencil values */
    context->ClearRenderTargetView(rtv.Get(), clearValue.color);
}

static void InitializeD3DColorTextureWithUploadBuffer(
    ID3D11DeviceContext*    context,
    D3D11Texture&           textureD3D,
    const Extent3D&         extent,
    const ClearValue&       clearValue)
{
    /* Find suitable image format for texture hardware format */
    ImageView imageViewDefault;

    const auto& formatDesc = GetFormatAttribs(textureD3D.GetBaseFormat());
    if (formatDesc.bitSize > 0)
    {
        /* Copy image format and data type from descriptor */
        imageViewDefault.format     = formatDesc.format;
        imageViewDefault.dataType   = formatDesc.dataType;

        /* Generate default image buffer */
        const std::size_t   imageSize   = extent.width * extent.height * extent.depth;
        DynamicByteArray    imageBuffer = GenerateImageBuffer(imageViewDefault.format, imageViewDefault.dataType, imageSize, clearValue.color);

        /* Update only the first MIP-map level for each array slice */
        imageViewDefault.data       = imageBuffer.data();
        imageViewDefault.dataSize   = GetMemoryFootprint(imageViewDefault.format, imageViewDefault.dataType, imageSize);

        for_range(layer, textureD3D.GetNumArrayLayers())
        {
            HRESULT hr = textureD3D.UpdateSubresource(
                /*context:*/        context,
                /*mipLevel:*/       0,
                /*baseArrayLayer:*/ layer,
                /*numArrayLayers:*/ 1,
                /*dstBox:*/         D3D11Types::MakeD3D11Box(0, 0, 0, extent.width, extent.height, extent.depth),
                /*imageView:*/      imageViewDefault
            );
            DXThrowIfFailed(hr, "in 'InitializeD3DColorTextureWithUploadBuffer': LLGL::D3D11Texture::UpdateSubresource failed");
        }
    }
}

void D3D11RenderSystem::InitializeGpuTexture(
    D3D11Texture&               textureD3D,
    const TextureDescriptor&    textureDesc,
    const ImageView*            initialImage)
{
    if (initialImage != nullptr)
    {
        /* Initialize texture with specified image descriptor */
        textureD3D.UpdateSubresource(
            /*context:*/        context_.Get(),
            /*mipLevel:*/       0,
            /*baseArrayLayer:*/ 0,
            /*numArrayLayers:*/ textureDesc.arrayLayers,
            /*dstBox:*/         D3D11Types::MakeD3D11Box(0, 0, 0, textureDesc.extent.width, textureDesc.extent.height, textureDesc.extent.depth),
            /*imageView:*/      *initialImage,
            /*report:*/         &(GetMutableReport())
        );
    }
    else if ((textureDesc.miscFlags & MiscFlags::NoInitialData) == 0)
    {
        /* Initialize texture with clear value using hardware accelerated clear function or CPU upload buffer */
        if (IsDepthOrStencilFormat(textureDesc.format))
        {
            const bool hasDSVBinding = ((textureDesc.bindFlags & BindFlags::DepthStencilAttachment) != 0);
            if (hasDSVBinding)
            {
                InitializeD3DDepthStencilTextureWithDSV(
                    device_.Get(),
                    context_.Get(),
                    textureD3D,
                    textureDesc.clearValue
                );
            }
            else
            {
                //LLGL_TRAP_NOT_IMPLEMENTED("initialize depth-stencil texture without DepthStencilAttachment binding"); //TODO
            }
        }
        else
        {
            const bool hasRTVBinding = ((textureDesc.bindFlags & BindFlags::ColorAttachment) != 0);
            if (hasRTVBinding)
            {
                InitializeD3DColorTextureWithRTV(
                    device_.Get(),
                    context_.Get(),
                    textureD3D,
                    textureDesc.clearValue
                );
            }
            else
            {
                InitializeD3DColorTextureWithUploadBuffer(
                    context_.Get(),
                    textureD3D,
                    textureDesc.extent,
                    textureDesc.clearValue
                );
            }
        }
    }
}


} // /namespace LLGL



// ================================================================================
