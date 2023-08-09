/*
 * CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_COMMAND_BUFFER_H
#define LLGL_C99_COMMAND_BUFFER_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stddef.h>
#include <stdbool.h>


LLGL_C_EXPORT void llglBegin(LLGLCommandBuffer commandBuffer);
LLGL_C_EXPORT void llglEnd();
LLGL_C_EXPORT void llglExecute(LLGLCommandBuffer deferredCommandBuffer);
LLGL_C_EXPORT void llglUpdateBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, const void* data, uint16_t dataSize);
LLGL_C_EXPORT void llglCopyBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, LLGLBuffer srcBuffer, uint64_t srcOffset, uint64_t size);
LLGL_C_EXPORT void llglCopyBufferFromTexture(LLGLBuffer dstBuffer, uint64_t dstOffset, LLGLTexture srcTexture, const LLGLTextureRegion* srcRegion, uint32_t rowStride, uint32_t layerStride);
LLGL_C_EXPORT void llglFillBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, uint32_t value, uint64_t fillSize);
LLGL_C_EXPORT void llglCopyTexture(LLGLTexture dstTexture, const LLGLTextureLocation* dstLocation, LLGLTexture srcTexture, const LLGLTextureLocation* srcLocation, const LLGLExtent3D* extent);
LLGL_C_EXPORT void llglCopyTextureFromBuffer(LLGLTexture dstTexture, const LLGLTextureRegion* dstRegion, LLGLBuffer srcBuffer, uint64_t srcOffset, uint32_t rowStride, uint32_t layerStride);
LLGL_C_EXPORT void llglCopyTextureFromFramebuffer(LLGLTexture dstTexture, const LLGLTextureRegion* dstRegion, const LLGLOffset2D* srcOffset);
LLGL_C_EXPORT void llglGenerateMips(LLGLTexture texture);
LLGL_C_EXPORT void llglGenerateMipsRange(LLGLTexture texture, const LLGLTextureSubresource* subresource);
LLGL_C_EXPORT void llglSetViewport(const LLGLViewport* viewport);
LLGL_C_EXPORT void llglSetViewports(uint32_t numViewports, const LLGLViewport* viewports);
LLGL_C_EXPORT void llglSetScissor(const LLGLScissor* scissor);
LLGL_C_EXPORT void llglSetScissors(uint32_t numScissors, const LLGLScissor* scissors);
LLGL_C_EXPORT void llglSetVertexBuffer(LLGLBuffer buffer);
LLGL_C_EXPORT void llglSetVertexBufferArray(LLGLBufferArray bufferArray);
LLGL_C_EXPORT void llglSetIndexBuffer(LLGLBuffer buffer);
LLGL_C_EXPORT void llglSetIndexBufferExt(LLGLBuffer buffer, LLGLFormat format, uint64_t offset);
LLGL_C_EXPORT void llglSetResourceHeap(LLGLResourceHeap resourceHeap, uint32_t descriptorSet);
LLGL_C_EXPORT void llglSetResource(uint32_t descriptor, LLGLResource resource);
LLGL_C_EXPORT void llglResetResourceSlots(LLGLResourceType resourceType, uint32_t firstSlot, uint32_t numSlots, long bindFlags, long stageFlags);
LLGL_C_EXPORT void llglBeginRenderPass(LLGLRenderTarget renderTarget);
LLGL_C_EXPORT void llglBeginRenderPassWithClear(LLGLRenderTarget renderTarget, LLGLRenderPass renderPass, uint32_t numClearValues, const LLGLClearValue* clearValues, uint32_t swapBufferIndex);
LLGL_C_EXPORT void llglEndRenderPass();
LLGL_C_EXPORT void llglClear(long flags, const LLGLClearValue* clearValue);
LLGL_C_EXPORT void llglClearAttachments(uint32_t numAttachments, const LLGLAttachmentClear* attachments);
LLGL_C_EXPORT void llglSetPipelineState(LLGLPipelineState pipelineState);
LLGL_C_EXPORT void llglSetBlendFactor(const float color[4]);
LLGL_C_EXPORT void llglSetStencilReference(uint32_t reference, LLGLStencilFace stencilFace);
LLGL_C_EXPORT void llglSetUniforms(uint32_t first, const void* data, uint16_t dataSize);
LLGL_C_EXPORT void llglBeginQuery(LLGLQueryHeap queryHeap, uint32_t query);
LLGL_C_EXPORT void llglEndQuery(LLGLQueryHeap queryHeap, uint32_t query);
LLGL_C_EXPORT void llglBeginRenderCondition(LLGLQueryHeap queryHeap, uint32_t query, LLGLRenderConditionMode mode);
LLGL_C_EXPORT void llglEndRenderCondition();
LLGL_C_EXPORT void llglBeginStreamOutput(uint32_t numBuffers, LLGLBuffer const * buffers);
LLGL_C_EXPORT void llglEndStreamOutput();
LLGL_C_EXPORT void llglDraw(uint32_t numVertices, uint32_t firstVertex);
LLGL_C_EXPORT void llglDrawIndexed(uint32_t numIndices, uint32_t firstIndex);
LLGL_C_EXPORT void llglDrawIndexedExt(uint32_t numIndices, uint32_t firstIndex, int32_t vertexOffset);
LLGL_C_EXPORT void llglDrawInstanced(uint32_t numVertices, uint32_t firstVertex, uint32_t numInstances);
LLGL_C_EXPORT void llglDrawInstancedExt(uint32_t numVertices, uint32_t firstVertex, uint32_t numInstances, uint32_t firstInstance);
LLGL_C_EXPORT void llglDrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex);
LLGL_C_EXPORT void llglDrawIndexedInstancedExt(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
LLGL_C_EXPORT void llglDrawIndirect(LLGLBuffer buffer, uint64_t offset);
LLGL_C_EXPORT void llglDrawIndirectExt(LLGLBuffer buffer, uint64_t offset, uint32_t numCommands, uint32_t stride);
LLGL_C_EXPORT void llglDrawIndexedIndirect(LLGLBuffer buffer, uint64_t offset);
LLGL_C_EXPORT void llglDrawIndexedIndirectExt(LLGLBuffer buffer, uint64_t offset, uint32_t numCommands, uint32_t stride);
LLGL_C_EXPORT void llglDispatch(uint32_t numWorkGroupsX, uint32_t numWorkGroupsY, uint32_t numWorkGroupsZ);
LLGL_C_EXPORT void llglDispatchIndirect(LLGLBuffer buffer, uint64_t offset);
LLGL_C_EXPORT void llglPushDebugGroup(const char* name);
LLGL_C_EXPORT void llglPopDebugGroup();
LLGL_C_EXPORT void llglDoNativeCommand(const void* nativeCommand, size_t nativeCommandSize);
LLGL_C_EXPORT bool llglGetNativeHandle(void* nativeHandle, size_t nativeHandleSize);


#endif



// ================================================================================
