/*
 * MTCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_COMMAND_BUFFER_H
#define LLGL_MT_COMMAND_BUFFER_H


#import <MetalKit/MetalKit.h>

#include <LLGL/CommandBufferExt.h>
#include "../StaticLimits.h"


namespace LLGL
{


class MTCommandBuffer : public CommandBufferExt
{

    public:

        /* ----- Common ----- */

        MTCommandBuffer(id<MTLCommandQueue> cmdQueue);
        ~MTCommandBuffer();

        /* ----- Encoding ----- */

        void Begin() override;
        void End() override;

        void UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize) override;
        void CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size) override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Clear ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Resource Heaps ----- */

        void SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;
        void SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet = 0) override;

        /* ----- Render Passes ----- */

        void BeginRenderPass(
            RenderTarget&       renderTarget,
            const RenderPass*   renderPass      = nullptr,
            std::uint32_t       numClearValues  = 0,
            const ClearValue*   clearValues     = nullptr
        ) override;

        void EndRenderPass() override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;
        void EndQuery(QueryHeap& queryHeap, std::uint32_t query = 0) override;

        void BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query = 0, const RenderConditionMode mode = RenderConditionMode::Wait) override;
        void EndRenderCondition() override;

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

        /* ----- Direct Resource Access ------ */

        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetTexture(Texture& texture, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;
        void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        void ResetResourceSlots(
            const ResourceType  resourceType,
            std::uint32_t       firstSlot,
            std::uint32_t       numSlots,
            long                bindFlags,
            long                stageFlags      = StageFlags::AllStages
        ) override;

    private:

        static const std::uint32_t g_maxNumVertexBuffers = 16;

        struct MTRenderEncoderState
        {
            MTLViewport                 viewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]      = {};
            NSUInteger                  viewportCount                                       = 0;
            MTLScissorRect              scissorRects[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]   = {};
            NSUInteger                  scissorRectCount                                    = 0;
            id<MTLBuffer>               vertexBuffer0                                       = nil;
            const id<MTLBuffer>*        vertexBuffers                                       = nullptr;
            NSUInteger                  vertexBufferOffset0                                 = 0;
            const NSUInteger*           vertexBufferOffsets                                 = nullptr;
            NSRange                     vertexBufferRange                                   = { 0, 0 };
            id<MTLRenderPipelineState>  renderPipelineState                                 = nil;
            id<MTLDepthStencilState>    depthStencilState                                   = nil;
        };

        struct MTClearValue
        {
            MTLClearColor   color   = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
            double          depth   = 1.0;
            std::uint32_t   stencil = 0;
        };

        // Submits all global lstates to the render encoder (i.e. vertex buffers, graphics pipeline, viewports etc.)
        void SubmitRenderEncoderState();
        void ResetRenderEncoderState();

        id<MTLCommandQueue>             cmdQueue_               = nil;
        id<MTLCommandBuffer>            cmdBuffer_              = nil;

        id<MTLRenderCommandEncoder>     renderEncoder_          = nil;
        id<MTLComputeCommandEncoder>    computeEncoder_         = nil;

        MTLPrimitiveType                primitiveType_          = MTLPrimitiveTypeTriangle;
        id<MTLBuffer>                   indexBuffer_            = nil;
        MTLIndexType                    indexType_              = MTLIndexTypeUInt32;
        NSUInteger                      indexTypeSize_          = 4;
        NSUInteger                      numPatchControlPoints_  = 0;
        MTLSize                         numThreadsPerGroup_     = { 1, 1, 1 };

        MTRenderEncoderState            renderEncoderState_;

        MTClearValue                    clearValue_;
        MTLRenderPassDescriptor*        renderPassDesc_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
