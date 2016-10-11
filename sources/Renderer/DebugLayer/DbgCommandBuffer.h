/*
 * DbgCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_COMMAND_BUFFER_H__
#define __LLGL_DBG_COMMAND_BUFFER_H__


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
        void SetViewportArray(unsigned int numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(unsigned int numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        void SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        void SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

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

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
        void EndRenderCondition() override;

        /* ----- Drawing ----- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) override;
        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset) override;

        /* ----- Compute ----- */

        void DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

        /* ----- Debugging members ----- */

        CommandBuffer& instance;

    private:

        void DebugGraphicsPipelineSet(const std::string& source);
        void DebugComputePipelineSet(const std::string& source);
        void DebugVertexBufferSet(const std::string& source);
        void DebugIndexBufferSet(const std::string& source);
        void DebugVertexLayout(const std::string& source);

        void DebugNumVertices(unsigned int numVertices, const std::string& source);
        void DebugNumInstances(unsigned int numInstances, unsigned int instanceOffset, const std::string& source);

        void DebugDraw(
            unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances,
            unsigned int instanceOffset, const std::string& source
        );
        void DebugDrawIndexed(
            unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex,
            int vertexOffset, unsigned int instanceOffset, const std::string& source
        );

        void DebugInstancing(const std::string& source);
        void DebugVertexLimit(unsigned int vertexCount, unsigned int vertexLimit, const std::string& source);
        void DebugThreadGroupLimit(unsigned int size, unsigned int limit, const std::string& source);

        void DebugShaderStageFlags(long shaderStageFlags, long validFlags, const std::string& source);
        void DebugBufferType(const BufferType bufferType, const BufferType compareType, const std::string& source);

        void WarnImproperVertices(const std::string& topologyName, unsigned int unusedVertices, const std::string& source);

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
