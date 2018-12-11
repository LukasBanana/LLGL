/*
 * DbgCommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
#include "DbgQueryHeap.h"
#include "DbgComputePipeline.h"

#include <LLGL/RenderingDebugger.h>
#include <LLGL/IndirectCommandArgs.h>
#include <algorithm>


namespace LLGL
{


DbgCommandBuffer::DbgCommandBuffer(
    CommandBuffer&                  instance,
    CommandBufferExt*               instanceExt,
    RenderingDebugger*              debugger,
    const RenderingCapabilities&    caps) :
        instance      { instance      },
        instanceExt   { instanceExt   },
        debugger_     { debugger      },
        features_     { caps.features },
        limits_       { caps.limits   }
{
}

/* ----- Encoding ----- */

void DbgCommandBuffer::Begin()
{
    ResetFrameProfile();

    if (debugger_)
        EnableRecording(true);

    instance.Begin();

    profile_.commandBufferEncodings++;
}

void DbgCommandBuffer::End()
{
    if (debugger_)
        EnableRecording(false);
    instance.End();
}

void DbgCommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
    }

    instance.UpdateBuffer(dstBufferDbg.instance, dstOffset, data, dataSize);

    profile_.bufferUpdates++;
}

void DbgCommandBuffer::CopyBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, Buffer& srcBuffer, std::uint64_t srcOffset, std::uint64_t size)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);
    auto& srcBufferDbg = LLGL_CAST(DbgBuffer&, srcBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
    }

    instance.CopyBuffer(dstBufferDbg.instance, dstOffset, srcBufferDbg.instance, srcOffset, size);

    profile_.bufferCopies++;
}

/* ----- Configuration ----- */

void DbgCommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    instance.SetGraphicsAPIDependentState(stateDesc, stateDescSize);
}

/* ----- Viewport and Scissor ----- */

void DbgCommandBuffer::SetViewport(const Viewport& viewport)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateViewport(viewport);
    }

    instance.SetViewport(viewport);
}

void DbgCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        AssertRecording();

        /* Validate all viewports in array */
        if (viewports)
        {
            for (std::uint32_t i = 0; i < numViewports; ++i)
                ValidateViewport(viewports[i]);
        }
        else
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "viewport array must not be a null pointer");

        /* Validate array size */
        if (numViewports == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no viewports are specified");
        else if (numViewports > limits_.maxViewports)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "viewport array exceeded maximal number of viewports (" + std::to_string(numViewports) +
                " specified but limit is " + std::to_string(limits_.maxViewports) + ")"
            );
        }
    }

    instance.SetViewports(numViewports, viewports);
}

void DbgCommandBuffer::SetScissor(const Scissor& scissor)
{
    LLGL_DBG_SOURCE;
    AssertRecording();
    instance.SetScissor(scissor);
}

void DbgCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
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
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        AssertInsideRenderPass();
    }
    instance.Clear(flags);

    profile_.attachmentClears++;
}

void DbgCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        AssertInsideRenderPass();
        for (std::uint32_t i = 0; i < numAttachments; ++i)
            ValidateAttachmentClear(attachments[i]);
    }

    instance.ClearAttachments(numAttachments, attachments);

    profile_.attachmentClears++;
}

/* ----- Buffers ------ */

void DbgCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(buffer.GetBindFlags(), BindFlags::VertexBuffer, "BindFlags::VertexBuffer");
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

    profile_.vertexBufferBindings++;
}

void DbgCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayDbg = LLGL_CAST(DbgBufferArray&, bufferArray);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(bufferArrayDbg.GetBindFlags(), BindFlags::VertexBuffer, "BindFlags::VertexBuffer");

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

    profile_.vertexBufferBindings++;
}

void DbgCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(bufferDbg.desc.bindFlags, BindFlags::IndexBuffer, "BindFlags::IndexBuffer");
        bindings_.indexBuffer = (&bufferDbg);
    }

    instance.SetIndexBuffer(bufferDbg.instance);

    profile_.indexBufferBindings++;
}

/* ----- Stream Output Buffers ------ */

void DbgCommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(buffer.GetBindFlags(), BindFlags::StreamOutputBuffer, "BindFlags::StreamOutputBuffer");
        bindings_.streamOutput = (&bufferDbg);
    }

    instance.SetStreamOutputBuffer(bufferDbg.instance);

    profile_.streamOutputBufferBindings++;
}

void DbgCommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(bufferArray.GetBindFlags(), BindFlags::StreamOutputBuffer, "BindFlags::StreamOutputBuffer");
    }

    instance.SetStreamOutputBufferArray(bufferArray);

    profile_.streamOutputBufferBindings++;
}

void DbgCommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        if (states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output is already busy");
        if (!bindings_.streamOutput)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "no stream-output buffer is bound");
        states_.streamOutputBusy = true;
    }

    instance.BeginStreamOutput(primitiveType);

    profile_.streamOutputSections++;
}

void DbgCommandBuffer::EndStreamOutput()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        if (!states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output has not started");
        states_.streamOutputBusy = false;
    }

    instance.EndStreamOutput();
}

/* ----- Resource View Heaps ----- */

//TODO: record bindings
void DbgCommandBuffer::SetGraphicsResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    LLGL_DBG_SOURCE;
    AssertRecording();

    instance.SetGraphicsResourceHeap(resourceHeap, firstSet);

    profile_.graphicsResourceHeapBindings++;
}

//TODO: record bindings
void DbgCommandBuffer::SetComputeResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstSet)
{
    LLGL_DBG_SOURCE;
    AssertRecording();

    instance.SetComputeResourceHeap(resourceHeap, firstSet);

    profile_.computeResourceHeapBindings++;
}

/* ----- Render Passes ----- */

void DbgCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        if (states_.insideRenderPass)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot begin new render pass while previous render pass is still active");
        states_.insideRenderPass = true;
    }

    if (renderTarget.IsRenderContext())
    {
        auto& renderContextDbg = LLGL_CAST(DbgRenderContext&, renderTarget);

        bindings_.renderContext = &renderContextDbg;
        bindings_.renderTarget  = nullptr;

        instance.BeginRenderPass(renderContextDbg.instance, renderPass, numClearValues, clearValues);
    }
    else
    {
        auto& renderTargetDbg = LLGL_CAST(DbgRenderTarget&, renderTarget);

        bindings_.renderContext = nullptr;
        bindings_.renderTarget  = &renderTargetDbg;

        instance.BeginRenderPass(renderTargetDbg.instance, renderPass, numClearValues, clearValues);
    }

    profile_.renderPassSections++;
}

void DbgCommandBuffer::EndRenderPass()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        if (!states_.insideRenderPass)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot end render pass while no render pass is currently active");
        states_.insideRenderPass = false;
    }

    instance.EndRenderPass();
}

/* ----- Pipeline States ----- */

void DbgCommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineDbg = LLGL_CAST(DbgGraphicsPipeline&, graphicsPipeline);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        bindings_.graphicsPipeline = (&graphicsPipelineDbg);

        if (auto shaderProgram = graphicsPipelineDbg.desc.shaderProgram)
        {
            auto shaderProgramDbg = LLGL_CAST(const DbgShaderProgram*, shaderProgram);
            bindings_.shaderProgram_        = shaderProgramDbg;
            bindings_.anyShaderAttributes   = !(shaderProgramDbg->GetVertexLayout().attributes.empty());
        }
        else
        {
            bindings_.shaderProgram_        = nullptr;
            bindings_.anyShaderAttributes   = false;
        }
    }

    /* Store primitive topology used in graphics pipeline */
    topology_ = graphicsPipelineDbg.desc.primitiveTopology;

    /* Call wrapped function */
    instance.SetGraphicsPipeline(graphicsPipelineDbg.instance);

    profile_.graphicsPipelineBindings++;
}

void DbgCommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    auto& computePipelineDbg = LLGL_CAST(DbgComputePipeline&, computePipeline);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        bindings_.computePipeline = &computePipelineDbg;

        if (auto shaderProgram = computePipelineDbg.desc.shaderProgram)
            bindings_.shaderProgram_ = LLGL_CAST(const DbgShaderProgram*, shaderProgram);
        else
            bindings_.shaderProgram_ = nullptr;
    }

    instance.SetComputePipeline(computePipelineDbg.instance);

    profile_.computePipelineBindings++;
}

/* ----- Queries ----- */

void DbgCommandBuffer::BeginQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapDbg = LLGL_CAST(DbgQueryHeap&, queryHeap);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        if (auto state = GetAndValidateQueryState(queryHeapDbg, query))
        {
            if (*state == DbgQueryHeap::State::Busy)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "query is already busy");
            *state = DbgQueryHeap::State::Busy;
        }
    }

    instance.BeginQuery(queryHeapDbg.instance, query);

    profile_.querySections++;
}

void DbgCommandBuffer::EndQuery(QueryHeap& queryHeap, std::uint32_t query)
{
    auto& queryHeapDbg = LLGL_CAST(DbgQueryHeap&, queryHeap);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        if (auto state = GetAndValidateQueryState(queryHeapDbg, query))
        {
            if (*state != DbgQueryHeap::State::Busy)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "query has not started");
            *state = DbgQueryHeap::State::Ready;
        }
    }

    instance.EndQuery(queryHeapDbg.instance, query);
}

void DbgCommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode)
{
    auto& queryHeapDbg = LLGL_CAST(DbgQueryHeap&, queryHeap);

    LLGL_DBG_SOURCE;
    AssertRecording();

    instance.BeginRenderCondition(queryHeapDbg.instance, query, mode);

    profile_.renderConditionSections++;
}

void DbgCommandBuffer::EndRenderCondition()
{
    LLGL_DBG_SOURCE;
    AssertRecording();
    instance.EndRenderCondition();
}

/* ----- Drawing ----- */

void DbgCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawCmd(numVertices, firstVertex, 1, 0);
    }

    instance.Draw(numVertices, firstVertex);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawIndexedCmd(numIndices, 1, firstIndex, 0, 0);
    }

    instance.DrawIndexed(numIndices, firstIndex);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawIndexedCmd(numIndices, 1, firstIndex, vertexOffset, 0);
    }

    instance.DrawIndexed(numIndices, firstIndex, vertexOffset);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertInstancingSupported();
        ValidateDrawCmd(numVertices, firstVertex, numInstances, 0);
    }

    instance.DrawInstanced(numVertices, firstVertex, numInstances);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertInstancingSupported();
        AssertOffsetInstancingSupported();
        ValidateDrawCmd(numVertices, firstVertex, numInstances, firstInstance);
    }

    instance.DrawInstanced(numVertices, firstVertex, numInstances, firstInstance);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertInstancingSupported();
        ValidateDrawIndexedCmd(numIndices, numInstances, firstIndex, 0, 0);
    }

    instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertInstancingSupported();
        ValidateDrawIndexedCmd(numIndices, numInstances, firstIndex, vertexOffset, 0);
    }

    instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexedInstanced(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertInstancingSupported();
        AssertOffsetInstancingSupported();
        ValidateDrawIndexedCmd(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);
    }

    instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateResourceFlag(bufferDbg.GetBindFlags(), BindFlags::IndirectBuffer, "BindFlags::IndirectBuffer");
        ValidateBufferRange(bufferDbg, offset, sizeof(DrawIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
    }

    instance.DrawIndirect(bufferDbg.instance, offset);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateResourceFlag(bufferDbg.GetBindFlags(), BindFlags::IndirectBuffer, "BindFlags::IndirectBuffer");
        ValidateBufferRange(bufferDbg, offset, stride*numCommands);
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
        ValidateAddressAlignment(stride, 4, "<stride> parameter");
    }

    instance.DrawIndirect(bufferDbg.instance, offset, numCommands, stride);

    profile_.drawCommands += numCommands;
}

void DbgCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateResourceFlag(bufferDbg.GetBindFlags(), BindFlags::IndirectBuffer, "BindFlags::IndirectBuffer");
        ValidateBufferRange(bufferDbg, offset, sizeof(DrawIndexedIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
    }

    instance.DrawIndexedIndirect(bufferDbg.instance, offset);

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateResourceFlag(bufferDbg.GetBindFlags(), BindFlags::IndirectBuffer, "BindFlags::IndirectBuffer");
        ValidateBufferRange(bufferDbg, offset, stride*numCommands);
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
        ValidateAddressAlignment(stride, 4, "<stride> parameter");
    }

    instance.DrawIndexedIndirect(bufferDbg.instance, offset, numCommands, stride);

    profile_.drawCommands += numCommands;
}

/* ----- Compute ----- */

void DbgCommandBuffer::Dispatch(std::uint32_t numWorkGroupsX, std::uint32_t numWorkGroupsY, std::uint32_t numWorkGroupsZ)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        if (numWorkGroupsX * numWorkGroupsY * numWorkGroupsZ == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "thread group size has volume of 0 units");

        AssertComputePipelineBound();
        ValidateThreadGroupLimit(numWorkGroupsX, limits_.maxComputeShaderWorkGroups[0]);
        ValidateThreadGroupLimit(numWorkGroupsY, limits_.maxComputeShaderWorkGroups[1]);
        ValidateThreadGroupLimit(numWorkGroupsZ, limits_.maxComputeShaderWorkGroups[2]);
    }

    instance.Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ);

    profile_.dispatchCommands++;
}

void DbgCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceFlag(bufferDbg.GetBindFlags(), BindFlags::IndirectBuffer, "BindFlags::IndirectBuffer");
        ValidateBufferRange(bufferDbg, offset, sizeof(DispatchIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
    }

    instance.DispatchIndirect(bufferDbg.instance, offset);

    profile_.dispatchCommands++;
}

/* ----- Direct Resource Access ------ */

void DbgCommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(buffer.GetBindFlags(), BindFlags::ConstantBuffer, "BindFlags::ConstantBuffer");
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }

    instanceExt->SetConstantBuffer(bufferDbg.instance, slot, stageFlags);

    profile_.constantBufferBindings++;
}

void DbgCommandBuffer::SetSampleBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(buffer.GetBindFlags(), BindFlags::SampleBuffer, "BindFlags::SampleBuffer");
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }

    instanceExt->SetSampleBuffer(bufferDbg.instance, slot, stageFlags);

    profile_.sampleBufferBindings++;
}

void DbgCommandBuffer::SetRWStorageBuffer(Buffer& buffer, std::uint32_t slot, long stageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateResourceFlag(buffer.GetBindFlags(), BindFlags::RWStorageBuffer, "BindFlags::RWStorageBuffer");
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }

    instanceExt->SetRWStorageBuffer(bufferDbg.instance, slot, stageFlags);

    profile_.rwStorageBufferBindings++;
}

void DbgCommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long stageFlags)
{
    AssertCommandBufferExt(__func__);

    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }

    instanceExt->SetTexture(textureDbg.instance, slot, stageFlags);

    profile_.textureBindings++;
}

void DbgCommandBuffer::SetSampler(Sampler& sampler, std::uint32_t slot, long stageFlags)
{
    AssertCommandBufferExt(__func__);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }

    instanceExt->SetSampler(sampler, slot, stageFlags);

    profile_.samplerBindings++;
}

void DbgCommandBuffer::ResetResourceSlots(
    const ResourceType  resourceType,
    std::uint32_t       firstSlot,
    std::uint32_t       numSlots,
    long                bindFlags,
    long                stageFlags)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (numSlots == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no slots are specified to reset");
        ValidateStageFlags(stageFlags, StageFlags::AllStages);
    }
    instanceExt->ResetResourceSlots(resourceType, firstSlot, numSlots, bindFlags, stageFlags);
}

/* ----- Extended functions ----- */

void DbgCommandBuffer::EnableRecording(bool enable)
{
    if (debugger_)
    {
        if (enable == states_.recording)
        {
            LLGL_DBG_SOURCE;
            if (enable)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot begin nested recording of command buffer");
            else
                LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot end recording of command buffer while no recording is currently active");
        }
        states_.recording = enable;
    }
}

void DbgCommandBuffer::NextProfile(FrameProfile& outputProfile)
{
    /* Copy frame profile values to output profile */
    std::copy(std::begin(profile_.values), std::end(profile_.values), std::begin(outputProfile.values));
}


/*
 * ======= Private: =======
 */

void DbgCommandBuffer::AssertCommandBufferExt(const char* funcName)
{
    if (!instanceExt)
        throw std::runtime_error("illegal function call for a non-extended command buffer: " + std::string(funcName));
}

void DbgCommandBuffer::ValidateViewport(const Viewport& viewport)
{
    if (viewport.width < 0.0f || viewport.height < 0.0f)
        LLGL_DBG_ERROR(ErrorType::UndefinedBehavior, "viewport of negative width or negative height");
    if (viewport.width == 0.0f || viewport.height == 0.0f)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "viewport of empty size (width or height is zero)");

    const auto w = static_cast<std::uint32_t>(viewport.width);
    const auto h = static_cast<std::uint32_t>(viewport.height);

    if ( ( viewport.width  > 0.0f && w > limits_.maxViewportSize[0] ) ||
         ( viewport.height > 0.0f && h > limits_.maxViewportSize[1] ) )
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "viewport exceeded maximal size ([" + std::to_string(w) + " x " + std::to_string(h) +
            "] specified but limit is [" + std::to_string(limits_.maxViewportSize[0]) + " x " + std::to_string(limits_.maxViewportSize[1]) + "])"
        );
    }
}

void DbgCommandBuffer::ValidateAttachmentClear(const AttachmentClear& attachment)
{
    if (bindings_.renderContext != nullptr)
    {
        if ((attachment.flags & ClearFlags::Color) != 0)
            ValidateAttachmentLimit(attachment.colorAttachment, 1);
    }
    else if (auto renderTarget = bindings_.renderTarget)
    {
        if ((attachment.flags & ClearFlags::Color) != 0)
        {
            ValidateAttachmentLimit(attachment.colorAttachment, renderTarget->GetNumColorAttachments());
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

void DbgCommandBuffer::ValidateVertexLayout()
{
    if (bindings_.graphicsPipeline && bindings_.numVertexBuffers > 0)
    {
        auto shaderProgramDbg = LLGL_CAST(const DbgShaderProgram*, bindings_.graphicsPipeline->desc.shaderProgram);
        const auto& vertexLayout = shaderProgramDbg->GetVertexLayout();

        /* Check if vertex layout is specified in active shader program */
        if (vertexLayout.bound)
            ValidateVertexLayoutAttributes(vertexLayout.attributes, bindings_.vertexBuffers, bindings_.numVertexBuffers);
        else if (bindings_.anyNonEmptyVertexBuffer)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "unspecified vertex layout in shader program while bound vertex buffers are non-empty");
    }
}

void DbgCommandBuffer::ValidateVertexLayoutAttributes(const std::vector<VertexAttribute>& shaderAttributes, DbgBuffer** vertexBuffers, std::uint32_t numVertexBuffers)
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

void DbgCommandBuffer::ValidateNumVertices(std::uint32_t numVertices)
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

void DbgCommandBuffer::ValidateNumInstances(std::uint32_t numInstances)
{
    if (numInstances == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no instances will be generated");
}

void DbgCommandBuffer::ValidateVertexID(std::uint32_t firstVertex)
{
    if (firstVertex > 0)
    {
        if (auto shaderProgramDbg = bindings_.shaderProgram_)
        {
            if (auto vertexID = shaderProgramDbg->GetVertexID())
            {
                LLGL_DBG_WARN(
                    WarningType::VaryingBehavior,
                    "bound shader program uses '" + std::string(vertexID) + "' while firstVertex > 0, which may result in varying behavior between different native APIs"
                );
            }
        }
    }
}

void DbgCommandBuffer::ValidateInstanceID(std::uint32_t firstInstance)
{
    if (firstInstance > 0)
    {
        if (auto shaderProgramDbg = bindings_.shaderProgram_)
        {
            if (auto instanceID = shaderProgramDbg->GetInstanceID())
            {
                LLGL_DBG_WARN(
                    WarningType::VaryingBehavior,
                    "bound shader program uses '" + std::string(instanceID) + "' while firstInstance > 0, which may result in varying behavior between different native APIs"
                );
            }
        }
    }
}

void DbgCommandBuffer::ValidateDrawCmd(
    std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t firstInstance)
{
    AssertRecording();
    AssertInsideRenderPass();
    AssertGraphicsPipelineBound();
    AssertVertexBufferBound();
    ValidateVertexLayout();
    ValidateNumVertices(numVertices);
    ValidateNumInstances(numInstances);
    ValidateVertexID(firstVertex);
    ValidateInstanceID(firstInstance);

    if (bindings_.numVertexBuffers > 0 && bindings_.anyShaderAttributes)
        ValidateVertexLimit(numVertices + firstVertex, static_cast<std::uint32_t>(bindings_.vertexBuffers[0]->elements));
}

void DbgCommandBuffer::ValidateDrawIndexedCmd(
    std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t firstInstance)
{
    AssertRecording();
    AssertInsideRenderPass();
    AssertGraphicsPipelineBound();
    AssertVertexBufferBound();
    AssertIndexBufferBound();
    ValidateVertexLayout();
    ValidateNumVertices(numVertices);
    ValidateNumInstances(numInstances);
    ValidateInstanceID(firstInstance);

    if (bindings_.indexBuffer)
        ValidateVertexLimit(numVertices + firstIndex, static_cast<std::uint32_t>(bindings_.indexBuffer->elements));
}

void DbgCommandBuffer::ValidateVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit)
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

void DbgCommandBuffer::ValidateThreadGroupLimit(std::uint32_t size, std::uint32_t limit)
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

void DbgCommandBuffer::ValidateAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound)
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

void DbgCommandBuffer::ValidateResourceFlag(long resourceFlags, long requiredFlag, const char* flagName)
{
    if ((resourceFlags & requiredFlag) != requiredFlag)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "resource was not created with the 'LLGL::" + std::string(flagName) + "' flag enabled"
        );
    }
}

void DbgCommandBuffer::ValidateStageFlags(long stageFlags, long validFlags)
{
    if ((stageFlags & validFlags) == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stage is specified");
    if ((stageFlags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown shader stage flags specified");
}

void DbgCommandBuffer::ValidateBufferRange(DbgBuffer& bufferDbg, std::uint64_t offset, std::uint64_t size)
{
    if (offset + size > bufferDbg.desc.size)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer range out of bounds (" + std::to_string(offset + size) +
            " specified but limit is " + std::to_string(bufferDbg.desc.size) + ")"
        );
    }
}

void DbgCommandBuffer::ValidateAddressAlignment(std::uint64_t address, std::uint64_t alignment, const char* addressName)
{
    if (alignment > 0 && (address % alignment != 0))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            std::string(addressName) + " not aligned to " + std::to_string(alignment) + " byte(s)"
        );
    }
}

bool DbgCommandBuffer::ValidateQueryIndex(DbgQueryHeap& queryHeap, std::uint32_t query)
{
    if (query >= queryHeap.states.size())
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "query index out of bounds (" + std::to_string(query) +
            " specified but upper bound is " + std::to_string(queryHeap.states.size()) + ")"
        );
        return false;
    }
    return true;
}

DbgQueryHeap::State* DbgCommandBuffer::GetAndValidateQueryState(DbgQueryHeap& queryHeap, std::uint32_t query)
{
    if (ValidateQueryIndex(queryHeap, query))
        return &(queryHeap.states[query]);
    else
        return nullptr;
}

void DbgCommandBuffer::AssertRecording()
{
    if (!states_.recording)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "command buffer must be in record mode: missing call to <LLGL::CommandQueue::Begin>");
}

void DbgCommandBuffer::AssertInsideRenderPass()
{
    if (!states_.insideRenderPass)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "operation is only allowed inside a render pass: missing call to <LLGL::CommandBuffer::BeginRenderPass>");
}

void DbgCommandBuffer::AssertGraphicsPipelineBound()
{
    if (!bindings_.graphicsPipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no graphics pipeline is bound: missing call to <LLGL::CommandBuffer::SetGraphicsPipeline>");
}

void DbgCommandBuffer::AssertComputePipelineBound()
{
    if (!bindings_.computePipeline)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no compute pipeline is bound: missing call to <LLGL::CommandBuffer::SetComputePipeline>");
}

void DbgCommandBuffer::AssertVertexBufferBound()
{
    if (bindings_.numVertexBuffers > 0)
    {
        for (std::uint32_t i = 0; i < bindings_.numVertexBuffers; ++i)
        {
            /* Check if buffer is initialized (ignore empty buffers) */
            auto buffer = bindings_.vertexBuffers[i];
            if (buffer->elements > 0 && !buffer->initialized)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized vertex buffer is bound at slot " + std::to_string(i));
            if (buffer->mapped)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex buffer used for drawing while being mapped to CPU local memory");
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no vertex buffer is bound");
}

void DbgCommandBuffer::AssertIndexBufferBound()
{
    if (auto buffer = bindings_.indexBuffer)
    {
        if (!buffer->initialized)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "uninitialized index buffer is bound");
        if (buffer->mapped)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "index buffer used for drawing while being mapped to CPU local memory");
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no index buffer is bound");
}

void DbgCommandBuffer::AssertInstancingSupported()
{
    if (!features_.hasInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("instancing");
}

void DbgCommandBuffer::AssertOffsetInstancingSupported()
{
    if (!features_.hasOffsetInstancing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("offset instancing");
}

void DbgCommandBuffer::AssertIndirectDrawingSupported()
{
    if (!features_.hasIndirectDrawing)
        LLGL_DBG_ERROR_NOT_SUPPORTED("indirect drawing");
}

void DbgCommandBuffer::WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices)
{
    LLGL_DBG_WARN(
        WarningType::ImproperArgument,
        "improper number of vertices for " + topologyName + " (" + std::to_string(unusedVertices) +
        " unused " + std::string(unusedVertices > 1 ? "vertices" : "vertex") + ")"
    );
}

void DbgCommandBuffer::ResetFrameProfile()
{
    /* Reset all counters of frame profile */
    std::fill(std::begin(profile_.values), std::end(profile_.values), 0);
}


} // /namespace LLGL



// ================================================================================
