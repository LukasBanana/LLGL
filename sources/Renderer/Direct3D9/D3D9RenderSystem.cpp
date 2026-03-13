/*
 * D3D9RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9RenderSystem.h"
#include "D3D9Core.h"
#include "D3D9Types.h"

#include "Buffer/D3D9VertexBuffer.h"
#include "Buffer/D3D9IndexBuffer.h"
#include "Buffer/D3D9EmulatedConstantBuffer.h"

#include "Shader/D3D9VertexShader.h"
#include "Shader/D3D9PixelShader.h"

#include "RenderState/D3D9StatePool.h"
#include "RenderState/D3D9FixedFunctionPSO.h"
#include "RenderState/D3D9ProgrammablePSO.h"

#include "../../Core/CoreUtils.h"
#include "../../Core/Vendor.h"
#include <LLGL/Window.h>
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/TypeNames.h>
#include <limits.h>


namespace LLGL
{


static void InitD3D9RendererShadingLanguages(std::vector<ShadingLanguage>& shadingLanguages)
{
    shadingLanguages =
    {
        ShadingLanguage::HLSL,
        ShadingLanguage::HLSL_2_0,
        ShadingLanguage::HLSL_2_0a,
        ShadingLanguage::HLSL_2_0b,
        ShadingLanguage::HLSL_3_0,
    };
}

static void InitD3D9RendererTextureFormats(std::vector<Format>& textureFormats)
{
    constexpr int firstFormatIndex  = static_cast<int>(Format::A8UNorm);
    constexpr int lastFormatIndex   = static_cast<int>(Format::BC5SNorm);
    constexpr int numFormats        = lastFormatIndex - firstFormatIndex + 1;
    textureFormats.reserve(static_cast<std::size_t>(numFormats));
    for_range(i, numFormats)
        textureFormats.push_back(static_cast<Format>(firstFormatIndex + i));
}

static void InitD3D9RendererFeatures(RenderingFeatures& features)
{
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = false;
    features.hasCubeArrayTextures           = false;
    features.hasMultiSampleTextures         = true;
    features.hasTextureViews                = false;
    features.hasTextureViewSwizzle          = false;
    features.hasTextureViewFormatSwizzle    = false;
    features.hasBufferViews                 = false;
    features.hasConstantBuffers             = false;
    features.hasStorageBuffers              = false;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = false;
    features.hasTessellatorStage            = false;
    features.hasComputeShaders              = false;
    features.hasInstancing                  = false;
    features.hasOffsetInstancing            = false;
    features.hasIndirectDrawing             = false;
    features.hasViewportArrays              = false;
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = false;
    features.hasPipelineStatistics          = false;
    features.hasRenderCondition             = false;
}

static bool IsMultiSampleTypeSupported(IDirect3D9* direct3d, D3DFORMAT format, UINT samples)
{
    D3DMULTISAMPLE_TYPE d3dType = D3D9Types::ToD3DMultiSampleType(samples);
    DWORD qualityLevels = 0;
    HRESULT hr = direct3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, TRUE, d3dType, &qualityLevels);
    return SUCCEEDED(hr);
}

static UINT FindSuitableMultiSampleFormat(IDirect3D9* direct3d, D3DFORMAT format)
{
    for (UINT samples = 16; samples >= 2; --samples)
    {
        if (IsMultiSampleTypeSupported(direct3d, format, samples))
            return samples;
    }
    return 1;
}

static void InitD3D9RendererLimits(IDirect3D9* direct3d, RenderingLimits& limits, const D3DCAPS9& inCaps)
{
    constexpr DWORD shaderConstantRegisterSize = 16;

    limits.maxTextureArrayLayers            = 0;
    limits.maxColorAttachments              = inCaps.NumSimultaneousRTs;
    limits.maxPatchVertices                 = 0;
    limits.max1DTextureSize                 = inCaps.MaxTextureWidth;
    limits.max2DTextureSize                 = inCaps.MaxTextureHeight;
    limits.max3DTextureSize                 = inCaps.MaxVolumeExtent;
    limits.maxCubeTextureSize               = inCaps.MaxTextureHeight;
    limits.maxAnisotropy                    = inCaps.MaxAnisotropy;
    limits.maxComputeShaderWorkGroups[0]    = 0;
    limits.maxComputeShaderWorkGroups[1]    = 0;
    limits.maxComputeShaderWorkGroups[2]    = 0;
    limits.maxComputeShaderWorkGroupSize[0] = 0;
    limits.maxComputeShaderWorkGroupSize[1] = 0;
    limits.maxComputeShaderWorkGroupSize[2] = 0;
    limits.maxViewports                     = 1;
    limits.maxViewportSize[0]               = static_cast<std::uint32_t>(inCaps.GuardBandRight - inCaps.GuardBandLeft);
    limits.maxViewportSize[1]               = static_cast<std::uint32_t>(inCaps.GuardBandBottom - inCaps.GuardBandTop);
    limits.maxBufferSize                    = std::numeric_limits<UINT>::max();
    limits.maxConstantBufferSize            = inCaps.MaxVertexShaderConst * shaderConstantRegisterSize;
    limits.maxStreamOutputs                 = 0;
    limits.maxTessFactor                    = 0;
    limits.minConstantBufferAlignment       = 4;
    limits.minSampledBufferAlignment        = 0;
    limits.minStorageBufferAlignment        = 0;
    limits.maxColorBufferSamples            = FindSuitableMultiSampleFormat(direct3d, D3DFMT_A8R8G8B8);
    limits.maxDepthBufferSamples            = FindSuitableMultiSampleFormat(direct3d, D3DFMT_D24S8);
    limits.maxStencilBufferSamples          = 1;
    limits.maxNoAttachmentSamples           = 1;
}

static void GetD3D9RenderingCaps(IDirect3D9* direct3d, RenderingCapabilities& outCaps, const D3DCAPS9& inCaps)
{
    InitD3D9RendererShadingLanguages(outCaps.shadingLanguages);
    InitD3D9RendererTextureFormats(outCaps.textureFormats);
    InitD3D9RendererFeatures(outCaps.features);
    InitD3D9RendererLimits(direct3d, outCaps.limits, inCaps);
}

static void GetD3D9RendererInfo(RendererInfo& outInfo, const D3DADAPTER_IDENTIFIER9& inAdapterIdent)
{
    outInfo.rendererName        = "Direct3D 9.0c";
    outInfo.deviceName          = inAdapterIdent.Description;
    outInfo.vendorName          = GetVendorName(GetVendorByID(static_cast<std::uint16_t>(inAdapterIdent.VendorId)));
    outInfo.shadingLanguageName = "HLSL 3.0";
}

D3D9RenderSystem::D3D9RenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    desc_         { renderSystemDesc               },
    commandQueue_ { MakeUnique<D3D9CommandQueue>() }
{
    (void)CreateDevice();
}

D3D9RenderSystem::~D3D9RenderSystem()
{
    /* Clear all render state containers first, the rest will be deleted automatically */
    D3D9StatePool::Get().Clear();
}

/* ----- Swap-chain ----- */

SwapChain* D3D9RenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<D3D9SwapChain>(device_.Get(), swapChainDesc, surface, GetRendererInfo());
}

void D3D9RenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* D3D9RenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* D3D9RenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    return commandBuffers_.emplace<D3D9CommandBuffer>(stateMngr_.get(), commandBufferDesc);
}

void D3D9RenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

//private
D3D9Buffer* D3D9RenderSystem::CreateD3D9Buffer(const BufferDescriptor& bufferDesc)
{
    constexpr long supportedBindFlags = (BindFlags::VertexBuffer | BindFlags::IndexBuffer | BindFlags::ConstantBuffer);
    const long bindFlags = bufferDesc.bindFlags & supportedBindFlags;

    if (bindFlags == BindFlags::VertexBuffer)
        return buffers_.emplace<D3D9VertexBuffer>(device_.Get(), bufferDesc);
    if (bindFlags == BindFlags::IndexBuffer)
        return buffers_.emplace<D3D9IndexBuffer>(device_.Get(), bufferDesc);
    if (bindFlags == BindFlags::ConstantBuffer)
        return buffers_.emplace<D3D9EmulatedConstantBuffer>(bufferDesc);

    return nullptr;
}

Buffer* D3D9RenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, GetRenderingCaps().limits.maxBufferSize);

    if (D3D9Buffer* bufferD3D = CreateD3D9Buffer(bufferDesc))
    {
        if (initialData != nullptr)
        {
            HRESULT hr = bufferD3D->Write(0, initialData, static_cast<UINT>(bufferDesc.size));
            D3DThrowIfFailed(hr, "failed to write D3D9 buffer with initial data");
        }
        return bufferD3D;
    }

    LLGL_TRAP("Invalid binding flags for D3D9 backend: 0x%08X", bufferDesc.bindFlags);
    return nullptr;
}

BufferArray* D3D9RenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<D3D9BufferArray>(numBuffers, bufferArray);
}

void D3D9RenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void D3D9RenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void D3D9RenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferD3D9 = LLGL_CAST(D3D9Buffer&, buffer);
    //bufferD3D9.Write(offset, data, dataSize);
}

void D3D9RenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferD3D9 = LLGL_CAST(D3D9Buffer&, buffer);
    //bufferD3D9.Read(offset, data, dataSize);
}

void* D3D9RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferD3D9 = LLGL_CAST(D3D9Buffer&, buffer);
    //return bufferD3D9.Map(access, 0, bufferD3D9.desc.size);
    return nullptr; //TODO
}

void* D3D9RenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferD3D9 = LLGL_CAST(D3D9Buffer&, buffer);
    //return bufferD3D9.Map(access, offset, length);
    return nullptr; //TODO
}

void D3D9RenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferD3D9 = LLGL_CAST(D3D9Buffer&, buffer);
    //bufferD3D9.Unmap();
}

/* ----- Textures ----- */

Texture* D3D9RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    return textures_.emplace<D3D9Texture>(device_.Get(), textureDesc, initialImage);
}
    
void D3D9RenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void D3D9RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageDesc)
{
    auto& textureD3D9 = LLGL_CAST(D3D9Texture&, texture);
    textureD3D9.Write(textureRegion, srcImageDesc);
}

void D3D9RenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    auto& textureD3D9 = LLGL_CAST(D3D9Texture&, texture);
    textureD3D9.Read(textureRegion, dstImageView);
}

/* ----- Sampler States ---- */

Sampler* D3D9RenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<D3D9EmulatedSampler>(samplerDesc);
}

void D3D9RenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

/* ----- Resource Views ----- */

ResourceHeap* D3D9RenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<D3D9ResourceHeap>(resourceHeapDesc, initialResourceViews);
}

void D3D9RenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t D3D9RenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapD3D9 = LLGL_CAST(D3D9ResourceHeap&, resourceHeap);
    return resourceHeapD3D9.WriteResourceViews(firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* D3D9RenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<D3D9RenderPass>(renderPassDesc);
}

void D3D9RenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* D3D9RenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<D3D9RenderTarget>(device_.Get(), renderTargetDesc);
}

void D3D9RenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* D3D9RenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    switch (shaderDesc.type)
    {
        case ShaderType::Vertex:
            return shaders_.emplace<D3D9VertexShader>(device_.Get(), shaderDesc);
        case ShaderType::Fragment:
            return shaders_.emplace<D3D9PixelShader>(device_.Get(), shaderDesc);
        default:
            LLGL_TRAP("%s shader type not supported in D3D9 backend", ToString(shaderDesc.type));
            return nullptr;
    }
}

void D3D9RenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* D3D9RenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<D3D9PipelineLayout>(pipelineLayoutDesc);
}

void D3D9RenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* D3D9RenderSystem::CreatePipelineCache(const Blob& /*initialBlob*/)
{
    return ProxyPipelineCache::CreateInstance(pipelineCacheProxy_);
}

void D3D9RenderSystem::Release(PipelineCache& pipelineCache)
{
    ProxyPipelineCache::ReleaseInstance(pipelineCacheProxy_, pipelineCache);
}

/* ----- Pipeline States ----- */

//TODO: check vertex and fragment shaders whether they are to be interpreted as fixed-function shaders
static bool IsD3DProgrammablePipeline(const GraphicsPipelineDescriptor& desc)
{
    const bool isHasValidPixelShader = (desc.fragmentShader != nullptr || desc.rasterizer.discardEnabled || desc.blend.targets[0].colorMask == 0);
    return isHasValidPixelShader;
}

PipelineState* D3D9RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    if (IsD3DProgrammablePipeline(pipelineStateDesc))
        return pipelineStates_.emplace<D3D9ProgrammablePSO>(pipelineStateDesc);
    else
        return pipelineStates_.emplace<D3D9FixedFunctionPSO>(pipelineStateDesc);
}

PipelineState* D3D9RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& /*pipelineStateDesc*/, PipelineCache* /*pipelineCache*/)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("compute PSOs");
    return nullptr;
}

PipelineState* D3D9RenderSystem::CreatePipelineState(const MeshPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("mesh PSOs");
    return nullptr;
}

void D3D9RenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* D3D9RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<D3D9QueryHeap>(queryHeapDesc);
}

void D3D9RenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* D3D9RenderSystem::CreateFence()
{
    return fences_.emplace<D3D9Fence>();
}

void D3D9RenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool D3D9RenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return (nativeHandle == nullptr || nativeHandleSize == 0); // dummy
}


/*
 * ======= Private: =======
 */

bool D3D9RenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
    {
        D3DADAPTER_IDENTIFIER9 adapterIdent;
        if (FAILED(direct3d_->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterIdent)))
            return false;
        GetD3D9RendererInfo(*outInfo, adapterIdent);
    }
    if (outCaps != nullptr)
        GetD3D9RenderingCaps(direct3d_.Get(), *outCaps, caps_);
    return true;
}

void D3D9RenderSystem::CreateDevice()
{
    /* Create primary D3D interface */
    direct3d_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (direct3d_.Get() == nullptr)
        LLGL_TRAP("Failed to create IDirect3D9 instance");

    /* Create D3D device with an initial placeholder swap-chain */
    HWND focusWndHandle = CreateFocusWindow();

    D3DPRESENT_PARAMETERS presentParams = {};
    {
        presentParams.BackBufferWidth               = 1;
        presentParams.BackBufferHeight              = 1;
        presentParams.BackBufferFormat              = D3DFMT_UNKNOWN;
        presentParams.BackBufferCount               = 1;

        presentParams.MultiSampleType               = D3DMULTISAMPLE_NONE;
        presentParams.MultiSampleQuality            = 0;

        presentParams.SwapEffect                    = D3DSWAPEFFECT_DISCARD;
        presentParams.hDeviceWindow                 = focusWndHandle;
        presentParams.Windowed                      = TRUE;
        presentParams.EnableAutoDepthStencil        = FALSE;
        presentParams.AutoDepthStencilFormat        = D3DFMT_UNKNOWN;
        presentParams.Flags                         = 0;

        presentParams.FullScreen_RefreshRateInHz    = 0; // FullScreen_RefreshRateInHz must be zero for Windowed mode
        presentParams.PresentationInterval          = 0;
    }
    HRESULT hr = direct3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, focusWndHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParams, device_.GetAddressOf());
    D3DThrowIfCreateFailed(hr, "IDirect3DDevice9");

    /* Get device capabilities immediately as they will be needed by various interfaces */
    hr = device_->GetDeviceCaps(&caps_);
    D3DThrowIfFailed(hr, "failed to get capabilities of IDirect3DDevice9");

    /* Create state manager for D3D device */
    stateMngr_ = MakeUnique<D3D9StateManager>(device_.Get());
}

HWND D3D9RenderSystem::CreateFocusWindow()
{
    WindowDescriptor windowDesc;
    {
        windowDesc.size = { 256, 256 };
    }
    focusWnd_ = Window::Create(windowDesc);

    NativeHandle nativeHandle = {};
    focusWnd_->GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    return nativeHandle.window;
}


} // /namespace LLGL



// ================================================================================
