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

        void SetDrawMode(const DrawMode drawMode) override;

        /* ----- Hardware buffers ------ */

        void BindVertexBuffer(VertexBuffer& vertexBuffer) override;
        void UnbindVertexBuffer() override;

        void BindIndexBuffer(IndexBuffer& indexBuffer) override;
        void UnbindIndexBuffer() override;

        void BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index) override;
        void UnbindConstantBuffer(unsigned int index) override;

        /* --- Drawing --- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices) override;
        void DrawIndexed(unsigned int numVertices, int indexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances) override;
        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset) override;
        void DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset, unsigned int instanceOffset) override;

        /* ----- Shader ----- */

        void BindShaderProgram(ShaderProgram& shaderProgram) override;
        void UnbindShaderProgram() override;

        void DispatchCompute(const Gs::Vector3ui& threadGroupSize) override;

    private:

        RenderingProfiler&  profiler_;

        DrawMode            drawMode_ = DrawMode::Triangles;

};


} // /namespace LLGL


#endif



// ================================================================================
