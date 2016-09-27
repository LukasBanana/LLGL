/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"

#include "DbgConstantBuffer.h"
#include "DbgStorageBuffer.h"
#include "DbgTexture.h"
#include "DbgShaderProgram.h"
#include "DbgQuery.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(
    RenderContext& instance, RenderingProfiler* profiler, RenderingDebugger* debugger, const RenderingCaps& caps) :
        instance_   ( instance ),
        profiler_   ( profiler ),
        debugger_   ( debugger ),
        caps_       ( caps     )
{
    ShareWindowAndVideoMode(instance);
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
    RenderContext::SetVideoMode(videoModeDesc);
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
    auto& vertexBufferDbg = LLGL_CAST(DbgVertexBuffer&, vertexBuffer);
    bindings_.vertexBuffer = (&vertexBufferDbg);
    vertexLayout_.attributes = vertexBufferDbg.format.GetAttributes();
    {
        instance_.SetVertexBuffer(vertexBufferDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setVertexBuffer.Inc());
}

void DbgRenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    auto& indexBufferDbg = LLGL_CAST(DbgIndexBuffer&, indexBuffer);
    bindings_.indexBuffer = (&indexBufferDbg);
    {
        instance_.SetIndexBuffer(indexBufferDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setIndexBuffer.Inc());
}

void DbgRenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, __FUNCTION__);
    auto& constantBufferDbg = LLGL_CAST(DbgConstantBuffer&, constantBuffer);
    {
        instance_.SetConstantBuffer(constantBufferDbg.instance, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

void DbgRenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    auto& storageBufferDbg = LLGL_CAST(DbgStorageBuffer&, storageBuffer);
    {
        instance_.SetStorageBuffer(storageBufferDbg.instance, slot);
    }
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

void* DbgRenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    void* result = nullptr;
    auto& storageBufferDbg = LLGL_CAST(DbgStorageBuffer&, storageBuffer);
    {
        result = instance_.MapStorageBuffer(storageBufferDbg.instance, access);
    }
    LLGL_DBG_PROFILER_DO(mapStorageBuffer.Inc());
    return result;
}

void DbgRenderContext::UnmapStorageBuffer()
{
    instance_.UnmapStorageBuffer();
}

/* ----- Textures ----- */

void DbgRenderContext::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, __FUNCTION__);
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    {
        instance_.SetTexture(textureDbg.instance, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

void DbgRenderContext::GenerateMips(Texture& texture)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    {
        instance_.GenerateMips(textureDbg.instance);
    }
    textureDbg.mipLevels = NumMipLevels(textureDbg.size);
}

/* ----- Sampler States ----- */

void DbgRenderContext::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, __FUNCTION__);
    {
        instance_.SetSampler(sampler, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setSampler.Inc());
}

/* ----- Render Targets ----- */

void DbgRenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    instance_.SetRenderTarget(renderTarget);
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

void DbgRenderContext::UnsetRenderTarget()
{
    instance_.UnsetRenderTarget();
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

/* ----- Pipeline States ----- */

void DbgRenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineDbg = LLGL_CAST(DbgGraphicsPipeline&, graphicsPipeline);
    bindings_.graphicsPipeline = (&graphicsPipelineDbg);
    topology_ = graphicsPipelineDbg.desc.primitiveTopology;
    {
        instance_.SetGraphicsPipeline(graphicsPipelineDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setGraphicsPipeline.Inc());
}

void DbgRenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    bindings_.computePipeline = (&computePipeline);
    {
        instance_.SetComputePipeline(computePipeline);
    }
    LLGL_DBG_PROFILER_DO(setComputePipeline.Inc());
}

/* ----- Queries ----- */

void DbgRenderContext::BeginQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state == DbgQuery::State::Busy)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query is already busy");
    queryDbg.state = DbgQuery::State::Busy;

    instance_.BeginQuery(queryDbg.instance);
}

void DbgRenderContext::EndQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state != DbgQuery::State::Busy)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query has not started");
    queryDbg.state = DbgQuery::State::Ready;

    instance_.EndQuery(queryDbg.instance);
}

bool DbgRenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state != DbgQuery::State::Ready)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query result is not ready");

    return instance_.QueryResult(queryDbg.instance, result);
}

void DbgRenderContext::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);
    instance_.BeginRenderCondition(queryDbg.instance, mode);
}

void DbgRenderContext::EndRenderCondition()
{
    instance_.EndRenderCondition();
}

/* ----- Drawing ----- */

void DbgRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, 1, 0, __FUNCTION__));
    {
        instance_.Draw(numVertices, firstVertex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, 0, 0, __FUNCTION__));
    {
        instance_.DrawIndexed(numVertices, firstIndex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, vertexOffset, 0, __FUNCTION__));
    {
        instance_.DrawIndexed(numVertices, firstIndex, vertexOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, 0, __FUNCTION__));
    {
        instance_.DrawInstanced(numVertices, firstVertex, numInstances);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, instanceOffset, __FUNCTION__));
    {
        instance_.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, 0, 0, __FUNCTION__));
    {
        instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, 0, __FUNCTION__));
    {
        instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset, __FUNCTION__));
    {
        instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

/* ----- Compute ----- */

void DbgRenderContext::DebugThreadGroupLimit(unsigned int size, unsigned int limit, const std::string& source)
{
    if (size > limit)
    {
        LLGL_DBG_ERROR_HERE(
            ErrorType::InvalidArgument,
            "thread group size X is too large (" + std::to_string(size) +
            " specified but limit is " + std::to_string(limit) + ")"
        );
    }
}

void DbgRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    if (threadGroupSize.x*threadGroupSize.y*threadGroupSize.z == 0)
        LLGL_DBG_WARN_HERE(WarningType::PointlessOperation, "thread group size has volume of 0 units");

    DebugComputePipelineSet(__FUNCTION__);
    DebugThreadGroupLimit(threadGroupSize.x, caps_.maxNumComputeShaderWorkGroups.x, __FUNCTION__);
    DebugThreadGroupLimit(threadGroupSize.y, caps_.maxNumComputeShaderWorkGroups.y, __FUNCTION__);
    DebugThreadGroupLimit(threadGroupSize.z, caps_.maxNumComputeShaderWorkGroups.z, __FUNCTION__);
    {
        instance_.DispatchCompute(threadGroupSize);
    }
    LLGL_DBG_PROFILER_DO(dispatchComputeCalls.Inc());
}

/* ----- Misc ----- */

void DbgRenderContext::SyncGPU()
{
    instance_.SyncGPU();
}


/*
 * ======= Private: =======
 */

void DbgRenderContext::DebugGraphicsPipelineSet(const std::string& source)
{
    if (!bindings_.graphicsPipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no graphics pipeline is bound", source);
}

void DbgRenderContext::DebugComputePipelineSet(const std::string& source)
{
    if (!bindings_.computePipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no compute pipeline is bound", source);
}

void DbgRenderContext::DebugVertexBufferSet(const std::string& source)
{
    if (!bindings_.vertexBuffer)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no vertex buffer is bound", source);
    else if (!bindings_.vertexBuffer->initialized)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized vertex buffer is bound", source);
}

void DbgRenderContext::DebugIndexBufferSet(const std::string& source)
{
    if (!bindings_.indexBuffer)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no index buffer is bound", source);
    else if (!bindings_.indexBuffer->initialized)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized index buffer is bound", source);
}

void DbgRenderContext::DebugVertexLayout(const std::string& source)
{
    if (bindings_.graphicsPipeline)
    {
        auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, bindings_.graphicsPipeline->desc.shaderProgram);
        const auto& vertexLayout = shaderProgramDbg->GetVertexLayout();

        /* Check if vertex layout is specified in active shader program */
        if (vertexLayout.bound)
        {
            /* Check if all vertex attributes are served by active vertex buffer(s) */
            if (vertexLayout.attributes != vertexLayout_.attributes)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex layout mismatch between shader program and vertex buffer(s)", source);
        }
        else
            LLGL_DBG_ERROR(ErrorType::InvalidState, "unspecified vertex layout in shader program", source);
    }
}

void DbgRenderContext::DebugNumVertices(unsigned int numVertices, const std::string& source)
{
    if (numVertices == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no vertices will be generated", source);

    switch (topology_)
    {
        case PrimitiveTopology::PointList:
            break;

        case PrimitiveTopology::LineList:
            if (numVertices % 2 != 0)
                WarnImproperVertices("line list", (numVertices % 2), source);
            break;

        case PrimitiveTopology::LineStrip:
            if (numVertices < 2)
                WarnImproperVertices("line strip", numVertices, source);
            break;

        case PrimitiveTopology::LineLoop:
            if (numVertices < 2)
                WarnImproperVertices("line loop", numVertices, source);
            break;

        case PrimitiveTopology::LineListAdjacency:
            if (numVertices % 2 != 0)
                WarnImproperVertices("line list adjacency", (numVertices % 2), source);
            break;

        case PrimitiveTopology::LineStripAdjacency:
            if (numVertices < 2)
                WarnImproperVertices("line strip adjacency", numVertices, source);
            break;

        case PrimitiveTopology::TriangleList:
            if (numVertices % 3 != 0)
                WarnImproperVertices("triangle list", (numVertices % 3), source);
            break;

        case PrimitiveTopology::TriangleStrip:
            if (numVertices < 3)
                WarnImproperVertices("triangle strip", numVertices, source);
            break;
        case PrimitiveTopology::TriangleFan:
            if (numVertices < 3)
                WarnImproperVertices("triangle fan", numVertices, source);
            break;

        case PrimitiveTopology::TriangleListAdjacency:
            if (numVertices % 3 != 0)
                WarnImproperVertices("triangle list adjacency", (numVertices % 3), source);
            break;

        case PrimitiveTopology::TriangleStripAdjacency:
            if (numVertices < 3)
                WarnImproperVertices("triangle strip adjacency", numVertices, source);
            break;

        default:
            if (topology_ >= PrimitiveTopology::Patches1 && topology_ <= PrimitiveTopology::Patches32)
            {
                auto numPatchVertices = static_cast<unsigned int>(topology_) - static_cast<unsigned int>(PrimitiveTopology::Patches1) + 1;
                if (numVertices % numPatchVertices != 0)
                    WarnImproperVertices("patches" + std::to_string(numPatchVertices), (numVertices % numPatchVertices), source);
            }
            break;
    }
}

void DbgRenderContext::DebugNumInstances(unsigned int numInstances, unsigned int instanceOffset, const std::string& source)
{
    if (numInstances == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no instances will be generated", source);
}

void DbgRenderContext::DebugDraw(
    unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances,
    unsigned int instanceOffset, const std::string& source)
{
    DebugGraphicsPipelineSet(source);
    DebugVertexBufferSet(source);
    DebugVertexLayout(source);
    DebugNumVertices(numVertices, source);
    DebugNumInstances(numInstances, instanceOffset, source);

    if (bindings_.vertexBuffer)
        DebugVertexLimit(numVertices + firstVertex, static_cast<unsigned int>(bindings_.vertexBuffer->elements), source);
}

void DbgRenderContext::DebugDrawIndexed(
    unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex,
    int vertexOffset, unsigned int instanceOffset, const std::string& source)
{
    DebugGraphicsPipelineSet(source);
    DebugVertexBufferSet(source);
    DebugIndexBufferSet(source);
    DebugVertexLayout(source);
    DebugNumVertices(numVertices, source);
    DebugNumInstances(numInstances, instanceOffset, source);

    if (bindings_.indexBuffer)
        DebugVertexLimit(numVertices + firstIndex, static_cast<unsigned int>(bindings_.indexBuffer->elements), source);
}

void DbgRenderContext::DebugInstancing(const std::string& source)
{
    if (!caps_.hasInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("instancing", source);
}

void DbgRenderContext::DebugVertexLimit(unsigned int vertexCount, unsigned int vertexLimit, const std::string& source)
{
    if (vertexCount > vertexLimit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "vertex index out of bounds (" + std::to_string(vertexCount) +
            " specified but limit is " + std::to_string(vertexLimit) + ")",
            source
        );
    }
}

void DbgRenderContext::DebugShaderStageFlags(long shaderStageFlags, const std::string& source)
{
    if ((shaderStageFlags & ShaderStageFlags::AllStages) == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stage is specified", source);
    if ((shaderStageFlags & (~ShaderStageFlags::AllStages)) != 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "unknown shader stage flag is specified", source);
}

void DbgRenderContext::WarnImproperVertices(const std::string& topologyName, unsigned int unusedVertices, const std::string& source)
{
    std::string vertexSingularPlural = (unusedVertices > 1 ? "vertices" : "vertex");

    LLGL_DBG_WARN(
        WarningType::ImproperArgument,
        ("improper number of vertices for " + topologyName + " (" + std::to_string(unusedVertices) + " unused " + vertexSingularPlural + ")"),
        source
    );
}


} // /namespace LLGL



// ================================================================================
