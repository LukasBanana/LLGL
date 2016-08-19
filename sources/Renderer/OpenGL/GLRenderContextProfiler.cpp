/*
 * GLRenderContextProfiler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderContextProfiler.h"


namespace LLGL
{


GLRenderContextProfiler::GLRenderContextProfiler(
    GLRenderSystem& renderSystem,
    const RenderContextDescriptor& desc,
    const std::shared_ptr<Window>& window,
    GLRenderContext* sharedRenderContext,
    RenderingProfiler& profiler) :
        GLRenderContext( renderSystem, desc, window, sharedRenderContext ),
        profiler_( profiler )
{
}

/* ----- Configuration ----- */

void GLRenderContextProfiler::SetClearColor(const ColorRGBAf& color)
{
    GLRenderContext::SetClearColor(color);
}

void GLRenderContextProfiler::SetClearDepth(float depth)
{
    GLRenderContext::SetClearDepth(depth);
}

void GLRenderContextProfiler::SetClearStencil(int stencil)
{
    GLRenderContext::SetClearStencil(stencil);
}

void GLRenderContextProfiler::ClearBuffers(long flags)
{
    GLRenderContext::ClearBuffers(flags);
}

void GLRenderContextProfiler::SetDrawMode(const DrawMode drawMode)
{
    drawMode_ = drawMode;
    GLRenderContext::SetDrawMode(drawMode);
}

/* ----- Hardware buffers ------ */

void GLRenderContextProfiler::BindVertexBuffer(VertexBuffer& vertexBuffer)
{
    GLRenderContext::BindVertexBuffer(vertexBuffer);
    profiler_.bindVertexBuffer.Inc();
}

void GLRenderContextProfiler::UnbindVertexBuffer()
{
    GLRenderContext::UnbindVertexBuffer();
    profiler_.bindVertexBuffer.Inc();
}

void GLRenderContextProfiler::BindIndexBuffer(IndexBuffer& indexBuffer)
{
    GLRenderContext::BindIndexBuffer(indexBuffer);
    profiler_.bindIndexBuffer.Inc();
}

void GLRenderContextProfiler::UnbindIndexBuffer()
{
    GLRenderContext::UnbindIndexBuffer();
    profiler_.bindIndexBuffer.Inc();
}

void GLRenderContextProfiler::BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index)
{
    GLRenderContext::BindConstantBuffer(constantBuffer, index);
    profiler_.bindConstantBuffer.Inc();
}

void GLRenderContextProfiler::UnbindConstantBuffer(unsigned int index)
{
    GLRenderContext::UnbindConstantBuffer(index);
    profiler_.bindConstantBuffer.Inc();
}

/* ----- Textures ----- */

void GLRenderContextProfiler::BindTexture(Texture& texture, unsigned int layer)
{
    GLRenderContext::BindTexture(texture, layer);
    profiler_.bindTexture.Inc();
}

void GLRenderContextProfiler::UnbindTexture(unsigned int layer)
{
    GLRenderContext::UnbindTexture(layer);
    profiler_.bindTexture.Inc();
}

/* ----- Pipeline states ----- */

void GLRenderContextProfiler::BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    GLRenderContext::BindGraphicsPipeline(graphicsPipeline);
    profiler_.bindGraphicsPipeline.Inc();
}

/*void GLRenderContextProfiler::BindComputePipeline(ComputePipeline& computePipeline)
{
}*/

/* --- Drawing --- */

void GLRenderContextProfiler::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    GLRenderContext::Draw(numVertices, firstVertex);
    profiler_.RecordDrawCall(drawMode_, numVertices);
}

void GLRenderContextProfiler::DrawIndexed(unsigned int numVertices)
{
    GLRenderContext::DrawIndexed(numVertices);
    profiler_.RecordDrawCall(drawMode_, numVertices);
}

void GLRenderContextProfiler::DrawIndexed(unsigned int numVertices, int indexOffset)
{
    GLRenderContext::DrawIndexed(numVertices, indexOffset);
    profiler_.RecordDrawCall(drawMode_, numVertices);
}

void GLRenderContextProfiler::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    GLRenderContext::DrawInstanced(numVertices, firstVertex, numInstances);
    profiler_.RecordDrawCall(drawMode_, numVertices);
}

void GLRenderContextProfiler::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    GLRenderContext::DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    profiler_.RecordDrawCall(drawMode_, numVertices);
}

void GLRenderContextProfiler::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances)
{
    GLRenderContext::DrawInstancedIndexed(numVertices, numInstances);
    profiler_.RecordDrawCall(drawMode_, numVertices, numInstances);
}

void GLRenderContextProfiler::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset)
{
    GLRenderContext::DrawInstancedIndexed(numVertices, numInstances, indexOffset);
    profiler_.RecordDrawCall(drawMode_, numVertices, numInstances);
}

void GLRenderContextProfiler::DrawInstancedIndexed(unsigned int numVertices, unsigned int numInstances, int indexOffset, unsigned int instanceOffset)
{
    GLRenderContext::DrawInstancedIndexed(numVertices, numInstances, indexOffset, instanceOffset);
    profiler_.RecordDrawCall(drawMode_, numVertices, numInstances);
}

/* ----- Compute ----- */

void GLRenderContextProfiler::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    GLRenderContext::DispatchCompute(threadGroupSize);
    profiler_.dispatchComputeCalls.Inc();
}


} // /namespace LLGL



// ================================================================================
