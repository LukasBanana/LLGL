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

/* ----- Hardware buffers ------ */

void GLRenderContextProfiler::SetVertexBuffer(VertexBuffer& vertexBuffer)
{
    GLRenderContext::SetVertexBuffer(vertexBuffer);
    profiler_.setVertexBuffer.Inc();
}

void GLRenderContextProfiler::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    GLRenderContext::SetIndexBuffer(indexBuffer);
    profiler_.setIndexBuffer.Inc();
}

void GLRenderContextProfiler::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot)
{
    GLRenderContext::SetConstantBuffer(constantBuffer, slot);
    profiler_.setConstantBuffer.Inc();
}

void GLRenderContextProfiler::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    GLRenderContext::SetStorageBuffer(storageBuffer, slot);
    profiler_.setStorageBuffer.Inc();
}

void* GLRenderContextProfiler::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    profiler_.mapStorageBuffer.Inc();
    return GLRenderContext::MapStorageBuffer(storageBuffer, access);
}

/* ----- Textures ----- */

void GLRenderContextProfiler::SetTexture(Texture& texture, unsigned int slot)
{
    GLRenderContext::SetTexture(texture, slot);
    profiler_.setTexture.Inc();
}

/* ----- Sampler States ----- */

void GLRenderContextProfiler::SetSampler(Sampler& sampler, unsigned int slot)
{
    GLRenderContext::SetSampler(sampler, slot);
    profiler_.setSampler.Inc();
}

/* ----- Render Targets ----- */

void GLRenderContextProfiler::SetRenderTarget(RenderTarget& renderTarget)
{
    GLRenderContext::SetRenderTarget(renderTarget);
    profiler_.setRenderTarget.Inc();
}

void GLRenderContextProfiler::UnsetRenderTarget()
{
    GLRenderContext::UnsetRenderTarget();
    profiler_.setRenderTarget.Inc();
}

/* ----- Pipeline states ----- */

void GLRenderContextProfiler::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    GLRenderContext::SetGraphicsPipeline(graphicsPipeline);
    profiler_.setGraphicsPipeline.Inc();
}

void GLRenderContextProfiler::SetComputePipeline(ComputePipeline& computePipeline)
{
    GLRenderContext::SetComputePipeline(computePipeline);
    profiler_.setComputePipeline.Inc();
}

/* --- Drawing --- */

void GLRenderContextProfiler::SetPrimitiveTopology(const PrimitiveTopology topology)
{
    topology_ = topology;
    GLRenderContext::SetPrimitiveTopology(topology);
}

void GLRenderContextProfiler::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    GLRenderContext::Draw(numVertices, firstVertex);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void GLRenderContextProfiler::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    GLRenderContext::DrawIndexed(numVertices, firstIndex);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void GLRenderContextProfiler::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    GLRenderContext::DrawIndexed(numVertices, firstIndex, vertexOffset);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void GLRenderContextProfiler::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    GLRenderContext::DrawInstanced(numVertices, firstVertex, numInstances);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void GLRenderContextProfiler::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    GLRenderContext::DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void GLRenderContextProfiler::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    GLRenderContext::DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void GLRenderContextProfiler::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    GLRenderContext::DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void GLRenderContextProfiler::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    GLRenderContext::DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

/* ----- Compute ----- */

void GLRenderContextProfiler::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    GLRenderContext::DispatchCompute(threadGroupSize);
    profiler_.dispatchComputeCalls.Inc();
}


} // /namespace LLGL



// ================================================================================
