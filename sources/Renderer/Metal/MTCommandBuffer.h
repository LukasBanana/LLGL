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


namespace LLGL
{


class MTCommandBuffer : public CommandBufferExt
{

    public:

        /* ----- Common ----- */

        ~MTCommandBuffer();

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
    
        /* ----- Constant Buffers ------ */

        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Storage Buffers ----- */

        void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Samplers ----- */

        void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

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

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;
        bool QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
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

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) override;
    
        /* ----- Extended functions ----- */
    
        void NextCommandBuffer(id<MTLCommandQueue> cmdQueue);

    private:
    
        static const std::uint32_t g_maxNumViewportsAndScissors = 32;
        static const std::uint32_t g_maxNumVertexBuffers        = 16;
    
        struct MTRenderEncoderState
        {
            MTLViewport                 viewports[g_maxNumViewportsAndScissors]     = {};
            NSUInteger                  viewportCount                               = 0;
            MTLScissorRect              scissorRects[g_maxNumViewportsAndScissors]  = {};
            NSUInteger                  scissorRectCount                            = 0;
            id<MTLBuffer>               vertexBuffer0                               = nil;
            const id<MTLBuffer>*        vertexBuffers                               = nullptr;
            NSUInteger                  vertexBufferOffset0                         = 0;
            const NSUInteger*           vertexBufferOffsets                         = nullptr;
            NSRange                     vertexBufferRange                           = { 0, 0 };
            id<MTLRenderPipelineState>  renderPipelineState                         = nil;
            id<MTLDepthStencilState>    depthStencilState                           = nil;
        };
    
        // Submits all global lstates to the render encoder (i.e. vertex buffers, graphics pipeline, viewports etc.)
        void SubmitRenderEncoderState();
        void ResetRenderEncoderState();

        id<MTLCommandBuffer>            cmdBuffer_              = nil;

        id<MTLRenderCommandEncoder>     renderEncoder_          = nil;
        id<MTLComputeCommandEncoder>    computeEncoder_         = nil;

        MTLPrimitiveType                primitiveType_          = MTLPrimitiveTypeTriangle;
        id<MTLBuffer>                   indexBuffer_            = nil;
        MTLIndexType                    indexType_              = MTLIndexTypeUInt32;
        NSUInteger                      indexTypeSize_          = 4;
        NSUInteger                      numPatchControlPoints_  = 0;
    
        MTRenderEncoderState            renderEncoderState_;

};


} // /namespace LLGL


#endif



// ================================================================================
