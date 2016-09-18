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

#include "DbgVertexBuffer.h"
#include "DbgIndexBuffer.h"
#include "DbgGraphicsPipeline.h"


namespace LLGL
{


class DbgRenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        DbgRenderContext(
            RenderContext& instance,
            RenderingProfiler* profiler,
            RenderingDebugger* debugger,
            const RenderingCaps& caps,
            const std::string& rendererName
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        void SetViewports(const std::vector<Viewport>& viewports) override;
        void SetScissors(const std::vector<Scissor>& scissors) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Hardware Buffers ------ */

        void SetVertexBuffer(VertexBuffer& vertexBuffer) override;
        void SetIndexBuffer(IndexBuffer& indexBuffer) override;
        void SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot) override;
        void SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot) override;

        void* MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access) override;
        void UnmapStorageBuffer() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot) override;

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

        /* ----- Drawing ----- */

        void SetPrimitiveTopology(const PrimitiveTopology topology) override;

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

        void DetermineRenderer(const std::string& rendererName);

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

        void WarnImproperVertices(const std::string& topologyName, unsigned int unusedVertices, const std::string& source);

        /* ----- Common objects ----- */

        RenderContext&          instance_;

        RenderingProfiler*      profiler_   = nullptr;
        RenderingDebugger*      debugger_   = nullptr;

        const RenderingCaps&    caps_;

        /* ----- Render states ----- */

        PrimitiveTopology       topology_   = PrimitiveTopology::TriangleList;

        struct Renderer
        {
            bool isOpenGL       = false;
            bool isDirect3D     = false;
            bool isVulkan       = false;
        }
        renderer_;

        struct Bindings
        {
            DbgVertexBuffer*        vertexBuffer        = nullptr;
            DbgIndexBuffer*         indexBuffer         = nullptr;
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
