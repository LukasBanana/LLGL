/*
 * WGRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGRenderSystem.h"
#include "RenderState/WGComputePipeline.h"
#include "RenderState/WGRenderPipeline.h"
#include "../../Core/Assertion.h"
#include "../../Core/StringUtils.h"


namespace LLGL
{


WGRenderSystem::WGRenderSystem(const RenderSystemDescriptor& desc)
{
    CreateWebGpuInstance();
    if (!RequestWebGpuAdapter())
        return;
    if (!RequestWebGpuDevice(desc.flags))
        return;
    CreateCommandQueue();
}

SwapChain* WGRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<WGSwapChain>(swapChainDesc, surface, *this);
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
    return commandBuffers_.emplace<WGCommandBuffer>(device_, commandBufferDesc);
}

void WGRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

Buffer* WGRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    return buffers_.emplace<WGBuffer>(bufferDesc, initialData);
}

BufferArray* WGRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void WGRenderSystem::Release(BufferArray& bufferArray)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
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
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(Texture& texture)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Sampler* WGRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(Sampler& sampler)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
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
    return shaders_.emplace<WGShader>(shaderDesc);
}

void WGRenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

PipelineLayout* WGRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

void WGRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
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
    return pipelineStates_.emplace<WGRenderPipeline>(pipelineStateDesc);
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

bool WGRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    return false; //todo
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

static const char* ToString(WGPUWaitStatus status)
{
    switch (status)
    {
        case WGPUWaitStatus_Success:    return "Success";
        case WGPUWaitStatus_TimedOut:   return "TimedOut";
        case WGPUWaitStatus_Error:      return "Error";
        default:                        return IntToHex(static_cast<std::uint32_t>(status));
    }
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

bool WGRenderSystem::RequestWebGpuDevice(long renderSystemFlags)
{
    /* Enable Dawn toggles depending on render system flags */
    SmallVector<const char*, 8> enabledToggles;
    if ((renderSystemFlags & RenderSystemFlags::DebugDevice) != 0)
        enabledToggles.push_back("dump_shaders");
    else
        enabledToggles.push_back("skip_validation");

    /* Request WebGPU device and wait for result */
    const WGPUFeatureName requiredFeatures[] =
    {
        WGPUFeatureName_CoreFeaturesAndLimits, //TODO
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
    WGPURequestDeviceCallbackInfo callbackInfo;
    {
        callbackInfo.nextInChain    = nullptr;
        callbackInfo.mode           = WGPUCallbackMode_WaitAnyOnly;
        callbackInfo.callback       = OnWebGpuRequestDevice;
        callbackInfo.userdata1      = &device_;
        callbackInfo.userdata2      = nullptr;
    }
    WGPUFutureWaitInfo waitInfo;
    {
        waitInfo.future     = wgpuAdapterRequestDevice(adapter_, &deviceDesc, callbackInfo);
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

void WGRenderSystem::CreateCommandQueue()
{
    commandQueue_ = MakeUnique<WGCommandQueue>(device_);
}


} // /namespace LLGL



// ================================================================================
