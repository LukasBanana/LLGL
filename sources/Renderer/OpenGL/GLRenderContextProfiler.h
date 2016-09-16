/*
 * GLRenderContextProfiler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_CONTEXT_PROFILER_H__
#define __LLGL_GL_RENDER_CONTEXT_PROFILER_H__


#include "GLRenderContext.h"
#include <LLGL/RenderingProfiler.h>


namespace LLGL
{


class GLRenderContextProfiler : public GLRenderContext
{

    public:

        /* ----- Common ----- */

        GLRenderContextProfiler(
            GLRenderSystem& renderSystem,
            const RenderContextDescriptor& desc,
            const std::shared_ptr<Window>& window,
            GLRenderContext* sharedRenderContext,
            RenderingProfiler& profiler
        );

        /* ----- Configuration ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Hardware buffers ------ */

        void SetVertexBuffer(VertexBuffer& vertexBuffer) override;
        void SetIndexBuffer(IndexBuffer& indexBuffer) override;
        void SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot) override;
        void SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot) override;

        void* MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access) override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void UnsetRenderTarget() override;

        /* ----- Pipeline states ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* --- Drawing --- */

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

    private:

        RenderingProfiler&  profiler_;

        PrimitiveTopology   topology_ = PrimitiveTopology::TriangleList;

};


} // /namespace LLGL


#endif



// ================================================================================
