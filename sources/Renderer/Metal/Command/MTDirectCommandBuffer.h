/*
 * MTDirectCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_DIRECT_COMMAND_BUFFER_H
#define LLGL_MT_DIRECT_COMMAND_BUFFER_H


#include "MTCommandBuffer.h"
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


class MTDirectCommandBuffer final : public MTCommandBuffer
{

    public:

        /* ----- Common ----- */

        MTDirectCommandBuffer(id<MTLDevice> device, MTCommandQueue& cmdQueue, const CommandBufferDescriptor& desc);
        ~MTDirectCommandBuffer();

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

        void Execute(CommandBuffer& deferredCommandBuffer) override;

        /* ----- Blitting ----- */

        void UpdateBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            const void*     data,
            std::uint16_t   dataSize
        ) override;

        void CopyBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            Buffer&         srcBuffer,
            std::uint64_t   srcOffset,
            std::uint64_t   size
        ) override;

        void CopyBufferFromTexture(
            Buffer&                 dstBuffer,
            std::uint64_t           dstOffset,
            Texture&                srcTexture,
            const TextureRegion&    srcRegion,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void FillBuffer(
            Buffer&         dstBuffer,
            std::uint64_t   dstOffset,
            std::uint32_t   value,
            std::uint64_t   fillSize    = Constants::wholeSize
        ) override;

        void CopyTexture(
            Texture&                dstTexture,
            const TextureLocation&  dstLocation,
            Texture&                srcTexture,
            const TextureLocation&  srcLocation,
            const Extent3D&         extent
        ) override;

        void CopyTextureFromBuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            Buffer&                 srcBuffer,
            std::uint64_t           srcOffset,
            std::uint32_t           rowStride   = 0,
            std::uint32_t           layerStride = 0
        ) override;

        void CopyTextureFromFramebuffer(
            Texture&                dstTexture,
            const TextureRegion&    dstRegion,
            const Offset2D&         srcOffset
        ) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, const TextureSubresource& subresource) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        /* ----- Resources ----- */

        void SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet = 0) override;

        void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) override;

        /* ----- Render Passes ----- */

        void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr,
            std::uint32_t       swapBufferIndex = Constants::currentSwapIndex
        ) override;

        void EndRenderPass() override;

        void Clear(long flags, const ClearValue& clearValue = {}) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Pipeline States ----- */

        void SetPipelineState(PipelineState& pipelineState) override;
        void SetBlendFactor(const float color[4]) override;
        void SetStencilReference(std::uint32_t reference, const StencilFace stencilFace = StencilFace::FrontAndBack) override;

        /* ----- Queries ----- */

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

        /* ----- Stream Output ------ */

        void BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers) override;
        void EndStreamOutput() override;

        /* ----- Drawing ----- */

        void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;

        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance) override;

        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance) override;

        void DrawIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset) override;
        void DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ) override;
        void DispatchIndirect(Buffer& buffer, std::uint64_t offset) override;

        /* ----- Debugging ----- */

        void PushDebugGroup(const char* name) override;
        void PopDebugGroup() override;

    public:

        // Returns false.
        bool IsMultiSubmitCmdBuffer() const override;

        // Returns the native MTLCommandBuffer object.
        inline id<MTLCommandBuffer> GetNative() const
        {
            return cmdBuffer_;
        }

        // Returns true if this is an immediate command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return ((GetFlags() & CommandBufferFlags::ImmediateSubmit) != 0);
        }

    private:

        void QueueDrawable(id<MTLDrawable> drawable);
        void PresentDrawables();
        
        // Returns the texture of the current drawable from the active framebuffer.
        id<MTLTexture> GetCurrentDrawableTexture() const;

        void FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value);
        void FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);

        id<MTLRenderCommandEncoder> DispatchTessellationAndGetRenderEncoder(NSUInteger numPatches, NSUInteger numInstances = 1);

        // Dispatches the specified amount of local threads in as large threadgroups as possible.
        void DispatchThreads1D(
            id<MTLComputeCommandEncoder>    computeEncoder,
            id<MTLComputePipelineState>     computePSO,
            NSUInteger                      numThreads
        );

    private:

        id<MTLCommandBuffer>            cmdBuffer_              = nil;
        dispatch_semaphore_t            cmdBufferSemaphore_     = nil;

        MTCommandQueue&                 cmdQueue_;
        MTCommandContext                context_;

        SmallVector<id<MTLDrawable>, 2> drawables_;

};


} // /namespace LLGL


#endif



// ================================================================================
