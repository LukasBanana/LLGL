/*
 * DbgCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMMAND_BUFFER_H
#define LLGL_DBG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>

#include "DbgGraphicsPipeline.h"


namespace LLGL
{


class DbgBuffer;

class DbgCommandBuffer : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        DbgCommandBuffer(
            CommandBuffer& instance,
            RenderingProfiler* profiler,
            RenderingDebugger* debugger,
            const RenderingCaps& caps
        );

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewportArray(std::uint32_t numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(std::uint32_t numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearTarget(std::uint32_t targetIndex, const LLGL::ColorRGBAf& color) override;

        /* ----- Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        void SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        void SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, std::uint32_t slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void SetRenderTarget(RenderContext& renderContext) override;

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

        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset) override;

        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) override;

        /* ----- Debugging members ----- */

        CommandBuffer& instance;

    private:

        void DebugGraphicsPipelineSet();
        void DebugComputePipelineSet();
        void DebugVertexBufferSet();
        void DebugIndexBufferSet();
        void DebugVertexLayout();

        void DebugNumVertices(std::uint32_t numVertices);
        void DebugNumInstances(std::uint32_t numInstances, std::uint32_t instanceOffset);

        void DebugDraw(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset);
        void DebugDrawIndexed(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset);

        void DebugInstancing();
        void DebugVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit);
        void DebugThreadGroupLimit(std::uint32_t size, std::uint32_t limit);

        void DebugShaderStageFlags(long shaderStageFlags, long validFlags);
        void DebugBufferType(const BufferType bufferType, const BufferType compareType);

        void WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices);

        /* ----- Common objects ----- */

        RenderingProfiler*      profiler_       = nullptr;
        RenderingDebugger*      debugger_       = nullptr;

        const RenderingCaps&    caps_;

        /* ----- Render states ----- */

        PrimitiveTopology       topology_       = PrimitiveTopology::TriangleList;
        VertexFormat            vertexFormat_;

        struct Bindings
        {
            DbgBuffer*              vertexBuffer        = nullptr;
            DbgBuffer*              indexBuffer         = nullptr;
            DbgBuffer*              streamOutput        = nullptr;
            DbgGraphicsPipeline*    graphicsPipeline    = nullptr;
            ComputePipeline*        computePipeline     = nullptr;
        }
        bindings_;

        struct States
        {
            bool streamOutputBusy = false;
        }
        states_;

};


} // /namespace LLGL


#endif



// ================================================================================
