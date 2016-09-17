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


DbgRenderContext::DbgRenderContext(RenderContext& instance) :
    instance_( instance )
{
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
}

void DbgRenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    instance_.SetIndexBuffer(indexBuffer);
}

void DbgRenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot)
{
    instance_.SetConstantBuffer(constantBuffer, slot);
}

void DbgRenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    instance_.SetStorageBuffer(storageBuffer, slot);
}

void* DbgRenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
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
}

void DbgRenderContext::GenerateMips(Texture& texture)
{
    instance_.GenerateMips(texture);
}

/* ----- Sampler States ----- */

void DbgRenderContext::SetSampler(Sampler& sampler, unsigned int slot)
{
    instance_.SetSampler(sampler, slot);
}

/* ----- Render Targets ----- */

void DbgRenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    instance_.SetRenderTarget(renderTarget);
}

void DbgRenderContext::UnsetRenderTarget()
{
    instance_.UnsetRenderTarget();
}

/* ----- Pipeline States ----- */

void DbgRenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    instance_.SetGraphicsPipeline(graphicsPipeline);
}

void DbgRenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    instance_.SetComputePipeline(computePipeline);
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
    instance_.SetPrimitiveTopology(topology);
}

void DbgRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    instance_.Draw(numVertices, firstVertex);
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    instance_.DrawIndexed(numVertices, firstIndex);
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    instance_.DrawIndexed(numVertices, firstIndex, vertexOffset);
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    instance_.DrawInstanced(numVertices, firstVertex, numInstances);
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    instance_.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void DbgRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    instance_.DispatchCompute(threadGroupSize);
}

/* ----- Misc ----- */

void DbgRenderContext::SyncGPU()
{
    instance_.SyncGPU();
}


} // /namespace LLGL



// ================================================================================
