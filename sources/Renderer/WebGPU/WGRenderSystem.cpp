/*
 * WGRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderSystem.h"
#include "WGCore.h"
#include "WGTypes.h"
#include "Buffer/WGIndexBuffer.h"
#include "RenderState/WGComputePipeline.h"
#include "RenderState/WGRenderPipeline.h"
#include "Shader/WGShaderModulePool.h"
#include "../../Core/Assertion.h"
#include "../../Platform/Debug.h"
#include <LLGL/Utils/ForRange.h>

#ifdef _WIN32
#include <Windows.h> //TEST
#endif


namespace LLGL
{


WGRenderSystem::WGRenderSystem(const RenderSystemDescriptor& desc) :
    isBreakOnErrorEnabled_ { ((desc.flags & RenderSystemFlags::DebugBreakOnError) != 0) }
{
    CreateWebGpuInstance();
    if (!RequestWebGpuAdapter())
        return;
    if (!RequestWebGpuDevice(desc.flags))
        return;
    CreateCommandQueue();
}

WGRenderSystem::~WGRenderSystem()
{
    WGShaderModulePool::Get().Clear();
}

SwapChain* WGRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<WGSwapChain>(*this, swapChainDesc, surface);
}

void WGRenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

CommandQueue* WGRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

CommandBuffer* WGRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    return commandBuffers_.emplace<WGCommandBuffer>(device_, commandQueue_->GetNative(), commandBufferDesc);
}

void WGRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

//private
WGBuffer* WGRenderSystem::CreateUninitializedBuffer(const BufferDescriptor& bufferDesc)
{
    if ((bufferDesc.bindFlags & BindFlags::IndexBuffer) != 0)
        return buffers_.emplace<WGIndexBuffer>(device_, bufferDesc);
    else
        return buffers_.emplace<WGBuffer>(device_, bufferDesc);
}

Buffer* WGRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    WGBuffer* buffer = CreateUninitializedBuffer(bufferDesc);
    if (initialData != nullptr)
        wgpuQueueWriteBuffer(commandQueue_->GetNative(), buffer->GetNative(), 0, initialData, bufferDesc.size);
    return buffer;
}

BufferArray* WGRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    return bufferArrays_.emplace<WGBufferArray>(numBuffers, bufferArray);
}

void WGRenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void WGRenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void WGRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferWG = LLGL_CAST(WGBuffer&, buffer);
    wgpuQueueWriteBuffer(commandQueue_->GetNative(), bufferWG.GetNative(), offset, data, static_cast<std::size_t>(dataSize));
}

void WGRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void* WGRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void* WGRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::UnmapBuffer(Buffer& buffer)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Texture* WGRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    WGTexture* textureWG = textures_.emplace<WGTexture>(device_, textureDesc);
    if (initialImage != nullptr)
    {
        const std::uint32_t numMipLevels = NumMipLevels(textureDesc);
        for_range(mipLevel, numMipLevels)
        {
            TextureRegion fullMipRegion;
            {
                fullMipRegion.extent                        = LLGL::GetMipExtent(textureDesc, mipLevel);
                fullMipRegion.subresource.numArrayLayers    = textureDesc.arrayLayers;
                fullMipRegion.subresource.baseMipLevel      = mipLevel;
            }
            textureWG->Write(commandQueue_->GetNative(), fullMipRegion, *initialImage);
        }
    }
    return textureWG;
}

void WGRenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void WGRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureWG = LLGL_CAST(WGTexture&, texture);
    textureWG.Write(commandQueue_->GetNative(), textureRegion, srcImageView);
}

void WGRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Sampler* WGRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<WGSampler>(device_, samplerDesc);
}

void WGRenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

ResourceHeap* WGRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(ResourceHeap& resourceHeap)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGRenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

RenderPass* WGRenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(RenderPass& renderPass)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

RenderTarget* WGRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(RenderTarget& renderTarget)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Shader* WGRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    return shaders_.emplace<WGShader>(instance_, device_, shaderDesc);
}

void WGRenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

PipelineLayout* WGRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<WGPipelineLayout>(device_, pipelineLayoutDesc, coreLimits_);
}

void WGRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

PipelineCache* WGRenderSystem::CreatePipelineCache(const Blob& initialBlob)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(PipelineCache& pipelineCache)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

PipelineState* WGRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    return pipelineStates_.emplace<WGRenderPipeline>(device_, pipelineStateDesc);
}

PipelineState* WGRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    return pipelineStates_.emplace<WGComputePipeline>(pipelineStateDesc);
}

PipelineState* WGRenderSystem::CreatePipelineState(const MeshPipelineDescriptor& /*pipelineStateDesc*/, PipelineCache* /*pipelineCache*/)
{
    return nullptr; // not supported
}

void WGRenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

QueryHeap* WGRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(QueryHeap& queryHeap)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Fence* WGRenderSystem::CreateFence()
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(Fence& fence)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

bool WGRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

static void QueryWebGpuRendererInfo(WGPUAdapter adapter, RendererInfo& outInfo)
{
    WGPUAdapterInfo adapterInfo = WGPU_ADAPTER_INFO_INIT;
    wgpuAdapterGetInfo(adapter, &adapterInfo);

    outInfo.rendererName        = "WebGPU";
    outInfo.deviceName          = ToStringView(adapterInfo.device);
    outInfo.vendorName          = ToStringView(adapterInfo.vendor);
    outInfo.shadingLanguageName = "WGSL";
    //outInfo.extensionNames      = { "" };

    /* Append information to the renderer name */
    UTF8String rendererNameInfo;
    auto AppendRendererNameInfo = [&rendererNameInfo](StringView info)
    {
        if (!info.empty())
        {
            if (!rendererNameInfo.empty())
                rendererNameInfo.append(", ");
            rendererNameInfo.append(info);
        }
    };

    AppendRendererNameInfo(ToStringView(adapterInfo.description));
    AppendRendererNameInfo(ToString(adapterInfo.backendType));
    AppendRendererNameInfo(ToString(adapterInfo.adapterType));

    if (!rendererNameInfo.empty())
        outInfo.rendererName.append(" ( " + rendererNameInfo + " )");
}

static std::vector<Format> QueryWebGpuSupportedTextureFormats(WGPUAdapter adapter)
{
    std::vector<Format> outFormats;

    const bool isTexCompressionBCSupported      = (wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionBC  ) == WGPU_TRUE);
    const bool isTexCompressionETC2Supported    = (wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionETC2) == WGPU_TRUE);
    const bool isTexCompressionASTCSupported    = (wgpuAdapterHasFeature(adapter, WGPUFeatureName_TextureCompressionASTC) == WGPU_TRUE);

    constexpr int firstTextureFormat = static_cast<int>(Format::Undefined) + 1;
    constexpr int numTextureFormats = static_cast<int>(Format::ETC2UNorm_sRGB) + 1;
    for_subrange(i, firstTextureFormat, numTextureFormats)
    {
        const Format format = static_cast<Format>(i);
        const WGPUTextureFormat wgpuFormat = WGTypes::ToWGTextureFormatOrDefault(format);

        if ((!isTexCompressionBCSupported   && WGTypes::IsWGTextureFormatBC  (wgpuFormat)) ||
            (!isTexCompressionETC2Supported && WGTypes::IsWGTextureFormatETC2(wgpuFormat)) ||
            (!isTexCompressionASTCSupported && WGTypes::IsWGTextureFormatASTC(wgpuFormat)))
        {
            continue;
        }

        if (wgpuFormat != WGPUTextureFormat_Undefined)
            outFormats.push_back(format);
    }

    return outFormats;
}

static void QueryWebGpuRenderingCaps(WGPUAdapter adapter, RenderingCapabilities& outCaps, WGCoreLimits& outCoreLimits)
{
    WGPULimits adapterLimits = {};
    wgpuAdapterGetLimits(adapter, &adapterLimits);

    outCaps.limits.max1DTextureSize                 = adapterLimits.maxTextureDimension1D;
    outCaps.limits.max2DTextureSize                 = adapterLimits.maxTextureDimension2D;
    outCaps.limits.max3DTextureSize                 = adapterLimits.maxTextureDimension3D;
    outCaps.limits.maxTextureArrayLayers            = adapterLimits.maxTextureArrayLayers;
    //adapterLimits.maxBindGroups;
    //adapterLimits.maxBindGroupsPlusVertexBuffers;
    //adapterLimits.maxBindingsPerBindGroup;
    //adapterLimits.maxDynamicUniformBuffersPerPipelineLayout;
    //adapterLimits.maxDynamicStorageBuffersPerPipelineLayout;
    //adapterLimits.maxSampledTexturesPerShaderStage;
    //adapterLimits.maxSamplersPerShaderStage;
    //adapterLimits.maxStorageBuffersPerShaderStage;
    //adapterLimits.maxStorageTexturesPerShaderStage;
    //adapterLimits.maxUniformBuffersPerShaderStage;
    //adapterLimits.maxUniformBufferBindingSize;
    //adapterLimits.maxStorageBufferBindingSize;
    outCaps.limits.minConstantBufferAlignment       = adapterLimits.minUniformBufferOffsetAlignment;
    outCaps.limits.minStorageBufferAlignment        = adapterLimits.minStorageBufferOffsetAlignment;
    //uint32_t maxVertexBuffers;
    outCaps.limits.maxBufferSize                    = adapterLimits.maxBufferSize;
    //adapterLimits.maxVertexAttributes;
    //adapterLimits.maxVertexBufferArrayStride;
    //adapterLimits.maxInterStageShaderVariables;
    outCaps.limits.maxColorAttachments              = adapterLimits.maxColorAttachments;
    //adapterLimits.maxColorAttachmentBytesPerSample;
    //adapterLimits.maxComputeWorkgroupStorageSize;
    //adapterLimits.maxComputeInvocationsPerWorkgroup;
    outCaps.limits.maxComputeShaderWorkGroupSize[0] = adapterLimits.maxComputeWorkgroupsPerDimension;
    outCaps.limits.maxComputeShaderWorkGroupSize[1] = adapterLimits.maxComputeWorkgroupsPerDimension;
    outCaps.limits.maxComputeShaderWorkGroupSize[2] = adapterLimits.maxComputeWorkgroupsPerDimension;
    outCaps.limits.maxComputeShaderWorkGroups[0]    = adapterLimits.maxComputeWorkgroupSizeX;
    outCaps.limits.maxComputeShaderWorkGroups[1]    = adapterLimits.maxComputeWorkgroupSizeY;
    outCaps.limits.maxComputeShaderWorkGroups[2]    = adapterLimits.maxComputeWorkgroupSizeZ;
    //adapterLimits.maxImmediateSize;

    outCaps.limits.maxViewports         = 1;
    outCaps.limits.maxViewportSize[0]   = 16384;
    outCaps.limits.maxViewportSize[1]   = 16384;

    outCaps.shadingLanguages            = { ShadingLanguage::WGSL/*, ShadingLanguage::SPIRV*/ };
    outCaps.textureFormats              = QueryWebGpuSupportedTextureFormats(adapter);

    /* Store core limits in render system */
    outCoreLimits.maxBindGroups             = adapterLimits.maxBindGroups;
    outCoreLimits.maxBindingsPerBindGroup   = adapterLimits.maxBindingsPerBindGroup;
}

bool WGRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (adapter_)
    {
        if (outInfo != nullptr)
            QueryWebGpuRendererInfo(adapter_, *outInfo);
        if (outCaps != nullptr)
            QueryWebGpuRenderingCaps(adapter_, *outCaps, coreLimits_);
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void WGRenderSystem::CreateWebGpuInstance()
{
    const WGPUInstanceFeatureName requiredFeatures[] =
    {
        WGPUInstanceFeatureName_TimedWaitAny,
    };
    WGPUInstanceDescriptor instanceDesc;
    {
        instanceDesc.nextInChain            = nullptr;
        instanceDesc.requiredFeatureCount   = LLGL_ARRAY_LENGTH(requiredFeatures);
        instanceDesc.requiredFeatures       = requiredFeatures;
        instanceDesc.requiredLimits         = nullptr;
    }
    instance_ = wgpuCreateInstance(&instanceDesc);
}

static void OnWebGpuRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2)
{
    LLGL_ASSERT(status == WGPURequestAdapterStatus_Success);
    *reinterpret_cast<WGPUAdapter*>(userdata1) = adapter;
}

bool WGRenderSystem::RequestWebGpuAdapter()
{
    WGPURequestAdapterOptions options;
    {
        options.nextInChain             = nullptr;
        options.featureLevel            = WGPUFeatureLevel_Undefined;
        options.powerPreference         = WGPUPowerPreference_HighPerformance;
        options.forceFallbackAdapter    = WGPU_FALSE;
        options.backendType             = WGPUBackendType_Undefined;
        options.compatibleSurface       = nullptr;
    }
    WGPURequestAdapterCallbackInfo callbackInfo;
    {
        callbackInfo.nextInChain    = nullptr;
        callbackInfo.mode           = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback       = OnWebGpuRequestAdapter;
        callbackInfo.userdata1      = &adapter_;
        callbackInfo.userdata2      = nullptr;
    }
    WGPUFutureWaitInfo waitInfo;
    {
        waitInfo.future     = wgpuInstanceRequestAdapter(instance_, &options, callbackInfo);
        waitInfo.completed  = WGPU_FALSE;
    }
    const WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance_, 1, &waitInfo, UINT64_MAX);

    if (waitStatus != WGPUWaitStatus_Success)
    {
        GetMutableReport().Errorf("failed to request WebGPU adapter (%s)", ToString(waitStatus));
        return false;
    }

    return true;
}

static void OnWebGpuRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2)
{
    LLGL_ASSERT(status == WGPURequestDeviceStatus_Success);
    *reinterpret_cast<WGPUDevice*>(userdata1) = device;
}

static void OnWebGpuLogging(WGPULoggingType type, WGPUStringView message, void* userdata1, void* userdata2)
{
    const WGPULoggingType logVerbosity = *static_cast<const WGPULoggingType*>(userdata2);
    if (type >= logVerbosity)
    {
        const std::string messageCstr{ message.data, message.length };
        DebugPuts(messageCstr.c_str());
    }

    if (type == WGPULoggingType_Error)
    {
        WGRenderSystem* renderSystemWG = static_cast<WGRenderSystem*>(userdata1);
        if (renderSystemWG->IsBreakOnErrorEnabled())
            DebugBreakOnError();
    }
}

bool WGRenderSystem::RequestWebGpuDevice(long renderSystemFlags)
{
    const bool isDebugDevice = ((renderSystemFlags & RenderSystemFlags::DebugDevice) != 0);

    #ifdef _WIN32
    ::SetEnvironmentVariable(TEXT("DAWN_DEBUG_BREAK_ON_ERROR"), TEXT("1"));
    #endif

    /* Enable Dawn toggles depending on render system flags */
    SmallVector<const char*, 8> enabledToggles;
    if (isDebugDevice)
    {
        enabledToggles =
        {
            "dump_shaders_on_failure",
            "use_user_defined_labels_in_backend",
            "disable_symbol_renaming",
            "emit_hlsl_debug_symbols",
        };
    }
    else
    {
        enabledToggles =
        {
            "skip_validation",
        };
    }

    /* Request WebGPU device and wait for result */
    const WGPUFeatureName requiredFeatures[] =
    {
        WGPUFeatureName_CoreFeaturesAndLimits, //TODO
        #ifndef LLGL_OS_WASM
        WGPUFeatureName_TextureCompressionBC,
        #endif
    };

    WGPUDawnTogglesDescriptor dawnToggleDesc;
    {
        dawnToggleDesc.chain                = WGPUChainedStruct{ nullptr, WGPUSType_DawnTogglesDescriptor };
        dawnToggleDesc.enabledToggleCount   = enabledToggles.size();
        dawnToggleDesc.enabledToggles       = enabledToggles.data();
        dawnToggleDesc.disabledToggleCount  = 0;
        dawnToggleDesc.disabledToggles      = nullptr;
    }
    WGPUDeviceDescriptor deviceDesc;
    {
        deviceDesc.nextInChain                  = &(dawnToggleDesc.chain);
        deviceDesc.label                        = WGPU_STRING_VIEW_INIT;
        deviceDesc.requiredFeatureCount         = LLGL_ARRAY_LENGTH(requiredFeatures);
        deviceDesc.requiredFeatures             = requiredFeatures;
        deviceDesc.requiredLimits               = nullptr;
        deviceDesc.defaultQueue                 = WGPU_QUEUE_DESCRIPTOR_INIT;
        deviceDesc.deviceLostCallbackInfo       = WGPU_DEVICE_LOST_CALLBACK_INFO_INIT; //WGPUDeviceLostCallbackInfo
        deviceDesc.uncapturedErrorCallbackInfo  = WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT; //WGPUUncapturedErrorCallbackInfo
    }
    WGPURequestDeviceCallbackInfo deviceCallbackInfo;
    {
        deviceCallbackInfo.nextInChain  = nullptr;
        deviceCallbackInfo.mode         = WGPUCallbackMode_WaitAnyOnly;
        deviceCallbackInfo.callback     = OnWebGpuRequestDevice;
        deviceCallbackInfo.userdata1    = &device_;
        deviceCallbackInfo.userdata2    = nullptr;
    }
    WGPUFutureWaitInfo waitInfo;
    {
        waitInfo.future     = wgpuAdapterRequestDevice(adapter_, &deviceDesc, deviceCallbackInfo);
        waitInfo.completed  = WGPU_FALSE;
    }
    const WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(instance_, 1, &waitInfo, UINT64_MAX);

    if (waitStatus != WGPUWaitStatus_Success)
    {
        GetMutableReport().Errorf("failed to request WebGPU adapter (%s)", ToString(waitStatus));
        return false;
    }

    /* Enable debug logging handler */
    if (isDebugDevice || isBreakOnErrorEnabled_)
    {
        if (isDebugDevice)
            logVerbosity_ = WGPULoggingType_Warning;

        WGPULoggingCallbackInfo loggingCallbackInfo;
        {
            loggingCallbackInfo.nextInChain = nullptr;
            loggingCallbackInfo.callback    = OnWebGpuLogging;
            loggingCallbackInfo.userdata1   = this;
            loggingCallbackInfo.userdata2   = &logVerbosity_;
        }
        wgpuDeviceSetLoggingCallback(device_, loggingCallbackInfo);
    }

    return true;
}

void WGRenderSystem::CreateCommandQueue()
{
    commandQueue_ = MakeUnique<WGCommandQueue>(device_);
}


} // /namespace LLGL



// ================================================================================
