/*
 * DbgCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMMAND_BUFFER_H
#define LLGL_DBG_COMMAND_BUFFER_H


#include <LLGL/CommandBufferExt.h>
#include "DbgGraphicsPipeline.h"
#include <cstdint>


namespace LLGL
{


class DbgBuffer;
class DbgRenderContext;
class DbgRenderTarget;
class DbgQuery;
class RenderingProfiler;
class RenderingDebugger;

class DbgCommandBuffer : public CommandBufferExt
{

    public:

        /* ----- Common ----- */

        DbgCommandBuffer(
            CommandBuffer& instance,
            CommandBufferExt* instanceExt,
            RenderingProfiler* profiler,
            RenderingDebugger* debugger,
            const RenderingCapabilities& caps
        );

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

        /* ----- Constant Buffers ------ */

        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Storage Buffers ------ */

        void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags = StageFlags::AllStages) override;

        /* ----- Resource View Heaps ----- */

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

        void EnableRecording(bool enable);

        /* ----- Debugging members ----- */

        CommandBuffer&      instance;
        CommandBufferExt*   instanceExt = nullptr;

    private:

        void AssertCommandBufferExt(const char* funcName);

        void ValidateViewport(const Viewport& viewport);
        void ValidateAttachmentClear(const AttachmentClear& attachment);

        void ValidateVertexLayout();
        void ValidateVertexLayoutAttributes(const std::vector<VertexAttribute>& shaderAttributes, DbgBuffer** vertexBuffers, std::uint32_t numVertexBuffers);

        void ValidateNumVertices(std::uint32_t numVertices);
        void ValidateNumInstances(std::uint32_t numInstances, std::uint32_t firstInstance);

        void ValidateDrawCmd(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance);
        void ValidateDrawIndexedCmd(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance);

        void ValidateVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit);
        void ValidateThreadGroupLimit(std::uint32_t size, std::uint32_t limit);
        void ValidateAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound);

        void ValidateStageFlags(long stageFlags, long validFlags);
        void ValidateBufferType(const BufferType bufferType, const BufferType compareType);

        void ValidateQueryResult(DbgQuery& query);

        void AssertRecording();
        void AssertInsideRenderPass();
        void AssertGraphicsPipelineBound();
        void AssertComputePipelineBound();
        void AssertVertexBufferBound();
        void AssertIndexBufferBound();

        void AssertInstancingSupported();
        void AssertOffsetInstancingSupported();

        void WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices);

        /* ----- Common objects ----- */

        RenderingProfiler*              profiler_               = nullptr;
        RenderingDebugger*              debugger_               = nullptr;

        //const RenderingCapabilities&    caps_;
        const RenderingFeatures&        features_;
        const RenderingLimits&          limits_;

        /* ----- Render states ----- */

        PrimitiveTopology               topology_               = PrimitiveTopology::TriangleList;

        struct Bindings
        {
            DbgRenderContext*       renderContext           = nullptr;
            DbgRenderTarget*        renderTarget            = nullptr;
            DbgBuffer*              vertexBufferStore[1]    = { nullptr };
            DbgBuffer**             vertexBuffers           = nullptr;
            std::uint32_t           numVertexBuffers        = 0;
            bool                    anyNonEmptyVertexBuffer = false;
            bool                    anyShaderAttributes     = false;
            DbgBuffer*              indexBuffer             = nullptr;
            DbgBuffer*              streamOutput            = nullptr;
            DbgGraphicsPipeline*    graphicsPipeline        = nullptr;
            ComputePipeline*        computePipeline         = nullptr;
        }
        bindings_;

        struct States
        {
            bool recording          = false;
            bool insideRenderPass   = false;
            bool streamOutputBusy   = false;
        }
        states_;

};


} // /namespace LLGL


#endif



// ================================================================================
