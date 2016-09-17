/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(RenderContext& instance, RenderingProfiler& profiler) :
    instance_( instance ),
    profiler_( profiler )
{
    ShareWindow(instance);
}

void DbgRenderContext::Present()
{
    instance_.Present();
}

/* ----- Configuration ----- */

void DbgRenderContext::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    instance_.SetGraphicsAPIDependentState(state);
}

void DbgRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    instance_.SetVideoMode(videoModeDesc);
}

void DbgRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    instance_.SetVsync(vsyncDesc);
}

void DbgRenderContext::SetViewports(const std::vector<Viewport>& viewports)
{
    instance_.SetViewports(viewports);
}

void DbgRenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    instance_.SetScissors(scissors);
}

void DbgRenderContext::SetClearColor(const ColorRGBAf& color)
{
    instance_.SetClearColor(color);
}

void DbgRenderContext::SetClearDepth(float depth)
{
    instance_.SetClearDepth(depth);
}

void DbgRenderContext::SetClearStencil(int stencil)
{
    instance_.SetClearStencil(stencil);
}

void DbgRenderContext::ClearBuffers(long flags)
{
    instance_.ClearBuffers(flags);
}

/* ----- Hardware Buffers ------ */

void DbgRenderContext::SetVertexBuffer(VertexBuffer& vertexBuffer)
{
    instance_.SetVertexBuffer(vertexBuffer);
    profiler_.setVertexBuffer.Inc();
}

void DbgRenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    instance_.SetIndexBuffer(indexBuffer);
    profiler_.setIndexBuffer.Inc();
}

void DbgRenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot)
{
    instance_.SetConstantBuffer(constantBuffer, slot);
    profiler_.setConstantBuffer.Inc();
}

void DbgRenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    instance_.SetStorageBuffer(storageBuffer, slot);
    profiler_.setStorageBuffer.Inc();
}

void* DbgRenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    profiler_.mapStorageBuffer.Inc();
    return instance_.MapStorageBuffer(storageBuffer, access);
}

void DbgRenderContext::UnmapStorageBuffer()
{
    instance_.UnmapStorageBuffer();
}

/* ----- Textures ----- */

void DbgRenderContext::SetTexture(Texture& texture, unsigned int slot)
{
    instance_.SetTexture(texture, slot);
    profiler_.setTexture.Inc();
}

void DbgRenderContext::GenerateMips(Texture& texture)
{
    instance_.GenerateMips(texture);
}

/* ----- Sampler States ----- */

void DbgRenderContext::SetSampler(Sampler& sampler, unsigned int slot)
{
    instance_.SetSampler(sampler, slot);
    profiler_.setSampler.Inc();
}

/* ----- Render Targets ----- */

void DbgRenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    instance_.SetRenderTarget(renderTarget);
    profiler_.setRenderTarget.Inc();
}

void DbgRenderContext::UnsetRenderTarget()
{
    instance_.UnsetRenderTarget();
    profiler_.setRenderTarget.Inc();
}

/* ----- Pipeline States ----- */

void DbgRenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    instance_.SetGraphicsPipeline(graphicsPipeline);
    profiler_.setGraphicsPipeline.Inc();
}

void DbgRenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    instance_.SetComputePipeline(computePipeline);
    profiler_.setComputePipeline.Inc();
}

/* ----- Queries ----- */

void DbgRenderContext::BeginQuery(Query& query)
{
    instance_.BeginQuery(query);
}

void DbgRenderContext::EndQuery(Query& query)
{
    instance_.EndQuery(query);
}

bool DbgRenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    return instance_.QueryResult(query, result);
}

/* ----- Drawing ----- */

void DbgRenderContext::SetPrimitiveTopology(const PrimitiveTopology topology)
{
    topology_ = topology;
    instance_.SetPrimitiveTopology(topology);
}

void DbgRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    instance_.Draw(numVertices, firstVertex);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    instance_.DrawIndexed(numVertices, firstIndex);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    instance_.DrawIndexed(numVertices, firstIndex, vertexOffset);
    profiler_.RecordDrawCall(topology_, numVertices);
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    instance_.DrawInstanced(numVertices, firstVertex, numInstances);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    instance_.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    profiler_.RecordDrawCall(topology_, numVertices, numInstances);
}

/* ----- Compute ----- */

void DbgRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    instance_.DispatchCompute(threadGroupSize);
    profiler_.dispatchComputeCalls.Inc();
}

/* ----- Misc ----- */

void DbgRenderContext::SyncGPU()
{
    instance_.SyncGPU();
}


} // /namespace LLGL



// ================================================================================
