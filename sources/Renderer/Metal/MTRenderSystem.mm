/*
 * MTRenderSystem.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderSystem.h"
#include "../CheckedCast.h"
#include "../TextureUtils.h"
#include "../../Core/Helper.h"
#include "../../Core/Vendor.h"
#include "MTFeatureSet.h"
#include "MTTypes.h"
#include "RenderState/MTGraphicsPSO.h"
#include "RenderState/MTComputePSO.h"
#include "RenderState/MTBuiltinPSOFactory.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Platform/Platform.h>
#include <AvailabilityMacros.h>


namespace LLGL
{


/* ----- Common ----- */

MTRenderSystem::MTRenderSystem()
{
    CreateDeviceResources();
    QueryRenderingCaps();
}

MTRenderSystem::~MTRenderSystem()
{
    [device_ release];
}

/* ----- Render Context ----- */

RenderContext* MTRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(renderContexts_, MakeUnique<MTRenderContext>(device_, desc, surface));
}

void MTRenderSystem::Release(RenderContext& renderContext)
{
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* MTRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* MTRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& /*desc*/)
{
    return TakeOwnership(commandBuffers_, MakeUnique<MTCommandBuffer>(device_, commandQueue_->GetNative()));
}

void MTRenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

Buffer* MTRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    return TakeOwnership(buffers_, MakeUnique<MTBuffer>(device_, desc, initialData));
}

BufferArray* MTRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    return TakeOwnership(bufferArrays_, MakeUnique<MTBufferArray>(bufferArray[0]->GetBindFlags(), numBuffers, bufferArray));
}

void MTRenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void MTRenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void MTRenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferMT = LLGL_CAST(MTBuffer&, dstBuffer);
    dstBufferMT.Write(static_cast<NSUInteger>(dstOffset), data, static_cast<NSUInteger>(dataSize));
}

void* MTRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    return bufferMT.Map(access);
}

void MTRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferMT = LLGL_CAST(MTBuffer&, buffer);
    return bufferMT.Unmap();
}

/* ----- Textures ----- */

Texture* MTRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    auto textureMT = MakeUnique<MTTexture>(device_, textureDesc);

    if (imageDesc)
    {
        textureMT->WriteRegion(
            //TextureRegion{ Offset3D{ 0, 0, 0 }, textureMT->GetMipExtent(0) },
            TextureRegion
            {
                TextureSubresource{ 0, textureDesc.arrayLayers, 0, 1 },
                Offset3D{ 0, 0, 0 },
                textureDesc.extent
            },
            *imageDesc
        );

        /* Generate MIP-maps if enabled */
        if (MustGenerateMipsOnCreate(textureDesc))
        {
            id<MTLCommandBuffer> cmdBuffer = [commandQueue_->GetNative() commandBuffer];
            {
                id<MTLBlitCommandEncoder> blitCmdEncoder = [cmdBuffer blitCommandEncoder];
                [blitCmdEncoder generateMipmapsForTexture:textureMT->GetNative()];
                [blitCmdEncoder endEncoding];
            }
            [cmdBuffer commit];
        }
    }

    return TakeOwnership(textures_, std::move(textureMT));
}

void MTRenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void MTRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    textureMT.WriteRegion(textureRegion, imageDesc);
}

void MTRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    auto& textureMT = LLGL_CAST(MTTexture&, texture);
    textureMT.ReadRegion(textureRegion, imageDesc);
}

/* ----- Sampler States ---- */

Sampler* MTRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<MTSampler>(device_, desc));
}

void MTRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Heaps ----- */

ResourceHeap* MTRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<MTResourceHeap>(desc));
}

void MTRenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* MTRenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    AssertCreateRenderPass(desc);
    return TakeOwnership(renderPasses_, MakeUnique<MTRenderPass>(desc));
}

void MTRenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* MTRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<MTRenderTarget>(device_, desc));
}

void MTRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* MTRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    AssertCreateShader(desc);
    return TakeOwnership(shaders_, MakeUnique<MTShader>(device_, desc));
}

ShaderProgram* MTRenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    #if 0//TESS
    AssertCreateShaderProgram(desc);
    #endif
    return TakeOwnership(shaderPrograms_, MakeUnique<MTShaderProgram>(device_, desc));
}

void MTRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

void MTRenderSystem::Release(ShaderProgram& shaderProgram)
{
    RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* MTRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<MTPipelineLayout>(desc));
}

void MTRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

PipelineState* MTRenderSystem::CreatePipelineState(const Blob& /*serializedCache*/)
{
    return nullptr;//TODO
}

PipelineState* MTRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return TakeOwnership(pipelineStates_, MakeUnique<MTGraphicsPSO>(device_, desc, GetDefaultRenderPass()));
}

PipelineState* MTRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* /*serializedCache*/)
{
    return TakeOwnership(pipelineStates_, MakeUnique<MTComputePSO>(device_, desc));
}

void MTRenderSystem::Release(PipelineState& pipelineState)
{
    RemoveFromUniqueSet(pipelineStates_, &pipelineState);
}

/* ----- Queries ----- */

QueryHeap* MTRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(QueryHeap& queryHeap)
{
    //todo
    //RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* MTRenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<MTFence>(device_));
}

void MTRenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
}


/*
 * ======= Private: =======
 */

void MTRenderSystem::CreateDeviceResources()
{
    /* Create Metal device */
    device_ = MTLCreateSystemDefaultDevice();
    if (device_ == nil)
        throw std::runtime_error("failed to create Metal device");
    
    /* Initialize renderer information */
    RendererInfo info;
    {
        info.rendererName           = "Metal " + std::string(QueryMetalVersion());
        info.deviceName             = [[device_ name] cStringUsingEncoding:NSUTF8StringEncoding];
        info.vendorName             = "Apple";
        info.shadingLanguageName    = "Metal Shading Language";
    }
    SetRendererInfo(info);

    /* Create command queue */
    commandQueue_ = MakeUnique<MTCommandQueue>(device_);

    /* Initialize builtin PSOs */
    MTBuiltinPSOFactory::Get().CreateBuiltinPSOs(device_);
}

void MTRenderSystem::QueryRenderingCaps()
{
    RenderingCapabilities caps;
    LoadFeatureSetCaps(device_, QueryHighestFeatureSet(), caps);
    SetRenderingCaps(caps);
}

const char* MTRenderSystem::QueryMetalVersion() const
{
    const auto featureSet = QueryHighestFeatureSet();

    #ifdef LLGL_OS_IOS

    //TODO:
    switch (featureSet)
    {
        case MTLFeatureSet_iOS_GPUFamily5_v1: return "GPU Family 5.v1";
        case MTLFeatureSet_iOS_GPUFamily4_v2: return "GPU Family 4.v2";
        case MTLFeatureSet_iOS_GPUFamily3_v4: return "GPU Family 3.v4";
        case MTLFeatureSet_iOS_GPUFamily2_v5: return "GPU Family 2.v5";
        case MTLFeatureSet_iOS_GPUFamily1_v5: return "GPU Family 1.v5";

        case MTLFeatureSet_iOS_GPUFamily4_v1: return "GPU Family 4.v1";
        case MTLFeatureSet_iOS_GPUFamily3_v3: return "GPU Family 3.v3";
        case MTLFeatureSet_iOS_GPUFamily2_v4: return "GPU Family 2.v4";
        case MTLFeatureSet_iOS_GPUFamily1_v4: return "GPU Family 1.v4";

        case MTLFeatureSet_iOS_GPUFamily3_v2: return "GPU Family 3.v2";
        case MTLFeatureSet_iOS_GPUFamily2_v3: return "GPU Family 2.v3";
        case MTLFeatureSet_iOS_GPUFamily1_v3: return "GPU Family 1.v3";

        case MTLFeatureSet_iOS_GPUFamily3_v1: return "GPU Family 3.v1";
        case MTLFeatureSet_iOS_GPUFamily2_v2: return "GPU Family 2.v2";
        case MTLFeatureSet_iOS_GPUFamily1_v2: return "GPU Family 1.v2";

        case MTLFeatureSet_iOS_GPUFamily2_v1: return "GPU Family 2.v1";
        case MTLFeatureSet_iOS_GPUFamily1_v1: return "GPU Family 1.v1";
    }

    #else

    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_11
    switch (featureSet)
    {
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
        case MTLFeatureSet_macOS_GPUFamily2_v1: return "2.1";
        case MTLFeatureSet_macOS_GPUFamily1_v4: return "1.4";
        #endif
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
        case MTLFeatureSet_macOS_GPUFamily1_v3: return "1.3";
        #endif
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
        case MTLFeatureSet_macOS_GPUFamily1_v2: return "1.2";
        #endif
        case MTLFeatureSet_macOS_GPUFamily1_v1: return "1.1";
        default:                                break;
    }
    #endif

    #endif // /LLGL_OS_IOS
    return "1.0";
}

#ifdef LLGL_OS_IOS

static void GetFeatureSetsForIOS(const MTLFeatureSet*& fsets, std::size_t& count, MTLFeatureSet& fsetDefault)
{
    static const MTLFeatureSet g_featureSetsIOS[] =
    {
        MTLFeatureSet_iOS_GPUFamily5_v1,
        MTLFeatureSet_iOS_GPUFamily4_v2,
        MTLFeatureSet_iOS_GPUFamily3_v4,
        MTLFeatureSet_iOS_GPUFamily2_v5,
        MTLFeatureSet_iOS_GPUFamily1_v5,

        MTLFeatureSet_iOS_GPUFamily4_v1,
        MTLFeatureSet_iOS_GPUFamily3_v3,
        MTLFeatureSet_iOS_GPUFamily2_v4,
        MTLFeatureSet_iOS_GPUFamily1_v4,

        MTLFeatureSet_iOS_GPUFamily3_v2,
        MTLFeatureSet_iOS_GPUFamily2_v3,
        MTLFeatureSet_iOS_GPUFamily1_v3,

        MTLFeatureSet_iOS_GPUFamily3_v1,
        MTLFeatureSet_iOS_GPUFamily2_v2,
        MTLFeatureSet_iOS_GPUFamily1_v2,

        MTLFeatureSet_iOS_GPUFamily2_v1,
        MTLFeatureSet_iOS_GPUFamily1_v1,
    };

    fsets       = g_featureSetsIOS;
    count       = sizeof(g_featureSetsIOS)/sizeof(g_featureSetsIOS[0]);
    fsetDefault = MTLFeatureSet_iOS_GPUFamily1_v1;
}

#else

static void GetFeatureSetsForMacOS(const MTLFeatureSet*& fsets, std::size_t& count, MTLFeatureSet& fsetDefault)
{
    static const MTLFeatureSet g_featureSetsMacOS[] =
    {
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
        MTLFeatureSet_macOS_ReadWriteTextureTier2,
        MTLFeatureSet_macOS_GPUFamily2_v1,
        MTLFeatureSet_macOS_GPUFamily1_v4,
        #endif
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
        MTLFeatureSet_macOS_GPUFamily1_v3,
        #endif
        #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
        MTLFeatureSet_macOS_GPUFamily1_v2,
        #endif
        MTLFeatureSet_macOS_GPUFamily1_v1,
    };

    fsets       = g_featureSetsMacOS;
    count       = sizeof(g_featureSetsMacOS)/sizeof(g_featureSetsMacOS[0]);
    fsetDefault = MTLFeatureSet_macOS_GPUFamily1_v1;
}

#endif // /LLGL_OS_IOS

MTLFeatureSet MTRenderSystem::QueryHighestFeatureSet() const
{
    /* Get list of feature sets for macOS or iOS */
    const MTLFeatureSet* fsets;
    std::size_t count;
    MTLFeatureSet fsetDefault;

    #ifdef LLGL_OS_IOS
    GetFeatureSetsForIOS(fsets, count, fsetDefault);
    #else
    GetFeatureSetsForMacOS(fsets, count, fsetDefault);
    #endif // /LLGL_OS_IOS

    /* Find highest supported feature set */
    for (std::size_t i = 0; i < count; ++i)
    {
        if ([device_ supportsFeatureSet:fsets[i]])
            return fsets[i];
    }

    return fsetDefault;
}

const MTRenderPass* MTRenderSystem::GetDefaultRenderPass() const
{
    if (!renderContexts_.empty())
    {
        if (auto renderPass = (*renderContexts_.begin())->GetRenderPass())
            return LLGL_CAST(const MTRenderPass*, renderPass);
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
