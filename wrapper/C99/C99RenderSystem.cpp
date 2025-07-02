/*
 * C99RenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "C99Internal.h"
#include "C99Bridge.h"


// namespace LLGL {


using namespace LLGL;

extern CommandQueue* g_CurrentCmdQueue;

static int gl_CurrentRenderSystemID = 0;
static RenderSystem* g_CurrentRenderSystem = NULL;
static std::vector<RenderSystemPtr> g_RenderSystems;

#define LLGL_ASSERT_RENDER_SYSTEM() \
    LLGL_ASSERT_PTR(g_CurrentRenderSystem)

#define LLGL_RELEASE(TYPE, OBJ)                                 \
    {                                                           \
        LLGL_ASSERT_RENDER_SYSTEM();                            \
        LLGL_ASSERT_PTR(LLGL_PTR(TYPE, OBJ));                   \
        g_CurrentRenderSystem->Release(LLGL_REF(TYPE, OBJ));    \
    }

LLGL_C_EXPORT int llglLoadRenderSystem(const char* moduleName)
{
    LLGLRenderSystemDescriptor desc = {};
    {
        desc.moduleName = moduleName;
    }
    return llglLoadRenderSystemExt(&desc, LLGL_NULL_OBJECT);
}

LLGL_C_EXPORT int llglLoadRenderSystemExt(const LLGLRenderSystemDescriptor* renderSystemDesc, LLGLReport report)
{
    LLGL_ASSERT_PTR(renderSystemDesc);
    RenderSystemDescriptor internalDesc;
    ConvertRenderSystemDesc(internalDesc, *renderSystemDesc);

    if (RenderSystemPtr renderSystem = RenderSystem::Load(internalDesc, LLGL_PTR(Report, report)))
    {
        size_t renderSystemIndex = g_RenderSystems.size();
        g_RenderSystems.push_back(std::move(renderSystem));

        int renderSystemID = (int)(renderSystemIndex + 1);
        llglMakeRenderSystemCurrent(renderSystemID);

        return renderSystemID;
    }

    return 0;
}

LLGL_C_EXPORT void llglUnloadRenderSystem()
{
    if (gl_CurrentRenderSystemID != 0)
    {
        size_t renderSystemIndex = (size_t)(gl_CurrentRenderSystemID - 1);
        RenderSystem::Unload(std::move(g_RenderSystems[renderSystemIndex]));
        g_RenderSystems.erase(g_RenderSystems.begin() + renderSystemIndex);
        llglMakeRenderSystemCurrent(0);
    }
}

LLGL_C_EXPORT void llglMakeRenderSystemCurrent(int id)
{
    if (gl_CurrentRenderSystemID != id)
    {
        if (id > 0)
        {
            size_t renderSystemIndex = (size_t)(id - 1);
            gl_CurrentRenderSystemID = id;
            g_CurrentRenderSystem = g_RenderSystems[renderSystemIndex].get();
            g_CurrentCmdQueue = g_CurrentRenderSystem->GetCommandQueue();
        }
        else
        {
            gl_CurrentRenderSystemID = 0;
            g_CurrentRenderSystem = NULL;
            g_CurrentCmdQueue = NULL;
        }
    }
}

LLGL_C_EXPORT int llglGetRendererID()
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return g_CurrentRenderSystem->GetRendererID();
}

LLGL_C_EXPORT const char* llglGetRendererName()
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return g_CurrentRenderSystem->GetName();
}

LLGL_C_EXPORT void llglGetRendererInfo(LLGLRendererInfo* outInfo)
{
    static RendererInfoC99Wrapper wrapper;
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(outInfo);
    ConvertRendererInfo(wrapper, *outInfo, g_CurrentRenderSystem->GetRendererInfo());
}

LLGL_C_EXPORT void llglGetRenderingCaps(LLGLRenderingCapabilities* outCaps)
{
    static RenderingCapabilitiesC99Wrapper wrapper;
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(outCaps);
    ConvertRenderingCaps(wrapper, *outCaps, g_CurrentRenderSystem->GetRenderingCaps());
}

LLGL_C_EXPORT LLGLReport llglGetRendererReport()
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return LLGLReport{ g_CurrentRenderSystem->GetReport() };
}

LLGL_C_EXPORT LLGLSwapChain llglCreateSwapChain(const LLGLSwapChainDescriptor* swapChainDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(swapChainDesc);
    return LLGLSwapChain{ g_CurrentRenderSystem->CreateSwapChain(*reinterpret_cast<const SwapChainDescriptor*>(swapChainDesc)) };
}

LLGL_C_EXPORT LLGLSwapChain llglCreateSwapChainExt(const LLGLSwapChainDescriptor* swapChainDesc, LLGLSurface surface)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(swapChainDesc);
    return LLGL_NULL_OBJECT; //todo
}

LLGL_C_EXPORT void llglReleaseSwapChain(LLGLSwapChain swapChain)
{
    LLGL_RELEASE(SwapChain, swapChain);
}

LLGL_C_EXPORT LLGLCommandBuffer llglCreateCommandBuffer(const LLGLCommandBufferDescriptor* commandBufferDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(commandBufferDesc);
    return LLGLCommandBuffer{ g_CurrentRenderSystem->CreateCommandBuffer(*reinterpret_cast<const CommandBufferDescriptor*>(commandBufferDesc)) };
}

LLGL_C_EXPORT void llglReleaseCommandBuffer(LLGLCommandBuffer commandBuffer)
{
    LLGL_RELEASE(CommandBuffer, commandBuffer);
}

LLGL_C_EXPORT LLGLBuffer llglCreateBuffer(const LLGLBufferDescriptor* bufferDesc, const void* initialData)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(bufferDesc);
    BufferDescriptor internalBufferDesc;
    SmallVector<VertexAttribute> internalVertexAttribs;
    ConvertBufferDesc(internalBufferDesc, internalVertexAttribs, *bufferDesc);
    return LLGLBuffer{ g_CurrentRenderSystem->CreateBuffer(internalBufferDesc, initialData) };
}

LLGL_C_EXPORT void llglReleaseBuffer(LLGLBuffer buffer)
{
    LLGL_RELEASE(Buffer, buffer);
}

LLGL_C_EXPORT void llglWriteBuffer(LLGLBuffer buffer, uint64_t offset, const void* data, uint64_t dataSize)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    g_CurrentRenderSystem->WriteBuffer(LLGL_REF(Buffer, buffer), offset, data, dataSize);
}

LLGL_C_EXPORT void llglReadBuffer(LLGLBuffer buffer, uint64_t offset, void* data, uint64_t dataSize)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    g_CurrentRenderSystem->ReadBuffer(LLGL_REF(Buffer, buffer), offset, data, dataSize);
}

LLGL_C_EXPORT void* llglMapBuffer(LLGLBuffer buffer, LLGLCPUAccess access)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return g_CurrentRenderSystem->MapBuffer(LLGL_REF(Buffer, buffer), (CPUAccess)access);
}

LLGL_C_EXPORT void* llglMapBufferRange(LLGLBuffer buffer, LLGLCPUAccess access, uint64_t offset, uint64_t length)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return g_CurrentRenderSystem->MapBuffer(LLGL_REF(Buffer, buffer), (CPUAccess)access, offset, length);
}

LLGL_C_EXPORT void llglUnmapBuffer(LLGLBuffer buffer)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    g_CurrentRenderSystem->UnmapBuffer(LLGL_REF(Buffer, buffer));
}

LLGL_C_EXPORT LLGLBufferArray llglCreateBufferArray(uint32_t numBuffers, const LLGLBuffer* buffers)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(buffers);
    return LLGLBufferArray{ g_CurrentRenderSystem->CreateBufferArray(numBuffers, reinterpret_cast<Buffer* const*>(buffers)) };
}

LLGL_C_EXPORT void llglReleaseBufferArray(LLGLBufferArray bufferArray)
{
    LLGL_RELEASE(BufferArray, bufferArray);
}

LLGL_C_EXPORT LLGLTexture llglCreateTexture(const LLGLTextureDescriptor* textureDesc, const LLGLImageView* initialImage)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(textureDesc);
    return LLGLTexture{ g_CurrentRenderSystem->CreateTexture(*reinterpret_cast<const TextureDescriptor*>(textureDesc), reinterpret_cast<const ImageView*>(initialImage)) };
}

LLGL_C_EXPORT void llglReleaseTexture(LLGLTexture texture)
{
    LLGL_RELEASE(Texture, texture);
}

LLGL_C_EXPORT void llglWriteTexture(LLGLTexture texture, const LLGLTextureRegion* textureRegion, const LLGLImageView* srcImageView)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(textureRegion);
    LLGL_ASSERT_PTR(srcImageView);
    g_CurrentRenderSystem->WriteTexture(LLGL_REF(Texture, texture), *reinterpret_cast<const TextureRegion*>(textureRegion), *reinterpret_cast<const ImageView*>(srcImageView));
}

LLGL_C_EXPORT void llglReadTexture(LLGLTexture texture, const LLGLTextureRegion* textureRegion, const LLGLMutableImageView* dstImageView)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(textureRegion);
    LLGL_ASSERT_PTR(dstImageView);
    g_CurrentRenderSystem->ReadTexture(LLGL_REF(Texture, texture), *reinterpret_cast<const TextureRegion*>(textureRegion), *reinterpret_cast<const MutableImageView*>(dstImageView));
}

LLGL_C_EXPORT LLGLSampler llglCreateSampler(const LLGLSamplerDescriptor* samplerDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(samplerDesc);
    return LLGLSampler{ g_CurrentRenderSystem->CreateSampler(*reinterpret_cast<const SamplerDescriptor*>(samplerDesc)) };
}

LLGL_C_EXPORT void llglReleaseSampler(LLGLSampler sampler)
{
    LLGL_RELEASE(Sampler, sampler);
}

LLGL_C_EXPORT LLGLResourceHeap llglCreateResourceHeap(const LLGLResourceHeapDescriptor* resourceHeapDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(resourceHeapDesc);
    return LLGLResourceHeap{ g_CurrentRenderSystem->CreateResourceHeap(*reinterpret_cast<const ResourceHeapDescriptor*>(resourceHeapDesc)) };
}

LLGL_C_EXPORT LLGLResourceHeap llglCreateResourceHeapExt(const LLGLResourceHeapDescriptor* resourceHeapDesc, size_t numInitialResourceViews, const LLGLResourceViewDescriptor* initialResourceViews)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(resourceHeapDesc);
    const ArrayView<ResourceViewDescriptor> internalInitialResourceViews{ reinterpret_cast<const ResourceViewDescriptor*>(initialResourceViews), numInitialResourceViews };
    return LLGLResourceHeap{ g_CurrentRenderSystem->CreateResourceHeap(*reinterpret_cast<const ResourceHeapDescriptor*>(resourceHeapDesc), internalInitialResourceViews) };
}

LLGL_C_EXPORT void llglReleaseResourceHeap(LLGLResourceHeap resourceHeap)
{
    LLGL_RELEASE(ResourceHeap, resourceHeap);
}

LLGL_C_EXPORT uint32_t llglWriteResourceHeap(LLGLResourceHeap resourceHeap, uint32_t firstDescriptor, size_t numResourceViews, const LLGLResourceViewDescriptor* resourceViews)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    if (numResourceViews > 0)
    {
        LLGL_ASSERT_PTR(resourceViews);
        const ArrayView<ResourceViewDescriptor> internalResourceViews{ reinterpret_cast<const ResourceViewDescriptor*>(resourceViews), numResourceViews };
        return g_CurrentRenderSystem->WriteResourceHeap(LLGL_REF(ResourceHeap, resourceHeap), firstDescriptor, internalResourceViews);
    }
    return 0;
}

LLGL_C_EXPORT LLGLRenderPass llglCreateRenderPass(const LLGLRenderPassDescriptor* renderPassDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(renderPassDesc);
    return LLGLRenderPass{ g_CurrentRenderSystem->CreateRenderPass(*reinterpret_cast<const RenderPassDescriptor*>(renderPassDesc)) };
}

LLGL_C_EXPORT void llglReleaseRenderPass(LLGLRenderPass renderPass)
{
    LLGL_RELEASE(RenderPass, renderPass);
}

LLGL_C_EXPORT LLGLRenderTarget llglCreateRenderTarget(const LLGLRenderTargetDescriptor* renderTargetDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(renderTargetDesc);
    return LLGLRenderTarget{ g_CurrentRenderSystem->CreateRenderTarget(*reinterpret_cast<const RenderTargetDescriptor*>(renderTargetDesc)) };
}

LLGL_C_EXPORT void llglReleaseRenderTarget(LLGLRenderTarget renderTarget)
{
    LLGL_RELEASE(RenderTarget, renderTarget);
}

LLGL_C_EXPORT LLGLShader llglCreateShader(const LLGLShaderDescriptor* shaderDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(shaderDesc);
    ShaderDescriptor internalShaderDesc;
    ConvertShaderDesc(internalShaderDesc, *shaderDesc);
    return LLGLShader{ g_CurrentRenderSystem->CreateShader(internalShaderDesc) };
}

LLGL_C_EXPORT void llglReleaseShader(LLGLShader shader)
{
    LLGL_RELEASE(Shader, shader);
}

LLGL_C_EXPORT LLGLPipelineLayout llglCreatePipelineLayout(const LLGLPipelineLayoutDescriptor* pipelineLayoutDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(pipelineLayoutDesc);
    PipelineLayoutDescriptor internalPipelineLayoutDesc;
    ConvertPipelineLayoutDesc(internalPipelineLayoutDesc, *pipelineLayoutDesc);
    return LLGLPipelineLayout{ g_CurrentRenderSystem->CreatePipelineLayout(internalPipelineLayoutDesc) };
}

LLGL_C_EXPORT void llglReleasePipelineLayout(LLGLPipelineLayout pipelineLayout)
{
    LLGL_RELEASE(PipelineLayout, pipelineLayout);
}

LLGL_C_EXPORT LLGLPipelineCache llglCreatePipelineCache(const void* initialBlobData, size_t initialBlobsize)
{
    Blob initialBlob = (initialBlobData != nullptr ? Blob::CreateWeakRef(initialBlobData, initialBlobsize) : Blob{});
    return LLGLPipelineCache{ g_CurrentRenderSystem->CreatePipelineCache(initialBlob) };
}

LLGL_C_EXPORT void llglReleasePipelineCache(LLGLPipelineCache pipelineCache)
{
    LLGL_RELEASE(PipelineCache, pipelineCache);
}

LLGL_C_EXPORT LLGLPipelineState llglCreateGraphicsPipelineState(const LLGLGraphicsPipelineDescriptor* pipelineStateDesc)
{
    return llglCreateGraphicsPipelineStateExt(pipelineStateDesc, LLGL_NULL_OBJECT);
}

LLGL_C_EXPORT LLGLPipelineState llglCreateGraphicsPipelineStateExt(const LLGLGraphicsPipelineDescriptor* pipelineStateDesc, LLGLPipelineCache pipelineCache)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(pipelineStateDesc);
    GraphicsPipelineDescriptor internalPipelineStateDesc;
    ConvertGraphicsPipelineDesc(internalPipelineStateDesc, *pipelineStateDesc);
    return LLGLPipelineState{ g_CurrentRenderSystem->CreatePipelineState(internalPipelineStateDesc, LLGL_PTR(PipelineCache, pipelineCache)) };
}

LLGL_C_EXPORT LLGLPipelineState llglCreateComputePipelineState(const LLGLComputePipelineDescriptor* pipelineStateDesc)
{
    return llglCreateComputePipelineStateExt(pipelineStateDesc, LLGL_NULL_OBJECT);
}

LLGL_C_EXPORT LLGLPipelineState llglCreateComputePipelineStateExt(const LLGLComputePipelineDescriptor* pipelineStateDesc, LLGLPipelineCache pipelineCache)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(pipelineStateDesc);
    ComputePipelineDescriptor internalPipelineStateDesc;
    ConvertComputePipelineDesc(internalPipelineStateDesc, *pipelineStateDesc);
    return LLGLPipelineState{ g_CurrentRenderSystem->CreatePipelineState(internalPipelineStateDesc, LLGL_PTR(PipelineCache, pipelineCache)) };
}

LLGL_C_EXPORT void llglReleasePipelineState(LLGLPipelineState pipelineState)
{
    LLGL_RELEASE(PipelineState, pipelineState);
}

LLGL_C_EXPORT LLGLQueryHeap llglCreateQueryHeap(const LLGLQueryHeapDescriptor* queryHeapDesc)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    LLGL_ASSERT_PTR(queryHeapDesc);
    return LLGLQueryHeap{ g_CurrentRenderSystem->CreateQueryHeap(*reinterpret_cast<const QueryHeapDescriptor*>(queryHeapDesc)) };
}

LLGL_C_EXPORT void llglReleaseQueryHeap(LLGLQueryHeap queryHeap)
{
    LLGL_RELEASE(QueryHeap, queryHeap);
}

LLGL_C_EXPORT LLGLFence llglCreateFence()
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return LLGLFence{ g_CurrentRenderSystem->CreateFence() };
}

LLGL_C_EXPORT void llglReleaseFence(LLGLFence fence)
{
    LLGL_RELEASE(Fence, fence);
}

LLGL_C_EXPORT bool llglGetRenderSystemNativeHandle(void* nativeHandle, size_t nativeHandleSize)
{
    LLGL_ASSERT_RENDER_SYSTEM();
    return g_CurrentRenderSystem->GetNativeHandle(nativeHandle, nativeHandleSize);
}


// } /namespace LLGL



// ================================================================================
