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


namespace LLGL
{


DbgRenderContext::DbgRenderContext(
    RenderContext& instance, RenderingProfiler* profiler, RenderingDebugger* debugger,
    const RenderingCaps& caps, const std::string& rendererName) :
        instance_   ( instance ),
        profiler_   ( profiler ),
        debugger_   ( debugger ),
        caps_       ( caps     )
{
    ShareWindow(instance);
    DetermineRenderer(rendererName);
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
    auto& vertexBufferDbg = LLGL_CAST(DbgVertexBuffer&, vertexBuffer);
    bindings_.vertexBuffer = (&vertexBufferDbg);

    instance_.SetVertexBuffer(vertexBufferDbg.instance);

    LLGL_DBG_PROFILER_DO(setVertexBuffer.Inc());
}

void DbgRenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    auto& indexBufferDbg = LLGL_CAST(DbgIndexBuffer&, indexBuffer);
    bindings_.indexBuffer = (&indexBufferDbg);

    instance_.SetIndexBuffer(indexBufferDbg.instance);

    LLGL_DBG_PROFILER_DO(setIndexBuffer.Inc());
}

void DbgRenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot)
{
    instance_.SetConstantBuffer(constantBuffer, slot);
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

void DbgRenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    instance_.SetStorageBuffer(storageBuffer, slot);
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

void* DbgRenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    LLGL_DBG_PROFILER_DO(mapStorageBuffer.Inc());
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
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

void DbgRenderContext::GenerateMips(Texture& texture)
{
    instance_.GenerateMips(texture);
}

/* ----- Sampler States ----- */

void DbgRenderContext::SetSampler(Sampler& sampler, unsigned int slot)
{
    instance_.SetSampler(sampler, slot);
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
    instance_.SetGraphicsPipeline(graphicsPipeline);
    LLGL_DBG_PROFILER_DO(setGraphicsPipeline.Inc());
}

void DbgRenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    instance_.SetComputePipeline(computePipeline);
    LLGL_DBG_PROFILER_DO(setComputePipeline.Inc());
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
    if (renderer_.isDirect3D)
    {
        switch (topology)
        {
            case PrimitiveTopology::LineLoop:
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology line loop", __FUNCTION__);
                break;
            case PrimitiveTopology::TriangleFan:
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology triangle fan", __FUNCTION__);
                break;
        }
    }

    topology_ = topology;
    instance_.SetPrimitiveTopology(topology);
}

void DbgRenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, 1, 0, __FUNCTION__));
    instance_.Draw(numVertices, firstVertex);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, 0, 0, __FUNCTION__));
    instance_.DrawIndexed(numVertices, firstIndex);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, 1, firstIndex, vertexOffset, 0, __FUNCTION__));
    instance_.DrawIndexed(numVertices, firstIndex, vertexOffset);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, 0, __FUNCTION__));
    instance_.DrawInstanced(numVertices, firstVertex, numInstances);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDraw(numVertices, firstVertex, numInstances, instanceOffset, __FUNCTION__));
    instance_.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, 0, 0, __FUNCTION__));
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, 0, __FUNCTION__));
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgRenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    DebugInstancing(__FUNCTION__);
    LLGL_DBG_DEBUGGER_DO(DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset, __FUNCTION__));
    instance_.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

/* ----- Compute ----- */

void DbgRenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    instance_.DispatchCompute(threadGroupSize);
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

static bool CompareSubStr(const std::string& lhs, const std::string& rhs)
{
    return (lhs.size() >= rhs.size() && lhs.substr(0, rhs.size()) == rhs);
}

void DbgRenderContext::DetermineRenderer(const std::string& rendererName)
{
    /* Determine renderer API by specified name */
    if (CompareSubStr(rendererName, "OpenGL"))
        renderer_.isOpenGL = true;
    else if (CompareSubStr(rendererName, "Direct3D"))
        renderer_.isDirect3D = true;
    else if (CompareSubStr(rendererName, "Vulkan"))
        renderer_.isVulkan = true;
}

void DbgRenderContext::DebugGraphicsPipelineSet(const std::string& source)
{
    //todo...
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
    DebugNumVertices(numVertices, source);
    DebugNumInstances(numInstances, instanceOffset, source);
}

void DbgRenderContext::DebugDrawIndexed(
    unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex,
    int vertexOffset, unsigned int instanceOffset, const std::string& source)
{
    DebugGraphicsPipelineSet(source);
    DebugVertexBufferSet(source);
    DebugIndexBufferSet(source);
    DebugNumVertices(numVertices, source);
    DebugNumInstances(numInstances, instanceOffset, source);
}

void DbgRenderContext::DebugInstancing(const std::string& source)
{
    if (!caps_.hasInstancing)
        ErrNotSupported("instancing", source);
}

void DbgRenderContext::ErrNotSupported(const std::string& featureName, const std::string& source)
{
    LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, featureName + " is not supported", source);
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
