/*
 * RenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_SYSTEM_H
#define LLGL_C99_RENDER_SYSTEM_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>


LLGL_C_EXPORT int llglLoadRenderSystem(const char* moduleName);
LLGL_C_EXPORT int llglLoadRenderSystemExt(const LLGLRenderSystemDescriptor* renderSystemDesc, LLGLReport report);
LLGL_C_EXPORT void llglUnloadRenderSystem();
LLGL_C_EXPORT void llglMakeRenderSystemCurrent(int id);

LLGL_C_EXPORT int llglGetRendererID();
LLGL_C_EXPORT const char* llglGetRendererName();
LLGL_C_EXPORT void llglGetRendererInfo(LLGLRendererInfo* outInfo);
LLGL_C_EXPORT void llglGetRenderingCaps(LLGLRenderingCapabilities* outCaps);
LLGL_C_EXPORT LLGLReport llglGetRendererReport();

LLGL_C_EXPORT LLGLSwapChain llglCreateSwapChain(const LLGLSwapChainDescriptor* swapChainDesc);
LLGL_C_EXPORT LLGLSwapChain llglCreateSwapChainExt(const LLGLSwapChainDescriptor* swapChainDesc, LLGLSurface surface);
LLGL_C_EXPORT void llglReleaseSwapChain(LLGLSwapChain swapChain);

LLGL_C_EXPORT LLGLCommandBuffer llglCreateCommandBuffer(const LLGLCommandBufferDescriptor* commandBufferDesc);
LLGL_C_EXPORT void llglReleaseCommandBuffer(LLGLCommandBuffer commandBuffer);

LLGL_C_EXPORT LLGLBuffer llglCreateBuffer(const LLGLBufferDescriptor* bufferDesc, const void* initialData);
LLGL_C_EXPORT void llglReleaseBuffer(LLGLBuffer buffer);
LLGL_C_EXPORT void llglWriteBuffer(LLGLBuffer buffer, uint64_t offset, const void* data, uint64_t dataSize);
LLGL_C_EXPORT void llglReadBuffer(LLGLBuffer buffer, uint64_t offset, void* data, uint64_t dataSize);
LLGL_C_EXPORT void* llglMapBuffer(LLGLBuffer buffer, LLGLCPUAccess access);
LLGL_C_EXPORT void* llglMapBufferRange(LLGLBuffer buffer, LLGLCPUAccess access, uint64_t offset, uint64_t length);
LLGL_C_EXPORT void llglUnmapBuffer(LLGLBuffer buffer);

LLGL_C_EXPORT LLGLBufferArray CreateBufferArray(uint32_t numBuffers, const LLGLBuffer* buffers);
LLGL_C_EXPORT void llglReleaseBufferArray(LLGLBufferArray bufferArray);

LLGL_C_EXPORT LLGLTexture llglCreateTexture(const LLGLTextureDescriptor* textureDesc, const LLGLSrcImageDescriptor* imageDesc);
LLGL_C_EXPORT void llglReleaseTexture(LLGLTexture texture);
LLGL_C_EXPORT void llglWriteTexture(LLGLTexture texture, const LLGLTextureRegion* textureRegion, const LLGLSrcImageDescriptor* imageDesc);
LLGL_C_EXPORT void llglReadTexture(LLGLTexture texture, const LLGLTextureRegion* textureRegion, const LLGLDstImageDescriptor* imageDesc);

LLGL_C_EXPORT LLGLSampler llglCreateSampler(const LLGLSamplerDescriptor* samplerDesc);
LLGL_C_EXPORT void llglReleaseSampler(LLGLSampler sampler);

LLGL_C_EXPORT LLGLResourceHeap llglCreateResourceHeap(const LLGLResourceHeapDescriptor* resourceHeapDesc);
LLGL_C_EXPORT LLGLResourceHeap llglCreateResourceHeapExt(const LLGLResourceHeapDescriptor* resourceHeapDesc, size_t numInitialResourceViews, const LLGLResourceViewDescriptor* initialResourceViews);
LLGL_C_EXPORT void llglReleaseResourceHeap(LLGLResourceHeap resourceHeap);
LLGL_C_EXPORT uint32_t llglWriteResourceHeap(LLGLResourceHeap resourceHeap, uint32_t firstDescriptor, size_t numResourceViews, const LLGLResourceViewDescriptor* resourceViews);

LLGL_C_EXPORT LLGLRenderPass llglCreateRenderPass(const LLGLRenderPassDescriptor* renderPassDesc);
LLGL_C_EXPORT void llglReleaseRenderPass(LLGLRenderPass renderPass);

LLGL_C_EXPORT LLGLRenderTarget llglCreateRenderTarget(const LLGLRenderTargetDescriptor* renderTargetDesc);
LLGL_C_EXPORT void llglReleaseRenderTarget(LLGLRenderTarget renderTarget);

LLGL_C_EXPORT LLGLShader llglCreateShader(const LLGLShaderDescriptor* shaderDesc);
LLGL_C_EXPORT void llglReleaseShader(LLGLShader shader);

LLGL_C_EXPORT LLGLPipelineLayout llglCreatePipelineLayout(const LLGLPipelineLayoutDescriptor* pipelineLayoutDesc);
LLGL_C_EXPORT void llglReleasePipelineLayout(LLGLPipelineLayout pipelineLayout);

LLGL_C_EXPORT LLGLPipelineState llglCreateGraphicsPipelineState(const LLGLGraphicsPipelineDescriptor* pipelineStateDesc);
LLGL_C_EXPORT LLGLPipelineState llglCreateComputePipelineState(const LLGLComputePipelineDescriptor* pipelineStateDesc);
LLGL_C_EXPORT void llglReleasePipelineState(LLGLPipelineState pipelineState);

LLGL_C_EXPORT LLGLQueryHeap llglCreateQueryHeap(const LLGLQueryHeapDescriptor* queryHeapDesc);
LLGL_C_EXPORT void llglReleaseQueryHeap(LLGLQueryHeap queryHeap);

LLGL_C_EXPORT LLGLFence llglCreateFence();
LLGL_C_EXPORT void llglReleaseFence(LLGLFence fence);

LLGL_C_EXPORT bool llglGetRenderSystemNativeHandle(void* nativeHandle, size_t nativeHandleSize);


#endif



// ================================================================================
