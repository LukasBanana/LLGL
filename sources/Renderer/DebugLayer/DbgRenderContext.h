/*
 * DbgRenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_RENDER_CONTEXT_H__
#define __LLGL_DBG_RENDER_CONTEXT_H__


#include <LLGL/RenderContext.h>
#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>

#include "DbgGraphicsPipeline.h"


namespace LLGL
{


class DbgBuffer;

class DbgRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        DbgRenderContext(
            RenderContext& instance,
            RenderingProfiler* profiler,
            RenderingDebugger* debugger,
            const RenderingCaps& caps
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewportArray(unsigned int numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(unsigned int numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Hardware Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;
        void SetIndexBuffer(Buffer& buffer) override;
        void SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBuffer(Buffer& buffer, unsigned int slot) override;

        void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void UnsetRenderTarget() override;

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

        void DispatchCompute(const Gs::Vector3ui& threadGroupSize) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

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

        void DebugShaderStageFlags(long shaderStageFlags, const std::string& source);
        void DebugBufferType(const Buffer& buffer, const BufferType type, const std::string& source);

        void WarnImproperVertices(const std::string& topologyName, unsigned int unusedVertices, const std::string& source);

        /* ----- Common objects ----- */

        RenderContext&          instance_;

        RenderingProfiler*      profiler_   = nullptr;
        RenderingDebugger*      debugger_   = nullptr;

        const RenderingCaps&    caps_;

        /* ----- Render states ----- */

        PrimitiveTopology       topology_   = PrimitiveTopology::TriangleList;

        struct Bindings
        {
            DbgBuffer*              vertexBuffer        = nullptr;
            DbgBuffer*              indexBuffer         = nullptr;
            DbgGraphicsPipeline*    graphicsPipeline    = nullptr;
            ComputePipeline*        computePipeline     = nullptr;
        }
        bindings_;

        struct VertexLayout
        {
            std::vector<LLGL::VertexAttribute> attributes;
        }
        vertexLayout_;

        /*struct MetaInfo
        {
            bool viewportVisible = true;
        }
        metaInfo_;*/

};


} // /namespace LLGL


#endif



// ================================================================================
