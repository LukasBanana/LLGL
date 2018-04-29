/*
 * DbgCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgCommandBuffer.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"

#include "DbgRenderContext.h"
#include "DbgBuffer.h"
#include "DbgBufferArray.h"
#include "DbgTexture.h"
#include "DbgRenderTarget.h"
#include "DbgShaderProgram.h"
#include "DbgQuery.h"


namespace LLGL
{


DbgCommandBuffer::DbgCommandBuffer(
    CommandBuffer& instance, CommandBufferExt* instanceExt, RenderingProfiler* profiler, RenderingDebugger* debugger, const RenderingCaps& caps) :
        instance    { instance    },
        instanceExt { instanceExt },
        profiler_   { profiler    },
        debugger_   { debugger    },
        caps_       { caps        }
{
}

/* ----- Configuration ----- */

void DbgCommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    instance.SetGraphicsAPIDependentState(state);
}

/* ----- Viewport and Scissor ----- */

void DbgCommandBuffer::SetViewport(const Viewport& viewport)
{
    instance.SetViewport(viewport);
}

void DbgCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (!viewports)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "viewport array must not be a null pointer");
        if (numViewports == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no viewports are specified");
    }

    instance.SetViewports(numViewports, viewports);
}

void DbgCommandBuffer::SetScissor(const Scissor& scissor)
{
    instance.SetScissor(scissor);
}

void DbgCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (!scissors)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "scissor array must not be a null pointer");
        if (numScissors == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no scissor rectangles are specified");
    }

    instance.SetScissors(numScissors, scissors);
}

/* ----- Clear ----- */

void DbgCommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    instance.SetClearColor(color);
}

void DbgCommandBuffer::SetClearDepth(float depth)
{
    instance.SetClearDepth(depth);
}

void DbgCommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    instance.SetClearStencil(stencil);
}

void DbgCommandBuffer::Clear(long flags)
{
    instance.Clear(flags);
}

void DbgCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        for (std::uint32_t i = 0; i < numAttachments; ++i)
            DebugAttachmentClear(attachments[i]);
    }

    instance.ClearAttachments(numAttachments, attachments);
}

/* ----- Buffers ------ */

void DbgCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(buffer.GetType(), BufferType::Vertex);
    }

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    
    if (debugger_)
    {
        bindings_.vertexBufferStore[0]      = (&bufferDbg);
        bindings_.vertexBuffers             = bindings_.vertexBufferStore;
        bindings_.numVertexBuffers          = 1;
        bindings_.anyNonEmptyVertexBuffer   = (bufferDbg.elements > 0);
    }
    
    instance.SetVertexBuffer(bufferDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setVertexBuffer.Inc());
}

void DbgCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(bufferArray.GetType(), BufferType::Vertex);
    }

    auto& bufferArrayDbg = LLGL_CAST(DbgBufferArray&, bufferArray);
    
    if (debugger_)
    {
        bindings_.vertexBuffers         = bufferArrayDbg.buffers.data();
        bindings_.numVertexBuffers      = static_cast<std::uint32_t>(bufferArrayDbg.buffers.size());

        /* Check if all vertex buffers are empty */
        bindings_.anyNonEmptyVertexBuffer = false;
        for (auto buffer : bufferArrayDbg.buffers)
        {
            if (buffer->elements > 0)
            {
                bindings_.anyNonEmptyVertexBuffer = true;
                break;
            }
        }
    }
    
    instance.SetVertexBufferArray(bufferArrayDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setVertexBuffer.Inc());
}

void DbgCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(buffer.GetType(), BufferType::Index);
        bindings_.indexBuffer = (&bufferDbg);
    }

    instance.SetIndexBuffer(bufferDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setIndexBuffer.Inc());
}

/* ----- Constant Buffers ------ */

void DbgCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(buffer.GetType(), BufferType::Constant);
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }

    instanceExt->SetConstantBuffer(bufferDbg.instance, slot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

void DbgCommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(bufferArray.GetType(), BufferType::Constant);
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }

    instanceExt->SetConstantBufferArray(bufferArray, startSlot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setConstantBuffer.Inc());
}

/* ----- Storage Buffers ------ */

void DbgCommandBuffer::SetStorageBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(buffer.GetType(), BufferType::Storage);
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages | ShaderStageFlags::ReadOnlyResource);
    }

    instanceExt->SetStorageBuffer(bufferDbg.instance, slot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

void DbgCommandBuffer::SetStorageBufferArray(BufferArray& bufferArray, std::uint32_t startSlot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(bufferArray.GetType(), BufferType::Storage);
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }

    instanceExt->SetStorageBufferArray(bufferArray, startSlot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setStorageBuffer.Inc());
}

/* ----- Stream Output Buffers ------ */

void DbgCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(buffer.GetType(), BufferType::StreamOutput);
        bindings_.streamOutput = (&bufferDbg);
    }

    instance.SetStreamOutputBuffer(bufferDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setStreamOutputBuffer.Inc());
}

void DbgCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugBufferType(bufferArray.GetType(), BufferType::StreamOutput);
    }
    
    instance.SetStreamOutputBufferArray(bufferArray);
    
    LLGL_DBG_PROFILER_DO(setStreamOutputBuffer.Inc());
}

void DbgCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output is already busy");
        if (!bindings_.streamOutput)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "no stream-output buffer is bound");
        states_.streamOutputBusy = true;
    }

    instance.BeginStreamOutput(primitiveType);
}

void DbgCommandBuffer::EndStreamOutput()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (!states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output has not started");
        states_.streamOutputBusy = false;
    }

    instance.EndStreamOutput();
}

/* ----- Textures ----- */

void DbgCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }
    
    instanceExt->SetTexture(textureDbg.instance, slot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

void DbgCommandBuffer::SetTextureArray(TextureArray& textureArray, std::uint32_t startSlot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }
    
    instanceExt->SetTextureArray(textureArray, startSlot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setTexture.Inc());
}

/* ----- Sampler States ----- */

void DbgCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }
    
    instanceExt->SetSampler(sampler, slot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setSampler.Inc());
}

void DbgCommandBuffer::SetSamplerArray(SamplerArray& samplerArray, std::uint32_t startSlot, long shaderStageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugShaderStageFlags(shaderStageFlags, ShaderStageFlags::AllStages);
    }
    
    instanceExt->SetSamplerArray(samplerArray, startSlot, shaderStageFlags);
    
    LLGL_DBG_PROFILER_DO(setSampler.Inc());
}

/* ----- Resource View Heaps ----- */

void DbgCommandBuffer::SetGraphicsResourceViewHeap(ResourceViewHeap& resourceHeap, std::uint32_t startSlot)
{
    instance.SetGraphicsResourceViewHeap(resourceHeap, startSlot);
}

void DbgCommandBuffer::SetComputeResourceViewHeap(ResourceViewHeap& resourceHeap, std::uint32_t startSlot)
{
    instance.SetComputeResourceViewHeap(resourceHeap, startSlot);
}

/* ----- Render Targets ----- */

void DbgCommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    auto& renderTargetDbg = LLGL_CAST(DbgRenderTarget&, renderTarget);
    
    if (debugger_)
    {
        bindings_.renderContext = nullptr;
        bindings_.renderTarget  = &renderTargetDbg;
    }

    instance.SetRenderTarget(renderTargetDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

void DbgCommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextDbg = LLGL_CAST(DbgRenderContext&, renderContext);
    
    if (debugger_)
    {
        bindings_.renderContext = &renderContextDbg;
        bindings_.renderTarget  = nullptr;
    }

    instance.SetRenderTarget(renderContextDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setRenderTarget.Inc());
}

/* ----- Pipeline States ----- */

void DbgCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineDbg = LLGL_CAST(DbgGraphicsPipeline&, graphicsPipeline);

    if (debugger_)
    {
        bindings_.graphicsPipeline = (&graphicsPipelineDbg);
        if (auto shaderProgram = graphicsPipelineDbg.desc.shaderProgram)
        {
            auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, shaderProgram);
            bindings_.anyShaderAttributes = !(shaderProgramDbg->GetVertexLayout().attributes.empty());
        }
        else
            bindings_.anyShaderAttributes = false;
    }

    topology_ = graphicsPipelineDbg.desc.primitiveTopology;
    instance.SetGraphicsPipeline(graphicsPipelineDbg.instance);
    
    LLGL_DBG_PROFILER_DO(setGraphicsPipeline.Inc());
}

void DbgCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    if (debugger_)
        bindings_.computePipeline = (&computePipeline);
    
    instance.SetComputePipeline(computePipeline);
    
    LLGL_DBG_PROFILER_DO(setComputePipeline.Inc());
}

/* ----- Queries ----- */

void DbgCommandBuffer::BeginQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (queryDbg.state == DbgQuery::State::Busy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "query is already busy");
        queryDbg.state = DbgQuery::State::Busy;
    }

    instance.BeginQuery(queryDbg.instance);
}

void DbgCommandBuffer::EndQuery(Query& query)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (queryDbg.state != DbgQuery::State::Busy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "query has not started");
        queryDbg.state = DbgQuery::State::Ready;
    }

    instance.EndQuery(queryDbg.instance);
}

bool DbgCommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (queryDbg.state != DbgQuery::State::Ready)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "query result is not ready");
    }

    return instance.QueryResult(queryDbg.instance, result);
}

bool DbgCommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    auto& queryDbg = LLGL_CAST(DbgQuery&, query);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (queryDbg.state != DbgQuery::State::Ready)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "query result is not ready");
    }

    return instance.QueryPipelineStatisticsResult(queryDbg.instance, result);
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

void DbgCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugDraw(numVertices, firstVertex, 1, 0);
    }
    
    instance.Draw(numVertices, firstVertex);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugDrawIndexed(numVertices, 1, firstIndex, 0, 0);
    }
    
    instance.DrawIndexed(numVertices, firstIndex);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugDrawIndexed(numVertices, 1, firstIndex, vertexOffset, 0);
    }
    
    instance.DrawIndexed(numVertices, firstIndex, vertexOffset);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices));
}

void DbgCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugInstancing();
        DebugDraw(numVertices, firstVertex, numInstances, 0);
    }
    
    instance.DrawInstanced(numVertices, firstVertex, numInstances);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugInstancing();
        DebugOffsetInstancing();
        DebugDraw(numVertices, firstVertex, numInstances, instanceOffset);
    }
    
    instance.DrawInstanced(numVertices, firstVertex, numInstances, instanceOffset);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugInstancing();
        DebugDrawIndexed(numVertices, numInstances, firstIndex, 0, 0);
    }
    
    instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugInstancing();
        DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, 0);
    }
    
    instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugInstancing();
        DebugOffsetInstancing();
        DebugDrawIndexed(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    }
    
    instance.DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
    
    LLGL_DBG_PROFILER_DO(RecordDrawCall(topology_, numVertices, numInstances));
}

/* ----- Compute ----- */

void DbgCommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        if (groupSizeX * groupSizeY * groupSizeZ == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "thread group size has volume of 0 units");

        DebugComputePipelineSet();
        DebugThreadGroupLimit(groupSizeX, caps_.maxNumComputeShaderWorkGroups[0]);
        DebugThreadGroupLimit(groupSizeY, caps_.maxNumComputeShaderWorkGroups[1]);
        DebugThreadGroupLimit(groupSizeZ, caps_.maxNumComputeShaderWorkGroups[2]);
    }
    
    instance.Dispatch(groupSizeX, groupSizeY, groupSizeZ);
    
    LLGL_DBG_PROFILER_DO(dispatchComputeCalls.Inc());
}


/*
 * ======= Private: =======
 */

void DbgCommandBuffer::AssertCommandBufferExt(const char* funcName)
{
    if (!instanceExt)
        throw std::runtime_error("illegal function call for a non-extended command buffer: " + std::string(funcName));
}

void DbgCommandBuffer::DebugAttachmentClear(const AttachmentClear& attachment)
{
    if (bindings_.renderContext != nullptr)
    {
        if ((attachment.flags & ClearFlags::Color) != 0)
            DebugAttachmentLimit(attachment.colorAttachment, 1);
    }
    else if (auto renderTarget = bindings_.renderTarget)
    {
        if ((attachment.flags & ClearFlags::Color) != 0)
        {
            DebugAttachmentLimit(attachment.colorAttachment, renderTarget->GetNumColorAttachments());
            if ((attachment.flags & ClearFlags::DepthStencil) != 0)
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot have color attachment and depth-stencil attachment within a single AttachmentClear command");
        }
        else
        {
            if ((attachment.flags & ClearFlags::Depth) != 0)
            {
                if (!renderTarget->HasDepthAttachment())
                    LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot clear depth with render target that does not have a depth or depth-stencil attachment");
            }
            if ((attachment.flags & ClearFlags::Stencil) != 0)
            {
                if (!renderTarget->HasStencilAttachment())
                    LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot clear stencil with render target that does not have a stencil or depth-stencil attachment");
            }
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no render target is bound");
}

void DbgCommandBuffer::DebugGraphicsPipelineSet()
{
    if (!bindings_.graphicsPipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no graphics pipeline is bound");
}

void DbgCommandBuffer::DebugComputePipelineSet()
{
    if (!bindings_.computePipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no compute pipeline is bound");
}

void DbgCommandBuffer::DebugVertexBufferSet()
{
    if (bindings_.numVertexBuffers > 0)
    {
        for (std::uint32_t i = 0; i < bindings_.numVertexBuffers; ++i)
        {
            /* Check if buffer is initialized (ignore empty buffers) */
            auto buffer = bindings_.vertexBuffers[i];
            if (buffer->elements > 0 && !buffer->initialized)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized vertex buffer is bound at slot " + std::to_string(i));
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no vertex buffer is bound");
}

void DbgCommandBuffer::DebugIndexBufferSet()
{
    if (!bindings_.indexBuffer)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no index buffer is bound");
    else if (!bindings_.indexBuffer->initialized)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized index buffer is bound");
}

void DbgCommandBuffer::DebugVertexLayout()
{
    if (bindings_.graphicsPipeline && bindings_.numVertexBuffers > 0)
    {
        auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, bindings_.graphicsPipeline->desc.shaderProgram);
        const auto& vertexLayout = shaderProgramDbg->GetVertexLayout();

        /* Check if vertex layout is specified in active shader program */
        if (vertexLayout.bound)
            DebugVertexLayoutAttributes(vertexLayout.attributes, bindings_.vertexBuffers, bindings_.numVertexBuffers);
        else if (bindings_.anyNonEmptyVertexBuffer)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "unspecified vertex layout in shader program while bound vertex buffers are non-empty");
    }
}

void DbgCommandBuffer::DebugVertexLayoutAttributes(const std::vector<VertexAttribute>& shaderAttributes, DbgBuffer** vertexBuffers, std::uint32_t numVertexBuffers)
{
    /* Check if all vertex attributes are served by active vertex buffer(s) */
    std::size_t attribIndex = 0;

    for (std::uint32_t bufferIndex = 0; attribIndex < shaderAttributes.size() && bufferIndex < numVertexBuffers; ++bufferIndex)
    {
        /* Compare remaining shader attributes with next vertex buffer attributes */
        const auto& vertexFormatRhs = vertexBuffers[bufferIndex]->desc.vertexBuffer.format;

        for (std::size_t i = 0; i < vertexFormatRhs.attributes.size() && attribIndex < shaderAttributes.size(); ++i, ++attribIndex)
        {
            /* Compare current vertex attributes */
            const auto& attribLhs = shaderAttributes[attribIndex];
            const auto& attribRhs = vertexFormatRhs.attributes[i];

            if (attribLhs != attribRhs)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex layout mismatch between shader program and vertex buffer(s)");
        }
    }

    if (attribIndex < shaderAttributes.size())
        LLGL_DBG_ERROR(ErrorType::InvalidState, "not all vertex attributes in the shader pipeline are covered by the bound vertex buffer(s)");
}

void DbgCommandBuffer::DebugNumVertices(std::uint32_t numVertices)
{
    if (numVertices == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no vertices will be generated");

    switch (topology_)
    {
        case PrimitiveTopology::PointList:
            break;

        case PrimitiveTopology::LineList:
            if (numVertices % 2 != 0)
                WarnImproperVertices("line list", (numVertices % 2));
            break;

        case PrimitiveTopology::LineStrip:
            if (numVertices < 2)
                WarnImproperVertices("line strip", numVertices);
            break;

        case PrimitiveTopology::LineLoop:
            if (numVertices < 2)
                WarnImproperVertices("line loop", numVertices);
            break;

        case PrimitiveTopology::LineListAdjacency:
            if (numVertices % 2 != 0)
                WarnImproperVertices("line list adjacency", (numVertices % 2));
            break;

        case PrimitiveTopology::LineStripAdjacency:
            if (numVertices < 2)
                WarnImproperVertices("line strip adjacency", numVertices);
            break;

        case PrimitiveTopology::TriangleList:
            if (numVertices % 3 != 0)
                WarnImproperVertices("triangle list", (numVertices % 3));
            break;

        case PrimitiveTopology::TriangleStrip:
            if (numVertices < 3)
                WarnImproperVertices("triangle strip", numVertices);
            break;
        case PrimitiveTopology::TriangleFan:
            if (numVertices < 3)
                WarnImproperVertices("triangle fan", numVertices);
            break;

        case PrimitiveTopology::TriangleListAdjacency:
            if (numVertices % 3 != 0)
                WarnImproperVertices("triangle list adjacency", (numVertices % 3));
            break;

        case PrimitiveTopology::TriangleStripAdjacency:
            if (numVertices < 3)
                WarnImproperVertices("triangle strip adjacency", numVertices);
            break;

        default:
            if (topology_ >= PrimitiveTopology::Patches1 && topology_ <= PrimitiveTopology::Patches32)
            {
                auto numPatchVertices = static_cast<std::uint32_t>(topology_) - static_cast<std::uint32_t>(PrimitiveTopology::Patches1) + 1;
                if (numVertices % numPatchVertices != 0)
                    WarnImproperVertices("patches" + std::to_string(numPatchVertices), (numVertices % numPatchVertices));
            }
            break;
    }
}

void DbgCommandBuffer::DebugNumInstances(std::uint32_t numInstances, std::uint32_t instanceOffset)
{
    if (numInstances == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no instances will be generated");
}

void DbgCommandBuffer::DebugDraw(
    std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset)
{
    DebugGraphicsPipelineSet();
    DebugVertexBufferSet();
    DebugVertexLayout();
    DebugNumVertices(numVertices);
    DebugNumInstances(numInstances, instanceOffset);

    if (bindings_.numVertexBuffers > 0 && bindings_.anyShaderAttributes)
        DebugVertexLimit(numVertices + firstVertex, static_cast<std::uint32_t>(bindings_.vertexBuffers[0]->elements));
}

void DbgCommandBuffer::DebugDrawIndexed(
    std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset)
{
    DebugGraphicsPipelineSet();
    DebugVertexBufferSet();
    DebugIndexBufferSet();
    DebugVertexLayout();
    DebugNumVertices(numVertices);
    DebugNumInstances(numInstances, instanceOffset);

    if (bindings_.indexBuffer)
        DebugVertexLimit(numVertices + firstIndex, static_cast<std::uint32_t>(bindings_.indexBuffer->elements));
}

void DbgCommandBuffer::DebugInstancing()
{
    if (!caps_.hasInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("instancing");
}

void DbgCommandBuffer::DebugOffsetInstancing()
{
    if (!caps_.hasOffsetInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("offset-instancing");
}

void DbgCommandBuffer::DebugVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit)
{
    if (vertexCount > vertexLimit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "vertex count out of bounds (" + std::to_string(vertexCount) +
            " specified but limit is " + std::to_string(vertexLimit) + ")"
        );
    }
}

void DbgCommandBuffer::DebugThreadGroupLimit(std::uint32_t size, std::uint32_t limit)
{
    if (size > limit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "thread group size X is too large (" + std::to_string(size) +
            " specified but limit is " + std::to_string(limit) + ")"
        );
    }
}

void DbgCommandBuffer::DebugAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound)
{
    if (attachmentIndex >= attachmentUpperBound)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "color attachment index out of bounds (" + std::to_string(attachmentIndex) +
            " specified but upper bound is " + std::to_string(attachmentUpperBound) + ")"
        );
    }
}

void DbgCommandBuffer::DebugShaderStageFlags(long shaderStageFlags, long validFlags)
{
    if ((shaderStageFlags & validFlags) == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stage is specified");
    if ((shaderStageFlags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "unknown shader stage flag is specified");
}

void DbgCommandBuffer::DebugBufferType(const BufferType bufferType, const BufferType compareType)
{
    if (bufferType != compareType)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid buffer type");
}

void DbgCommandBuffer::WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices)
{
    std::string vertexSingularPlural = (unusedVertices > 1 ? "vertices" : "vertex");

    LLGL_DBG_WARN(
        WarningType::ImproperArgument,
        ("improper number of vertices for " + topologyName + " (" + std::to_string(unusedVertices) + " unused " + vertexSingularPlural + ")")
    );
}


} // /namespace LLGL



// ================================================================================
