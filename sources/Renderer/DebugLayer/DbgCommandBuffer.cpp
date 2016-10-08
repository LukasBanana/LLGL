/*
 * DbgCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgCommandBuffer.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"

#include "DbgRenderContext.h"
#include "DbgBuffer.h"
#include "DbgTexture.h"
#include "DbgRenderTarget.h"
#include "DbgShaderProgram.h"
#include "DbgQuery.h"


namespace LLGL
{


DbgCommandBuffer::DbgCommandBuffer(
    CommandBuffer& instance, RenderingProfiler* profiler, RenderingDebugger* debugger, const RenderingCaps& caps) :
        instance    ( instance ),
        profiler_   ( profiler ),
        debugger_   ( debugger ),
        caps_       ( caps     )
{
}

/* ----- Configuration ----- */

void DbgCommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    instance.SetGraphicsAPIDependentState(state);
}

void DbgCommandBuffer::SetViewport(const Viewport& viewport)
{
    instance.SetViewport(viewport);
}

void DbgCommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    if (!viewportArray)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "viewport array must not be a null pointer");
    if (numViewports == 0)
        LLGL_DBG_WARN_HERE(WarningType::PointlessOperation, "no viewports are specified");
    instance.SetViewportArray(numViewports, viewportArray);
}

void DbgCommandBuffer::SetScissor(const Scissor& scissor)
{
    instance.SetScissor(scissor);
}

void DbgCommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
{
    if (!scissorArray)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "scissor array must not be a null pointer");
    if (numScissors == 0)
        LLGL_DBG_WARN_HERE(WarningType::PointlessOperation, "no scissor rectangles are specified");
    instance.SetScissorArray(numScissors, scissorArray);
}

void DbgCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    instance.SetClearColor(color);
}

void DbgCommandBuffer::SetClearDepth(float depth)
{
    instance.SetClearDepth(depth);
}

void DbgCommandBuffer::SetClearStencil(int stencil)
{
    instance.SetClearStencil(stencil);
}

void DbgCommandBuffer::ClearBuffers(long flags)
{
    instance.ClearBuffers(flags);
}

/* ----- Buffers ------ */

void DbgCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    DebugBufferType(buffer.GetType(), BufferType::Vertex, __FUNCTION__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    
    if (debugger_)
    {
        bindings_.vertexBuffer = (&bufferDbg);
        vertexFormat_ = bufferDbg.desc.vertexBuffer.vertexFormat;
    }
    
    instance.SetVertexBuffer(bufferDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setVertexBuffer.Inc());
}

void DbgCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    //todo...
    instance.SetVertexBufferArray(bufferArray);
}

void DbgCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    DebugBufferType(buffer.GetType(), BufferType::Index, __FUNCTION__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    bindings_.indexBuffer = (&bufferDbg);
    {
        instance.SetIndexBuffer(bufferDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setIndexBuffer.Inc());
}

void DbgCommandBuffer::SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    DebugBufferType(buffer.GetType(), BufferType::Constant, __FUNCTION__);
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages, __FUNCTION__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    {
        instance.SetConstantBuffer(bufferDbg.instance, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

void DbgCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags)
{
    DebugBufferType(bufferArray.GetType(), BufferType::Constant, __FUNCTION__);
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages, __FUNCTION__);
    {
        instance.SetConstantBufferArray(bufferArray, startSlot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

void DbgCommandBuffer::SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    DebugBufferType(buffer.GetType(), BufferType::Storage, __FUNCTION__);
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages | ShaderStageFlags::ReadOnlyResource, __FUNCTION__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    {
        instance.SetStorageBuffer(bufferDbg.instance, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

void DbgCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    DebugBufferType(buffer.GetType(), BufferType::StreamOutput, __FUNCTION__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    {
        instance.SetStreamOutputBuffer(bufferDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

/* ----- Textures ----- */

void DbgCommandBuffer::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages, __FUNCTION__);
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    {
        instance.SetTexture(textureDbg.instance, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

void DbgCommandBuffer::SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages, __FUNCTION__);
    {
        instance.SetTextureArray(textureArray, startSlot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

/* ----- Sampler States ----- */

void DbgCommandBuffer::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages, __FUNCTION__);
    {
        instance.SetSampler(sampler, slot, shaderStageFlags);
    }
    LLGL_DBG_PROFILER_DO(setSampler.Inc());
}

/* ----- Render Targets ----- */

void DbgCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    auto& renderTargetDbg = LLGL_CAST(DbgRenderTarget&, renderTarget);
    {
        instance.SetRenderTarget(renderTargetDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

void DbgCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextDbg = LLGL_CAST(DbgRenderContext&, renderContext);
    {
        instance.SetRenderTarget(renderContextDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

/* ----- Pipeline States ----- */

void DbgCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineDbg = LLGL_CAST(DbgGraphicsPipeline&, graphicsPipeline);
    bindings_.graphicsPipeline = (&graphicsPipelineDbg);
    topology_ = graphicsPipelineDbg.desc.primitiveTopology;
    {
        instance.SetGraphicsPipeline(graphicsPipelineDbg.instance);
    }
    LLGL_DBG_PROFILER_DO(setGraphicsPipeline.Inc());
}

void DbgCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    bindings_.computePipeline = (&computePipeline);
    {
        instance.SetComputePipeline(computePipeline);
    }
    LLGL_DBG_PROFILER_DO(setComputePipeline.Inc());
}

/* ----- Queries ----- */

void DbgCommandBuffer::BeginQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state == DbgQuery::State::Busy)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query is already busy");
    queryDbg.state = DbgQuery::State::Busy;

    instance.BeginQuery(queryDbg.instance);
}

void DbgCommandBuffer::EndQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state != DbgQuery::State::Busy)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query has not started");
    queryDbg.state = DbgQuery::State::Ready;

    instance.EndQuery(queryDbg.instance);
}

bool DbgCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    /* Validate query state */
    if (queryDbg.state != DbgQuery::State::Ready)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidState, "query result is not ready");

    return instance.QueryResult(queryDbg.instance, result);
}

void DbgCommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);
    instance.BeginRenderCondition(queryDbg.instance, mode);
}

void DbgCommandBuffer::EndRenderCondition()
{
    instance.EndRenderCondition();
}

/* ----- Drawing ----- */

void DbgCommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, 1, 0, __FUNCTION__));
    {
        instance.Draw(numVertices, firstVertex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, 0, 0, __FUNCTION__));
    {
        instance.DrawIndexed(numVertices, firstIndex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, vertexOffset, 0, __FUNCTION__));
    {
        instance.DrawIndexed(numVertices, firstIndex, vertexOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, 0, __FUNCTION__));
    {
        instance.DrawInstanced(numVertices, firstVertex, numInstances);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, instanceOffset, __FUNCTION__));
    {
        instance.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, 0, 0, __FUNCTION__));
    {
        instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, 0, __FUNCTION__));
    {
        instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset, __FUNCTION__));
    {
        instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    }
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

/* ----- Compute ----- */

void DbgCommandBuffer::DebugThreadGroupLimit(unsigned int size, unsigned int limit, const std::string& source)
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

void DbgCommandBuffer::DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    if (groupSizeX * groupSizeY * groupSizeZ == 0)
        LLGL_DBG_WARN_HERE(WarningType::PointlessOperation, "thread group size has volume of 0 units");

    DebugComputePipelineSet(__FUNCTION__);
    DebugThreadGroupLimit(groupSizeX, caps_.maxNumComputeShaderWorkGroups.x, __FUNCTION__);
    DebugThreadGroupLimit(groupSizeY, caps_.maxNumComputeShaderWorkGroups.y, __FUNCTION__);
    DebugThreadGroupLimit(groupSizeZ, caps_.maxNumComputeShaderWorkGroups.z, __FUNCTION__);
    {
        instance.DispatchCompute(groupSizeX, groupSizeY, groupSizeZ);
    }
    LLGL_DBG_PROFILER_DO(dispatchComputeCalls.Inc());
}

/* ----- Misc ----- */

void DbgCommandBuffer::SyncGPU()
{
    instance.SyncGPU();
}


/*
 * ======= Private: =======
 */

void DbgCommandBuffer::DebugGraphicsPipelineSet(const std::string& source)
{
    if (!bindings_.graphicsPipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no graphics pipeline is bound", source);
}

void DbgCommandBuffer::DebugComputePipelineSet(const std::string& source)
{
    if (!bindings_.computePipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no compute pipeline is bound", source);
}

void DbgCommandBuffer::DebugVertexBufferSet(const std::string& source)
{
    if (!bindings_.vertexBuffer)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no vertex buffer is bound", source);
    else if (!bindings_.vertexBuffer->initialized)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized vertex buffer is bound", source);
}

void DbgCommandBuffer::DebugIndexBufferSet(const std::string& source)
{
    if (!bindings_.indexBuffer)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no index buffer is bound", source);
    else if (!bindings_.indexBuffer->initialized)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized index buffer is bound", source);
}

void DbgCommandBuffer::DebugVertexLayout(const std::string& source)
{
    if (bindings_.graphicsPipeline)
    {
        auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, bindings_.graphicsPipeline->desc.shaderProgram);
        const auto& vertexLayout = shaderProgramDbg->GetVertexLayout();

        /* Check if vertex layout is specified in active shader program */
        if (vertexLayout.bound)
        {
            /* Check if all vertex attributes are served by active vertex buffer(s) */
            if (vertexLayout.attributes != vertexFormat_.attributes)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex layout mismatch between shader program and vertex buffer(s)", source);
        }
        else
            LLGL_DBG_ERROR(ErrorType::InvalidState, "unspecified vertex layout in shader program", source);
    }
}

void DbgCommandBuffer::DebugNumVertices(unsigned int numVertices, const std::string& source)
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

void DbgCommandBuffer::DebugNumInstances(unsigned int numInstances, unsigned int instanceOffset, const std::string& source)
{
    if (numInstances == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no instances will be generated", source);
}

void DbgCommandBuffer::DebugDraw(
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

void DbgCommandBuffer::DebugDrawIndexed(
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

void DbgCommandBuffer::DebugInstancing(const std::string& source)
{
    if (!caps_.hasInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("instancing", source);
}

void DbgCommandBuffer::DebugVertexLimit(unsigned int vertexCount, unsigned int vertexLimit, const std::string& source)
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

void DbgCommandBuffer::DebugShaderStageFlags(long shaderStageFlags, long validFlags, const std::string& source)
{
    if ((shaderStageFlags & validFlags) == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stage is specified", source);
    if ((shaderStageFlags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "unknown shader stage flag is specified", source);
}

void DbgCommandBuffer::DebugBufferType(const BufferType bufferType, const BufferType compareType, const std::string& source)
{
    if (bufferType != compareType)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid buffer type", source);
}

void DbgCommandBuffer::WarnImproperVertices(const std::string& topologyName, unsigned int unusedVertices, const std::string& source)
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
