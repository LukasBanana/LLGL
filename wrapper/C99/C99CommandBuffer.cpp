/*
 * C99CommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/CommandBuffer.h>
#include <LLGL-C/CommandBuffer.h>
#include <LLGL/Utils/ForRange.h>
#include "C99Internal.h"
#include "../../sources/Core/Assertion.h"


// namespace LLGL {


using namespace LLGL;

static thread_local CommandBuffer* g_CurrentCmdBuf = NULL;


LLGL_C_EXPORT void llglBegin(LLGLCommandBuffer commandBuffer)
{
    LLGL_ASSERT(g_CurrentCmdBuf == NULL);
    g_CurrentCmdBuf = LLGL_PTR(CommandBuffer, commandBuffer);
    g_CurrentCmdBuf->Begin();
}

LLGL_C_EXPORT void llglEnd()
{
    LLGL_ASSERT(g_CurrentCmdBuf != NULL);
    g_CurrentCmdBuf->End();
    g_CurrentCmdBuf = NULL;
}

LLGL_C_EXPORT void llglExecute(LLGLCommandBuffer secondaryCommandBuffer)
{
    g_CurrentCmdBuf->Execute(LLGL_REF(CommandBuffer, secondaryCommandBuffer));
}

LLGL_C_EXPORT void llglUpdateBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, const void* data, uint64_t dataSize)
{
    g_CurrentCmdBuf->UpdateBuffer(LLGL_REF(Buffer, dstBuffer), dstOffset, data, dataSize);
}

LLGL_C_EXPORT void llglCopyBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, LLGLBuffer srcBuffer, uint64_t srcOffset, uint64_t size)
{
    g_CurrentCmdBuf->CopyBuffer(LLGL_REF(Buffer, dstBuffer), dstOffset, LLGL_REF(Buffer, srcBuffer), srcOffset, size);
}

LLGL_C_EXPORT void llglCopyBufferFromTexture(LLGLBuffer dstBuffer, uint64_t dstOffset, LLGLTexture srcTexture, const LLGLTextureRegion* srcRegion, uint32_t rowStride, uint32_t layerStride)
{
    g_CurrentCmdBuf->CopyBufferFromTexture(LLGL_REF(Buffer, dstBuffer), dstOffset, LLGL_REF(Texture, srcTexture), *(const TextureRegion*)srcRegion, rowStride, layerStride);
}

LLGL_C_EXPORT void llglFillBuffer(LLGLBuffer dstBuffer, uint64_t dstOffset, uint32_t value, uint64_t fillSize)
{
    g_CurrentCmdBuf->FillBuffer(LLGL_REF(Buffer, dstBuffer), dstOffset, value, fillSize);
}

LLGL_C_EXPORT void llglCopyTexture(LLGLTexture dstTexture, const LLGLTextureLocation* dstLocation, LLGLTexture srcTexture, const LLGLTextureLocation* srcLocation, const LLGLExtent3D* extent)
{
    g_CurrentCmdBuf->CopyTexture(LLGL_REF(Texture, dstTexture), *(const TextureLocation*)dstLocation, LLGL_REF(Texture, srcTexture), *(const TextureLocation*)srcLocation, *(const Extent3D*)extent);
}

LLGL_C_EXPORT void llglCopyTextureFromBuffer(LLGLTexture dstTexture, const LLGLTextureRegion* dstRegion, LLGLBuffer srcBuffer, uint64_t srcOffset, uint32_t rowStride, uint32_t layerStride)
{
    g_CurrentCmdBuf->CopyTextureFromBuffer(LLGL_REF(Texture, dstTexture), *(const TextureRegion*)dstRegion, LLGL_REF(Buffer, srcBuffer), srcOffset, rowStride, layerStride);
}

LLGL_C_EXPORT void llglCopyTextureFromFramebuffer(LLGLTexture dstTexture, const LLGLTextureRegion* dstRegion, const LLGLOffset2D* srcOffset)
{
    g_CurrentCmdBuf->CopyTextureFromFramebuffer(LLGL_REF(Texture, dstTexture), *(const TextureRegion*)dstRegion, *(const Offset2D*)srcOffset);
}

LLGL_C_EXPORT void llglGenerateMips(LLGLTexture texture)
{
    g_CurrentCmdBuf->GenerateMips(LLGL_REF(Texture, texture));
}

LLGL_C_EXPORT void llglGenerateMipsRange(LLGLTexture texture, const LLGLTextureSubresource* subresource)
{
    g_CurrentCmdBuf->GenerateMips(LLGL_REF(Texture, texture), *(const TextureSubresource*)subresource);
}

LLGL_C_EXPORT void llglSetViewport(const LLGLViewport* viewport)
{
    g_CurrentCmdBuf->SetViewport(*(const Viewport*)viewport);
}

LLGL_C_EXPORT void llglSetViewports(uint32_t numViewports, const LLGLViewport* viewports)
{
    g_CurrentCmdBuf->SetViewports(numViewports, (const Viewport*)viewports);
}

LLGL_C_EXPORT void llglSetScissor(const LLGLScissor* scissor)
{
    g_CurrentCmdBuf->SetScissor(*(const Scissor*)scissor);
}

LLGL_C_EXPORT void llglSetScissors(uint32_t numScissors, const LLGLScissor* scissors)
{
    g_CurrentCmdBuf->SetScissors(numScissors, (const Scissor*)scissors);
}

LLGL_C_EXPORT void llglSetVertexBuffer(LLGLBuffer buffer)
{
    g_CurrentCmdBuf->SetVertexBuffer(LLGL_REF(Buffer, buffer));
}

LLGL_C_EXPORT void llglSetVertexBufferArray(LLGLBufferArray bufferArray)
{
    g_CurrentCmdBuf->SetVertexBufferArray(LLGL_REF(BufferArray, bufferArray));
}

LLGL_C_EXPORT void llglSetIndexBuffer(LLGLBuffer buffer)
{
    g_CurrentCmdBuf->SetIndexBuffer(LLGL_REF(Buffer, buffer));
}

LLGL_C_EXPORT void llglSetIndexBufferExt(LLGLBuffer buffer, LLGLFormat format, uint64_t offset)
{
    g_CurrentCmdBuf->SetIndexBuffer(LLGL_REF(Buffer, buffer), (Format)format, offset);
}

LLGL_C_EXPORT void llglSetResourceHeap(LLGLResourceHeap resourceHeap, uint32_t descriptorSet)
{
    g_CurrentCmdBuf->SetResourceHeap(LLGL_REF(ResourceHeap, resourceHeap), descriptorSet);
}

LLGL_C_EXPORT void llglSetResource(uint32_t descriptor, LLGLResource resource)
{
    g_CurrentCmdBuf->SetResource(descriptor, LLGL_REF(Resource, resource));
}

LLGL_C_EXPORT void llglResourceBarrier(uint32_t numBuffers, const LLGLBuffer* buffers, uint32_t numTextures, const LLGLTexture* textures)
{
    constexpr uint32_t maxStaticArray = 64;
    Buffer* internalBuffers[maxStaticArray];
    Texture* internalTextures[maxStaticArray];
    if (numBuffers <= maxStaticArray && numTextures <= maxStaticArray)
    {
        for_range(i, numBuffers)
            internalBuffers[i] = LLGL_PTR(Buffer, buffers[i]);
        for_range(i, numTextures)
            internalTextures[i] = LLGL_PTR(Texture, textures[i]);
        g_CurrentCmdBuf->ResourceBarrier(numBuffers, internalBuffers, numTextures, internalTextures);
    }
    else
    {
        while (numBuffers > 0 || numTextures > 0)
        {
            const uint32_t numBuffersPerBatch = std::min<uint32_t>(numBuffers, maxStaticArray);
            for_range(i, numBuffersPerBatch)
                internalBuffers[i] = LLGL_PTR(Buffer, buffers[i]);

            const uint32_t numTexturesPerBatch = std::min<uint32_t>(numTextures, maxStaticArray);
            for_range(i, numTexturesPerBatch)
                internalTextures[i] = LLGL_PTR(Texture, textures[i]);

            g_CurrentCmdBuf->ResourceBarrier(numBuffersPerBatch, internalBuffers, numTexturesPerBatch, internalTextures);

            numBuffers -= numBuffersPerBatch;
            buffers += numBuffersPerBatch;
            textures += numTexturesPerBatch;
        }
    }
}

LLGL_C_EXPORT void llglResetResourceSlots(LLGLResourceType resourceType, uint32_t firstSlot, uint32_t numSlots, long bindFlags, long stageFlags)
{
    // deprecated
}

LLGL_C_EXPORT void llglBeginRenderPass(LLGLRenderTarget renderTarget)
{
    g_CurrentCmdBuf->BeginRenderPass(LLGL_REF(RenderTarget, renderTarget));
}

LLGL_C_EXPORT void llglBeginRenderPassWithClear(LLGLRenderTarget renderTarget, LLGLRenderPass renderPass, uint32_t numClearValues, const LLGLClearValue* clearValues, uint32_t swapBufferIndex)
{
    g_CurrentCmdBuf->BeginRenderPass(LLGL_REF(RenderTarget, renderTarget), LLGL_PTR(RenderPass, renderPass), numClearValues, (const ClearValue*)clearValues, swapBufferIndex);
}

LLGL_C_EXPORT void llglEndRenderPass()
{
    g_CurrentCmdBuf->EndRenderPass();
}

LLGL_C_EXPORT void llglClear(long flags, const LLGLClearValue* clearValue)
{
    g_CurrentCmdBuf->Clear(flags, *(const ClearValue*)clearValue);
}

LLGL_C_EXPORT void llglClearAttachments(uint32_t numAttachments, const LLGLAttachmentClear* attachments)
{
    g_CurrentCmdBuf->ClearAttachments(numAttachments, (const AttachmentClear*)attachments);
}

LLGL_C_EXPORT void llglSetPipelineState(LLGLPipelineState pipelineState)
{
    g_CurrentCmdBuf->SetPipelineState(LLGL_REF(PipelineState, pipelineState));
}

LLGL_C_EXPORT void llglSetBlendFactor(const float color[4])
{
    g_CurrentCmdBuf->SetBlendFactor(color);
}

LLGL_C_EXPORT void llglSetStencilReference(uint32_t reference, LLGLStencilFace stencilFace)
{
    g_CurrentCmdBuf->SetStencilReference(reference, (StencilFace)stencilFace);
}

LLGL_C_EXPORT void llglSetUniforms(uint32_t first, const void* data, uint16_t dataSize)
{
    g_CurrentCmdBuf->SetUniforms(first, data, dataSize);
}

LLGL_C_EXPORT void llglBeginQuery(LLGLQueryHeap queryHeap, uint32_t query)
{
    g_CurrentCmdBuf->BeginQuery(LLGL_REF(QueryHeap, queryHeap), query);
}

LLGL_C_EXPORT void llglEndQuery(LLGLQueryHeap queryHeap, uint32_t query)
{
    g_CurrentCmdBuf->EndQuery(LLGL_REF(QueryHeap, queryHeap), query);
}

LLGL_C_EXPORT void llglBeginRenderCondition(LLGLQueryHeap queryHeap, uint32_t query, LLGLRenderConditionMode mode)
{
    g_CurrentCmdBuf->BeginRenderCondition(LLGL_REF(QueryHeap, queryHeap), query, (RenderConditionMode)mode);
}

LLGL_C_EXPORT void llglEndRenderCondition()
{
    g_CurrentCmdBuf->EndRenderCondition();
}

LLGL_C_EXPORT void llglBeginStreamOutput(uint32_t numBuffers, LLGLBuffer const * buffers)
{
    Buffer* internalBuffers[LLGL_MAX_NUM_SO_BUFFERS];
    for_range(i, numBuffers)
        internalBuffers[i] = LLGL_PTR(Buffer, buffers[i]);
    g_CurrentCmdBuf->BeginStreamOutput(numBuffers, internalBuffers);
}

LLGL_C_EXPORT void llglEndStreamOutput()
{
    g_CurrentCmdBuf->EndStreamOutput();
}

LLGL_C_EXPORT void llglDraw(uint32_t numVertices, uint32_t firstVertex)
{
    g_CurrentCmdBuf->Draw(numVertices, firstVertex);
}

LLGL_C_EXPORT void llglDrawIndexed(uint32_t numIndices, uint32_t firstIndex)
{
    g_CurrentCmdBuf->DrawIndexed(numIndices, firstIndex);
}

LLGL_C_EXPORT void llglDrawIndexedExt(uint32_t numIndices, uint32_t firstIndex, int32_t vertexOffset)
{
    g_CurrentCmdBuf->DrawIndexed(numIndices, firstIndex, vertexOffset);
}

LLGL_C_EXPORT void llglDrawInstanced(uint32_t numVertices, uint32_t firstVertex, uint32_t numInstances)
{
    g_CurrentCmdBuf->DrawInstanced(numVertices, firstVertex, numInstances);
}

LLGL_C_EXPORT void llglDrawInstancedExt(uint32_t numVertices, uint32_t firstVertex, uint32_t numInstances, uint32_t firstInstance)
{
    g_CurrentCmdBuf->DrawInstanced(numVertices, firstVertex, numInstances, firstInstance);
}

LLGL_C_EXPORT void llglDrawIndexedInstanced(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex)
{
    g_CurrentCmdBuf->DrawIndexedInstanced(numIndices, numInstances, firstIndex);
}

LLGL_C_EXPORT void llglDrawIndexedInstancedExt(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    g_CurrentCmdBuf->DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
}

LLGL_C_EXPORT void llglDrawIndirect(LLGLBuffer buffer, uint64_t offset)
{
    g_CurrentCmdBuf->DrawIndirect(LLGL_REF(Buffer, buffer), offset);
}

LLGL_C_EXPORT void llglDrawIndirectExt(LLGLBuffer buffer, uint64_t offset, uint32_t numCommands, uint32_t stride)
{
    g_CurrentCmdBuf->DrawIndirect(LLGL_REF(Buffer, buffer), offset, numCommands, stride);
}

LLGL_C_EXPORT void llglDrawIndexedIndirect(LLGLBuffer buffer, uint64_t offset)
{
    g_CurrentCmdBuf->DrawIndexedIndirect(LLGL_REF(Buffer, buffer), offset);
}

LLGL_C_EXPORT void llglDrawIndexedIndirectExt(LLGLBuffer buffer, uint64_t offset, uint32_t numCommands, uint32_t stride)
{
    g_CurrentCmdBuf->DrawIndexedIndirect(LLGL_REF(Buffer, buffer), offset, numCommands, stride);
}

LLGL_C_EXPORT void llglDrawStreamOutput()
{
    g_CurrentCmdBuf->DrawStreamOutput();
}

LLGL_C_EXPORT void llglDispatch(uint32_t numWorkGroupsX, uint32_t numWorkGroupsY, uint32_t numWorkGroupsZ)
{
    g_CurrentCmdBuf->Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);
}

LLGL_C_EXPORT void llglDispatchIndirect(LLGLBuffer buffer, uint64_t offset)
{
    g_CurrentCmdBuf->DispatchIndirect(LLGL_REF(Buffer, buffer), offset);
}

LLGL_C_EXPORT void llglPushDebugGroup(const char* name)
{
    g_CurrentCmdBuf->PushDebugGroup(name);
}

LLGL_C_EXPORT void llglPopDebugGroup()
{
    g_CurrentCmdBuf->PopDebugGroup();
}

LLGL_C_EXPORT void llglDoNativeCommand(const void* nativeCommand, size_t nativeCommandSize)
{
    g_CurrentCmdBuf->DoNativeCommand(nativeCommand, nativeCommandSize);
}

LLGL_C_EXPORT bool llglGetNativeHandle(void* nativeHandle, size_t nativeHandleSize)
{
    return g_CurrentCmdBuf->GetNativeHandle(nativeHandle, nativeHandleSize);
}


// } /namespace LLGL



// ================================================================================
