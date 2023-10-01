/*
 * DbgCommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgCommandBuffer.h"
#include "DbgCore.h"
#include "../CheckedCast.h"
#include "../ResourceUtils.h"
#include "../PipelineStateUtils.h"
#include "../../Core/StringUtils.h"
#include "../../Core/Assertion.h"

#include "DbgSwapChain.h"
#include "Buffer/DbgBuffer.h"
#include "Buffer/DbgBufferArray.h"
#include "RenderState/DbgQueryHeap.h"
#include "RenderState/DbgPipelineState.h"
#include "RenderState/DbgPipelineLayout.h"
#include "RenderState/DbgResourceHeap.h"
#include "Shader/DbgShader.h"
#include "Texture/DbgTexture.h"
#include "Texture/DbgRenderTarget.h"

#include <LLGL/RenderingDebugger.h>
#include <LLGL/IndirectArguments.h>
#include <LLGL/TypeInfo.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


#define LLGL_DBG_COMMAND(NAME, CMD) \
    if (perfProfilerEnabled_)       \
    {                               \
        StartTimer(NAME);           \
        CMD;                        \
        EndTimer();                 \
    }                               \
    else                            \
    {                               \
        CMD;                        \
    }

#define LLGL_DBG_ASSERT_PTR(NAME) \
    AssertNullPointer(NAME, #NAME)

static const char* GetLabelOrDefault(const std::string& label, const char* defaultLabel)
{
    if (label.empty())
        return defaultLabel;
    else
        return label.c_str();
}

DbgCommandBuffer::DbgCommandBuffer(
    RenderSystem&                   renderSystemInstance,
    CommandQueue&                   commandQueueInstance,
    CommandBuffer&                  commandBufferInstance,
    RenderingDebugger*              debugger,
    RenderingProfiler*              profiler,
    const CommandBufferDescriptor&  desc,
    const RenderingCapabilities&    caps)
:
    instance        { commandBufferInstance                                             },
    desc            { desc                                                              },
    debugger_       { debugger                                                          },
    profiler_       { profiler                                                          },
    features_       { caps.features                                                     },
    limits_         { caps.limits                                                       },
    queryTimerPool_ { renderSystemInstance, commandQueueInstance, commandBufferInstance }
{
}

/* ----- Encoding ----- */

void DbgCommandBuffer::Begin()
{
    /* Reset previous states and records */
    ResetStates();
    ResetRecords();

    /* Enable performance profiler if it was scheduled */
    perfProfilerEnabled_ = (profiler_ != nullptr && profiler_->timeRecordingEnabled);
    if (perfProfilerEnabled_)
        queryTimerPool_.Reset();

    /* Begin with command recording  */
    if (debugger_)
        EnableRecording(true);

    instance.Begin();

    profile_.commandBufferEncodings++;
}

void DbgCommandBuffer::End()
{
    /* End with command recording */
    if (debugger_)
        EnableRecording(false);
    instance.End();

    /* Resolve timer query results for performance profiler */
    if (perfProfilerEnabled_)
        queryTimerPool_.TakeRecords(profile_.timeRecords);
}

void DbgCommandBuffer::Execute(CommandBuffer& deferredCommandBuffer)
{
    auto& commandBufferDbg = LLGL_CAST(DbgCommandBuffer&, deferredCommandBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        if (&deferredCommandBuffer == this)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "command buffer tried to execute itself");

        ValidateBindFlags(
            commandBufferDbg.desc.flags,
            CommandBufferFlags::Secondary,
            CommandBufferFlags::Secondary,
            "LLGL::CommandBuffer"
        );
    }

    LLGL_DBG_COMMAND( "Execute", instance.Execute(commandBufferDbg.instance) );
}

/* ----- Blitting ----- */

void DbgCommandBuffer::UpdateBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    const void*     data,
    std::uint16_t   dataSize)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBufferRange(dstBufferDbg, dstOffset, dataSize, "destination range");
    }

    LLGL_DBG_COMMAND( "UpdateBuffer", instance.UpdateBuffer(dstBufferDbg.instance, dstOffset, data, dataSize) );

    profile_.bufferUpdates++;
}

void DbgCommandBuffer::CopyBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    Buffer&         srcBuffer,
    std::uint64_t   srcOffset,
    std::uint64_t   size)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);
    auto& srcBufferDbg = LLGL_CAST(DbgBuffer&, srcBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBufferRange(dstBufferDbg, dstOffset, size, "destination range");
        ValidateBufferRange(srcBufferDbg, srcOffset, size, "source range");
        ValidateBindBufferFlags(dstBufferDbg, BindFlags::CopyDst);
        ValidateBindBufferFlags(srcBufferDbg, BindFlags::CopySrc);
    }

    LLGL_DBG_COMMAND( "CopyBuffer", instance.CopyBuffer(dstBufferDbg.instance, dstOffset, srcBufferDbg.instance, srcOffset, size) );

    profile_.bufferCopies++;
}

// Returns the minimum required memory footprint to copy the specified texture region into a buffer
static std::size_t GetTextureRegionMinFootprint(const DbgTexture& textureDbg, const TextureRegion& region)
{
    const std::uint32_t numTexels = NumMipTexels(textureDbg.GetType(), region.extent, region.subresource.baseMipLevel);
    return GetMemoryFootprint(textureDbg.GetFormat(), numTexels);
}

void DbgCommandBuffer::CopyBufferFromTexture(
    Buffer&                 dstBuffer,
    std::uint64_t           dstOffset,
    Texture&                srcTexture,
    const TextureRegion&    srcRegion,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);
    auto& srcTextureDbg = LLGL_CAST(DbgTexture&, srcTexture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindBufferFlags(dstBufferDbg, BindFlags::CopyDst);
        ValidateBufferRange(dstBufferDbg, dstOffset, GetTextureRegionMinFootprint(srcTextureDbg, srcRegion));
        ValidateBindTextureFlags(srcTextureDbg, BindFlags::CopySrc);
        ValidateTextureRegion(srcTextureDbg, srcRegion);
        ValidateTextureBufferCopyStrides(srcTextureDbg, rowStride, layerStride, srcRegion.extent);
    }

    LLGL_DBG_COMMAND( "CopyBufferFromTexture", instance.CopyBufferFromTexture(dstBufferDbg.instance, dstOffset, srcTextureDbg.instance, srcRegion, rowStride, layerStride) );

    profile_.bufferCopies++;
}

void DbgCommandBuffer::FillBuffer(
    Buffer&         dstBuffer,
    std::uint64_t   dstOffset,
    std::uint32_t   value,
    std::uint64_t   fillSize)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindBufferFlags(dstBufferDbg, BindFlags::CopyDst);

        if (fillSize == LLGL_WHOLE_SIZE)
        {
            if (dstOffset != 0)
                LLGL_DBG_WARN(WarningType::ImproperArgument, "non-zero argument for 'dstOffset' is ignored because 'fillSize' is set to LLGL::wholeSize");
        }
        else
        {
            if (fillSize % 4 != 0)
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer fill size is not a multiple of 4");
            ValidateBufferRange(dstBufferDbg, dstOffset, fillSize);
        }
    }

    LLGL_DBG_COMMAND( "FillBuffer", instance.FillBuffer(dstBufferDbg.instance, dstOffset, value, fillSize) );

    profile_.bufferFills++;
}

void DbgCommandBuffer::CopyTexture(
    Texture&                dstTexture,
    const TextureLocation&  dstLocation,
    Texture&                srcTexture,
    const TextureLocation&  srcLocation,
    const Extent3D&         extent)
{
    auto& dstTextureDbg = LLGL_CAST(DbgTexture&, dstTexture);
    auto& srcTextureDbg = LLGL_CAST(DbgTexture&, srcTexture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindTextureFlags(dstTextureDbg, BindFlags::CopyDst);
        ValidateBindTextureFlags(srcTextureDbg, BindFlags::CopySrc);
    }

    LLGL_DBG_COMMAND( "CopyTexture", instance.CopyTexture(dstTextureDbg.instance, dstLocation, srcTextureDbg.instance, srcLocation, extent) );

    profile_.textureCopies++;
}

void DbgCommandBuffer::CopyTextureFromBuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    Buffer&                 srcBuffer,
    std::uint64_t           srcOffset,
    std::uint32_t           rowStride,
    std::uint32_t           layerStride)
{
    auto& dstTextureDbg = LLGL_CAST(DbgTexture&, dstTexture);
    auto& srcBufferDbg = LLGL_CAST(DbgBuffer&, srcBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindTextureFlags(dstTextureDbg, BindFlags::CopyDst);
        ValidateTextureRegion(dstTextureDbg, dstRegion);
        ValidateBindBufferFlags(srcBufferDbg, BindFlags::CopySrc);
        ValidateBufferRange(srcBufferDbg, srcOffset, GetTextureRegionMinFootprint(dstTextureDbg, dstRegion));
        ValidateTextureBufferCopyStrides(dstTextureDbg, rowStride, layerStride, dstRegion.extent);
    }

    LLGL_DBG_COMMAND( "CopyTextureFromBuffer", instance.CopyTextureFromBuffer(dstTextureDbg.instance, dstRegion, srcBufferDbg.instance, srcOffset, rowStride, layerStride) );

    profile_.textureCopies++;
}

void DbgCommandBuffer::CopyTextureFromFramebuffer(
    Texture&                dstTexture,
    const TextureRegion&    dstRegion,
    const Offset2D&         srcOffset)
{
    auto& dstTextureDbg = LLGL_CAST(DbgTexture&, dstTexture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindTextureFlags(dstTextureDbg, BindFlags::CopyDst);
        ValidateTextureRegion(dstTextureDbg, dstRegion);
        if (dstRegion.subresource.numArrayLayers > 1)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot copy texture from framebuffer with number of array layers greater than 1");
        if (dstRegion.extent.depth != 1)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot copy texture from framebuffer with a depth extent of " + std::to_string(dstRegion.extent.depth));
        if (bindings_.swapChain == nullptr)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "copy texture from framebuffer is only supported for SwapChain framebuffers");
        if (DbgRenderTarget* renderTargetDbg = bindings_.renderTarget)
            ValidateRenderTargetRange(*renderTargetDbg, srcOffset, Extent2D{ dstRegion.extent.width, dstRegion.extent.height });
    }

    LLGL_DBG_COMMAND( "CopyTextureFromFramebuffer", instance.CopyTextureFromFramebuffer(dstTextureDbg.instance, dstRegion, srcOffset) );

    profile_.textureCopies++;
}

void DbgCommandBuffer::GenerateMips(Texture& texture)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateGenerateMips(textureDbg);
    }

    LLGL_DBG_COMMAND( "GenerateMips", instance.GenerateMips(textureDbg.instance) );

    profile_.mipMapsGenerations++;
}

void DbgCommandBuffer::GenerateMips(Texture& texture, const TextureSubresource& subresource)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateGenerateMips(textureDbg, &subresource);
    }

    LLGL_DBG_COMMAND( "GenerateMips", instance.GenerateMips(textureDbg.instance, subresource) );

    profile_.mipMapsGenerations++;
}

/* ----- Viewport and Scissor ----- */

void DbgCommandBuffer::SetViewport(const Viewport& viewport)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateViewport(viewport);

        /* Store information how many viewports are bound since at least one must be active when Draw* commands are issued */
        bindings_.numViewports = 1;
    }

    LLGL_DBG_COMMAND( "SetViewport", instance.SetViewport(viewport) );
}

void DbgCommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        AssertRecording();
        LLGL_DBG_ASSERT_PTR(viewports);

        /* Validate all viewports in array */
        if (viewports)
        {
            for_range(i, numViewports)
                ValidateViewport(viewports[i]);
        }

        /* Validate array size */
        if (numViewports > limits_.maxViewports)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "viewport array index out of bounds: " +
                std::to_string(numViewports) + " specified but limit is " + std::to_string(limits_.maxViewports)
            );
        }
        else if (numViewports == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no viewports are specified");

        /* Store information how many viewports are bound since at least one must be active when Draw* commands are issued */
        bindings_.numViewports = numViewports;
    }

    LLGL_DBG_COMMAND( "SetViewports", instance.SetViewports(numViewports, viewports) );
}

void DbgCommandBuffer::SetScissor(const Scissor& scissor)
{
    LLGL_DBG_SOURCE;
    AssertRecording();
    LLGL_DBG_COMMAND( "SetScissor", instance.SetScissor(scissor) );
}

void DbgCommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        LLGL_DBG_ASSERT_PTR(scissors);
        if (numScissors == 0)
            LLGL_DBG_WARN(WarningType::PointlessOperation, "no scissor rectangles are specified");
    }

    LLGL_DBG_COMMAND( "SetScissors", instance.SetScissors(numScissors, scissors) );
}

/* ----- Buffers ------ */

void DbgCommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindBufferFlags(bufferDbg, BindFlags::VertexBuffer);

        bindings_.vertexBufferStore[0]  = (&bufferDbg);
        bindings_.vertexBuffers         = bindings_.vertexBufferStore;
        bindings_.numVertexBuffers      = 1;
    }

    LLGL_DBG_COMMAND( "SetVertexBuffer", instance.SetVertexBuffer(bufferDbg.instance) );

    profile_.vertexBufferBindings++;
}

void DbgCommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& bufferArrayDbg = LLGL_CAST(DbgBufferArray&, bufferArray);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateBindFlags(bufferArrayDbg.GetBindFlags(), BindFlags::VertexBuffer, BindFlags::VertexBuffer, "LLGL::BufferArray");

        bindings_.vertexBuffers     = bufferArrayDbg.buffers.data();
        bindings_.numVertexBuffers  = static_cast<std::uint32_t>(bufferArrayDbg.buffers.size());
    }

    LLGL_DBG_COMMAND( "SetVertexBufferArray", instance.SetVertexBufferArray(bufferArrayDbg.instance) );

    profile_.vertexBufferBindings++;
}

void DbgCommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        ValidateBindBufferFlags(bufferDbg, BindFlags::IndexBuffer);
        ValidateIndexType(bufferDbg.desc.format);

        bindings_.indexBuffer           = (&bufferDbg);
        bindings_.indexBufferFormatSize = 0;
        bindings_.indexBufferOffset     = 0;
    }

    LLGL_DBG_COMMAND( "SetIndexBuffer", instance.SetIndexBuffer(bufferDbg.instance) );

    profile_.indexBufferBindings++;
}

//TODO: validation of <offset> param
void DbgCommandBuffer::SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        ValidateBindBufferFlags(bufferDbg, BindFlags::IndexBuffer);
        ValidateIndexType(format);

        bindings_.indexBuffer           = (&bufferDbg);
        bindings_.indexBufferFormatSize = (GetFormatAttribs(format).bitSize / 8);
        bindings_.indexBufferOffset     = offset;

        if (offset > bufferDbg.desc.size)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "index buffer offset out of bounds: " + std::to_string(offset) +
                " specified but limit is " + std::to_string(bufferDbg.desc.size)
            );
        }
    }

    LLGL_DBG_COMMAND( "SetIndexBuffer", instance.SetIndexBuffer(bufferDbg.instance, format, offset) );

    profile_.indexBufferBindings++;
}

/* ----- Resources ----- */

//TODO: also record individual resource bindings
void DbgCommandBuffer::SetResourceHeap(ResourceHeap& resourceHeap, std::uint32_t descriptorSet)
{
    auto& resourceHeapDbg = LLGL_CAST(DbgResourceHeap&, resourceHeap);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateDescriptorSetIndex(descriptorSet, resourceHeapDbg.GetNumDescriptorSets(), resourceHeapDbg.label.c_str());
        bindings_.bindingTable.resourceHeap = &resourceHeap;
    }

    LLGL_DBG_COMMAND( "SetResourceHeap", instance.SetResourceHeap(resourceHeapDbg.instance, descriptorSet) );

    profile_.resourceHeapBindings++;
}

void DbgCommandBuffer::SetResource(std::uint32_t descriptor, Resource& resource)
{
    const BindingDescriptor* bindingDesc = nullptr;

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        if (auto* pso = bindings_.pipelineState)
        {
            if (auto* psoLayout = pso->pipelineLayout)
                bindingDesc = GetAndValidateResourceDescFromPipeline(*psoLayout, descriptor, resource);
        }
        else
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot bind resource without pipeline state");

        if (descriptor < bindings_.bindingTable.resources.size())
            bindings_.bindingTable.resources[descriptor] = &resource;
    }

    switch (resource.GetResourceType())
    {
        case ResourceType::Undefined:
        break;

        case ResourceType::Buffer:
        {
            /* Forward buffer resource to wrapped instance */
            auto& bufferDbg = LLGL_CAST(DbgBuffer&, resource);

            if (bindingDesc != nullptr)
            {
                ValidateBindFlags(
                    bufferDbg.desc.bindFlags,
                    bindingDesc->bindFlags,
                    (BindFlags::ConstantBuffer | BindFlags::Sampled | BindFlags::Storage),
                    GetLabelOrDefault(bufferDbg.label, "LLGL::Buffer")
                );
            }

            LLGL_DBG_COMMAND( "SetResource", instance.SetResource(descriptor, bufferDbg.instance) );

            /* Record binding for profiling */
            if (bindingDesc != nullptr)
            {
                if ((bindingDesc->bindFlags & BindFlags::ConstantBuffer) != 0)
                    profile_.constantBufferBindings++;
                if ((bindingDesc->bindFlags & BindFlags::Sampled) != 0)
                    profile_.sampledBufferBindings++;
                if ((bindingDesc->bindFlags & BindFlags::Storage) != 0)
                    profile_.storageBufferBindings++;
            }
        }
        break;

        case ResourceType::Texture:
        {
            /* Forward texture resource to wrapped instance */
            auto& textureDbg = LLGL_CAST(DbgTexture&, resource);

            if (bindingDesc != nullptr)
            {
                ValidateBindFlags(
                    textureDbg.desc.bindFlags,
                    bindingDesc->bindFlags,
                    (BindFlags::Sampled | BindFlags::Storage | BindFlags::CombinedSampler),
                    GetLabelOrDefault(textureDbg.label, "LLGL::Buffer")
                );
            }

            LLGL_DBG_COMMAND( "SetResource", instance.SetResource(descriptor, textureDbg.instance) );

            /* Record binding for profiling */
            if (bindingDesc != nullptr)
            {
                if ((bindingDesc->bindFlags & BindFlags::Sampled) != 0)
                    profile_.sampledTextureBindings++;
                if ((bindingDesc->bindFlags & BindFlags::Storage) != 0)
                    profile_.storageTextureBindings++;
            }
        }
        break;

        case ResourceType::Sampler:
        {
            /* No bind flags allowed for samplers */
            //TODO: use DbgSampler
            if (bindingDesc != nullptr)
                ValidateBindFlags(0, bindingDesc->bindFlags, 0, "LLGL::Sampler");

            /* Forward sampler resource to wrapped instance */
            LLGL_DBG_COMMAND( "SetResource", instance.SetResource(descriptor, resource) );

            /* Record binding for profiling */
            profile_.samplerBindings++;
        }
        break;
    }
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

    LLGL_DBG_COMMAND( "ResetResourceSlots", instance.ResetResourceSlots(resourceType, firstSlot, numSlots, bindFlags, stageFlags) );
}

/* ----- Render Passes ----- */

void DbgCommandBuffer::BeginRenderPass(
    RenderTarget&       renderTarget,
    const RenderPass*   renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues,
    std::uint32_t       swapBufferIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        if (states_.insideRenderPass)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot begin new render pass while previous render pass is still active");
        states_.insideRenderPass = true;
    }

    renderPass = DbgGetInstance<DbgRenderPass>(renderPass);

    if (LLGL::IsInstanceOf<SwapChain>(renderTarget))
    {
        auto& swapChainDbg = LLGL_CAST(DbgSwapChain&, renderTarget);

        bindings_.swapChain     = &swapChainDbg;
        bindings_.renderTarget  = nullptr;

        /* Record swap-chain frame to validate when submitting the command buffer */
        if (debugger_)
        {
            const std::uint32_t actualSwapBufferIndex = (swapBufferIndex == LLGL_CURRENT_SWAP_INDEX ? swapChainDbg.GetCurrentSwapIndex() : swapBufferIndex);
            records_.swapChainFrames.push_back({ bindings_.swapChain, actualSwapBufferIndex });
            ValidateSwapBufferIndex(swapChainDbg, actualSwapBufferIndex);
        }

        instance.BeginRenderPass(swapChainDbg.instance, renderPass, numClearValues, clearValues, swapBufferIndex);
    }
    else
    {
        auto& renderTargetDbg = LLGL_CAST(DbgRenderTarget&, renderTarget);

        bindings_.swapChain     = nullptr;
        bindings_.renderTarget  = &renderTargetDbg;

        instance.BeginRenderPass(renderTargetDbg.instance, renderPass, numClearValues, clearValues, swapBufferIndex);
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

void DbgCommandBuffer::Clear(long flags, const ClearValue& clearValue)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        AssertInsideRenderPass();
    }

    LLGL_DBG_COMMAND( "Clear", instance.Clear(flags, clearValue) );

    profile_.attachmentClears++;
}

void DbgCommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        AssertInsideRenderPass();
        for_range(i, numAttachments)
            ValidateAttachmentClear(attachments[i]);
    }

    LLGL_DBG_COMMAND( "ClearAttachments", instance.ClearAttachments(numAttachments, attachments) );

    profile_.attachmentClears++;
}

/* ----- Pipeline States ----- */

void DbgCommandBuffer::SetPipelineState(PipelineState& pipelineState)
{
    auto& pipelineStateDbg = LLGL_CAST(DbgPipelineState&, pipelineState);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        /* Bind graphics pipeline and unbind compute pipeline */
        bindings_.pipelineState         = (&pipelineStateDbg);
        bindings_.anyShaderAttributes   = false;

        if (pipelineStateDbg.isGraphicsPSO)
        {
            if (auto vertexShader = pipelineStateDbg.graphicsDesc.vertexShader)
            {
                auto vertexShaderDbg = LLGL_CAST(const DbgShader*, vertexShader);
                //TODO: store bound vertex shader
                bindings_.anyShaderAttributes = !(vertexShaderDbg->desc.vertex.inputAttribs.empty());
            }

            /* Store dynamic states */
            bindings_.blendFactorSet = !pipelineStateDbg.HasDynamicBlendFactor();
            bindings_.stencilRefSet = !pipelineStateDbg.HasDynamicStencilRef();

            /* If the PSO was created with static viewports, this PSO dictates the number of bound viewports */
            if (!pipelineStateDbg.graphicsDesc.viewports.empty())
                bindings_.numViewports = static_cast<std::uint32_t>(pipelineStateDbg.graphicsDesc.viewports.size());

            if (!states_.insideRenderPass)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "graphics PSO must be bound inside a render pass");
        }
        else
        {
            if (auto computeShader = pipelineStateDbg.computeDesc.computeShader)
            {
                //TODO: store bound compute shader
            }

            if (states_.insideRenderPass)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "compute PSO must be bound outside a render pass");
        }

        ResetBindingTable(bindings_.pipelineState->pipelineLayout);
    }

    /* Store primitive topology used in graphics pipeline */
    if (pipelineStateDbg.isGraphicsPSO)
        topology_ = pipelineStateDbg.graphicsDesc.primitiveTopology;

    /* Call wrapped function */
    LLGL_DBG_COMMAND( "SetPipelineState", instance.SetPipelineState(pipelineStateDbg.instance) );

    if (pipelineStateDbg.isGraphicsPSO)
        profile_.graphicsPipelineBindings++;
    else
        profile_.computePipelineBindings++;
}

void DbgCommandBuffer::SetBlendFactor(const float color[4])
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (auto pipelineStateDbg = AssertAndGetGraphicsPSO())
        {
            if (pipelineStateDbg->graphicsDesc.blend.blendFactorDynamic)
                bindings_.blendFactorSet = true;
            else
                LLGL_DBG_ERROR(ErrorType::InvalidState, "graphics pipeline was not created with 'blendFactorDynamic' enabled");
        }
    }

    LLGL_DBG_COMMAND( "SetBlendFactor", instance.SetBlendFactor(color) );
}

void DbgCommandBuffer::SetStencilReference(std::uint32_t reference, const StencilFace stencilFace)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (auto pipelineStateDbg = AssertAndGetGraphicsPSO())
        {
            if (pipelineStateDbg->graphicsDesc.stencil.referenceDynamic)
                bindings_.stencilRefSet = true;
            else
                LLGL_DBG_ERROR(ErrorType::InvalidState, "graphics pipeline was not created with 'referenceDynamic' enabled");
        }
    }

    LLGL_DBG_COMMAND( "SetStencilReference", instance.SetStencilReference(reference, stencilFace) );
}

void DbgCommandBuffer::SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        LLGL_DBG_ASSERT_PTR(data);
        if (auto* pso = bindings_.pipelineState)
        {
            if (auto* psoLayout = pso->pipelineLayout)
                ValidateUniforms(*psoLayout, first, dataSize);
        }
        else
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot set uniforms without pipeline state");
    }

    LLGL_DBG_COMMAND( "SetUniforms", instance.SetUniforms(first, data, dataSize) );
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

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
        ValidateRenderCondition(queryHeapDbg, query);
    }

    instance.BeginRenderCondition(queryHeapDbg.instance, query, mode);

    profile_.renderConditionSections++;
}

void DbgCommandBuffer::EndRenderCondition()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();
    }
    instance.EndRenderCondition();
}

/* ----- Stream Output ------ */

void DbgCommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers)
{
    Buffer* bufferInstances[LLGL_MAX_NUM_SO_BUFFERS];
    bool validationFailed = false;

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        ValidateStreamOutputs(numBuffers);
        numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);

        /* Bind stream-output buffers */
        for (std::uint32_t i = 0; i < numBuffers; ++i)
        {
            auto bufferDbg = LLGL_CAST(DbgBuffer*, buffers[i]);
            if (bufferDbg != nullptr)
            {
                ValidateBindBufferFlags(*bufferDbg, BindFlags::StreamOutputBuffer);
                bindings_.streamOutputs[i] = bufferDbg;
                bufferInstances[i] = &(bufferDbg->instance);
            }
            else
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "null pointer in array of stream-output buffers"
                );
                validationFailed = true;
            }
        }

        bindings_.numStreamOutputs = numBuffers;

        /* Validate stream-outputs are currently not active */
        if (states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output is already busy");
        states_.streamOutputBusy = true;
    }
    else
    {
        /* Only gather buffer instances from array */
        numBuffers = std::min(numBuffers, LLGL_MAX_NUM_SO_BUFFERS);
        for (std::uint32_t i = 0; i < numBuffers; ++i)
        {
            auto bufferDbg = LLGL_CAST(DbgBuffer*, buffers[i]);
            if (bufferDbg != nullptr)
                bufferInstances[i] = &(bufferDbg->instance);
            else
                return;
        }
    }

    if (!validationFailed)
        instance.BeginStreamOutput(numBuffers, bufferInstances);

    profile_.streamOutputSections++;
}

void DbgCommandBuffer::EndStreamOutput()
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertRecording();

        /* Validate stream-outputs are currently active */
        if (!states_.streamOutputBusy)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "stream-output has not started");
        states_.streamOutputBusy = false;

        bindings_.numStreamOutputs = 0;
    }

    instance.EndStreamOutput();
}

/* ----- Drawing ----- */

void DbgCommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawCmd(numVertices, firstVertex, 1, 0);
    }

    LLGL_DBG_COMMAND( "Draw", instance.Draw(numVertices, firstVertex) );

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawIndexedCmd(numIndices, 1, firstIndex, 0, 0);
    }

    LLGL_DBG_COMMAND( "DrawIndexed", instance.DrawIndexed(numIndices, firstIndex) );

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexed(std::uint32_t numIndices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateDrawIndexedCmd(numIndices, 1, firstIndex, vertexOffset, 0);
    }

    LLGL_DBG_COMMAND( "DrawIndexed", instance.DrawIndexed(numIndices, firstIndex, vertexOffset) );

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

    LLGL_DBG_COMMAND( "DrawInstanced", instance.DrawInstanced(numVertices, firstVertex, numInstances) );

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

    LLGL_DBG_COMMAND( "DrawInstanced", instance.DrawInstanced(numVertices, firstVertex, numInstances, firstInstance) );

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

    LLGL_DBG_COMMAND( "DrawIndexedInstanced", instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex) );

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

    LLGL_DBG_COMMAND( "DrawIndexedInstanced", instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset) );

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

    LLGL_DBG_COMMAND( "DrawIndexedInstanced", instance.DrawIndexedInstanced(numIndices, numInstances, firstIndex, vertexOffset, firstInstance) );

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateBindBufferFlags(bufferDbg, BindFlags::IndirectBuffer);
        ValidateBufferRange(bufferDbg, offset, sizeof(DrawIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
    }

    LLGL_DBG_COMMAND( "DrawIndirect", instance.DrawIndirect(bufferDbg.instance, offset) );

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateBindBufferFlags(bufferDbg, BindFlags::IndirectBuffer);
        ValidateBufferRange(bufferDbg, offset, stride*numCommands);
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
        ValidateAddressAlignment(stride, 4, "<stride> parameter");
    }

    LLGL_DBG_COMMAND( "DrawIndirect", instance.DrawIndirect(bufferDbg.instance, offset, numCommands, stride) );

    profile_.drawCommands += numCommands;
}

void DbgCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateBindBufferFlags(bufferDbg, BindFlags::IndirectBuffer);
        ValidateBufferRange(bufferDbg, offset, sizeof(DrawIndexedIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
    }

    LLGL_DBG_COMMAND( "DrawIndexedIndirect", instance.DrawIndexedIndirect(bufferDbg.instance, offset) );

    profile_.drawCommands++;
}

void DbgCommandBuffer::DrawIndexedIndirect(Buffer& buffer, std::uint64_t offset, std::uint32_t numCommands, std::uint32_t stride)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        AssertIndirectDrawingSupported();
        ValidateBindBufferFlags(bufferDbg, BindFlags::IndirectBuffer);
        ValidateBufferRange(bufferDbg, offset, stride*numCommands);
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
        ValidateAddressAlignment(stride, 4, "<stride> parameter");
    }

    LLGL_DBG_COMMAND( "DrawIndexedIndirect", instance.DrawIndexedIndirect(bufferDbg.instance, offset, numCommands, stride) );

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
        ValidateBindingTable();
    }

    LLGL_DBG_COMMAND( "Dispatch", instance.Dispatch(numWorkGroupsX, numWorkGroupsY, numWorkGroupsZ) );

    profile_.dispatchCommands++;
}

void DbgCommandBuffer::DispatchIndirect(Buffer& buffer, std::uint64_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateBindBufferFlags(bufferDbg, BindFlags::IndirectBuffer);
        ValidateBufferRange(bufferDbg, offset, sizeof(DispatchIndirectArguments));
        ValidateAddressAlignment(offset, 4, "<offset> parameter");
        ValidateBindingTable();
    }

    LLGL_DBG_COMMAND( "DispatchIndirect", instance.DispatchIndirect(bufferDbg.instance, offset) );

    profile_.dispatchCommands++;
}

/* ----- Debugging ----- */

void DbgCommandBuffer::PushDebugGroup(const char* name)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        LLGL_DBG_ASSERT_PTR(name);
        debugger_->SetDebugGroup(name);
    }

    if (!name)
        name = "<null pointer>";

    debugGroups_.push(name);
    instance.PushDebugGroup(name);
}

void DbgCommandBuffer::PopDebugGroup()
{
    instance.PopDebugGroup();
    debugGroups_.pop();

    if (debugger_)
    {
        if (debugGroups_.empty())
            debugger_->SetDebugGroup(nullptr);
        else
            debugger_->SetDebugGroup(debugGroups_.top().c_str());
    }
}

/* ----- Extensions ----- */

void DbgCommandBuffer::DoNativeCommand(const void* nativeCommand, std::size_t nativeCommandSize)
{
    LLGL_DBG_COMMAND( "DoNativeCommand", instance.DoNativeCommand(nativeCommand, nativeCommandSize) );
}

bool DbgCommandBuffer::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return instance.GetNativeHandle(nativeHandle, nativeHandleSize);
}

/* ----- Internal ----- */

void DbgCommandBuffer::NextProfile(FrameProfile& outputProfile)
{
    /* Copy frame profile values to output profile */
    ::memcpy(outputProfile.values, profile_.values, sizeof(profile_.values));
    outputProfile.timeRecords = std::move(profile_.timeRecords);
}

void DbgCommandBuffer::ValidateSubmit()
{
    for (const SwapChainFramePair& pair : records_.swapChainFrames)
    {
        if (pair.swapChain->GetCurrentSwapIndex() != pair.frame)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "command buffer submitted with swap-chain " +
                std::string(pair.swapChain->label.empty() ? "" : "['" + pair.swapChain->label + "'] ") +
                "back-buffer [" + std::to_string(pair.frame) + "] while swap-chain has current back buffer [" +
                std::to_string(pair.swapChain->GetCurrentSwapIndex()) + "]"
            );
        }
    }
}

#undef LLGL_DBG_COMMAND


/*
 * ======= Private: =======
 */

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

void DbgCommandBuffer::ValidateGenerateMips(DbgTexture& textureDbg, const TextureSubresource* subresource)
{
    if ((textureDbg.desc.bindFlags & BindFlags::ColorAttachment) == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidState,
            "cannot generate MIP-maps for texture that was created without 'LLGL::BindFlags::ColorAttachment' flag"
        );
    }

    if (subresource != nullptr)
    {
        /* Validate for subresource */
        if (subresource->numMipLevels == 0)
        {
            LLGL_DBG_WARN(
                WarningType::PointlessOperation,
                "generating a total number of 0 MIP-maps for texture has no effect"
            );
        }
        else if (subresource->baseMipLevel + subresource->numMipLevels > textureDbg.mipLevels)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot generate MIP-maps for texture with subresource being out of bounds: "
                "MIP-map range is [0, " + std::to_string(textureDbg.mipLevels) +
                "), but [" + std::to_string(subresource->baseMipLevel) + ", " +
                std::to_string(subresource->baseMipLevel + subresource->numMipLevels) + ") was specified"
            );
        }

        if (subresource->numArrayLayers == 0)
        {
            LLGL_DBG_WARN(
                WarningType::PointlessOperation,
                "generating MIP-maps with a total number of 0 array layers for texture has no effect"
            );
        }
        else if (subresource->baseArrayLayer + subresource->numArrayLayers > textureDbg.desc.arrayLayers)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot generate MIP-maps for texture with subresource being out of bounds: "
                "array layer range is [0, " + std::to_string(textureDbg.desc.arrayLayers) +
                "), but [" + std::to_string(subresource->baseArrayLayer) + ", " +
                std::to_string(subresource->baseArrayLayer + subresource->numArrayLayers) + ") was specified"
            );
        }
    }
    else
    {
        /* Validate for entire MIP chain */
        if (textureDbg.mipLevels == 1)
        {
            LLGL_DBG_WARN(
                WarningType::PointlessOperation,
                "generate MIP-maps for texture with only a single MIP-map has no effect"
            );
        }
    }
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
            "viewport exceeded maximal size: [" + std::to_string(w) + " x " + std::to_string(h) +
            "] specified but limit is [" + std::to_string(limits_.maxViewportSize[0]) + " x " + std::to_string(limits_.maxViewportSize[1]) + "])"
        );
    }
}

void DbgCommandBuffer::ValidateAttachmentClear(const AttachmentClear& attachment)
{
    if (bindings_.swapChain != nullptr)
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
    if (auto pso = bindings_.pipelineState)
    {
        if (pso->isGraphicsPSO && bindings_.numVertexBuffers > 0)
        {
            if (auto vertexShader = pso->graphicsDesc.vertexShader)
            {
                auto vertexShaderDbg = LLGL_CAST(const DbgShader*, vertexShader);
                const auto& inputAttribs = vertexShaderDbg->desc.vertex.inputAttribs;
                if (!inputAttribs.empty())
                    ValidateVertexLayoutAttributes(inputAttribs, bindings_.vertexBuffers, bindings_.numVertexBuffers);
            }
        }
    }
}

void DbgCommandBuffer::ValidateVertexLayoutAttributes(const ArrayView<VertexAttribute>& shaderVertexAttribs, DbgBuffer* const * vertexBuffers, std::uint32_t numVertexBuffers)
{
    /* Check if all vertex attributes are served by active vertex buffer(s) */
    std::size_t attribIndex = 0;

    for (std::uint32_t bufferIndex = 0; attribIndex < shaderVertexAttribs.size() && bufferIndex < numVertexBuffers; ++bufferIndex)
    {
        /* Compare remaining shader attributes with next vertex buffer attributes */
        const auto& bufferVertexAttribs = vertexBuffers[bufferIndex]->desc.vertexAttribs;

        for (std::size_t i = 0; i < bufferVertexAttribs.size() && attribIndex < shaderVertexAttribs.size(); ++i, ++attribIndex)
        {
            /* Compare current vertex attributes */
            const auto& attribLhs = shaderVertexAttribs[attribIndex];
            const auto& attribRhs = bufferVertexAttribs[i];

            if (attribLhs != attribRhs)
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex layout mismatch between shader program and vertex buffer(s)");
        }
    }

    if (attribIndex < shaderVertexAttribs.size())
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
        if (auto vertexShaderDbg = bindings_.vertexShader)
        {
            if (auto vertexID = vertexShaderDbg->GetVertexID())
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
        if (auto vertexShaderDbg = bindings_.vertexShader)
        {
            if (auto instanceID = vertexShaderDbg->GetInstanceID())
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
    AssertViewportBound();
    ValidateDynamicStates();
    ValidateVertexLayout();
    ValidateNumVertices(numVertices);
    ValidateNumInstances(numInstances);
    ValidateVertexID(firstVertex);
    ValidateInstanceID(firstInstance);
    ValidateBindingTable();

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
    AssertViewportBound();
    ValidateDynamicStates();
    ValidateVertexLayout();
    ValidateNumVertices(numVertices);
    ValidateNumInstances(numInstances);
    ValidateInstanceID(firstInstance);
    ValidateBindingTable();

    if (bindings_.indexBuffer)
    {
        if (bindings_.indexBufferFormatSize > 0)
        {
            ValidateVertexLimit(
                numVertices + firstIndex,
                static_cast<std::uint32_t>((bindings_.indexBuffer->desc.size - bindings_.indexBufferOffset) / bindings_.indexBufferFormatSize)
            );
        }
        else
        {
            ValidateVertexLimit(
                numVertices + firstIndex,
                static_cast<std::uint32_t>(bindings_.indexBuffer->elements)
            );
        }
    }
}

void DbgCommandBuffer::ValidateVertexLimit(std::uint32_t vertexCount, std::uint32_t vertexLimit)
{
    if (vertexCount > vertexLimit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "vertex count out of bounds: " + std::to_string(vertexCount) +
            " specified but limit is " + std::to_string(vertexLimit)
        );
    }
}

void DbgCommandBuffer::ValidateThreadGroupLimit(std::uint32_t size, std::uint32_t limit)
{
    if (size > limit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "thread group size X out of bounds: " + std::to_string(size) +
            " specified but limit is " + std::to_string(limit)
        );
    }
}

void DbgCommandBuffer::ValidateAttachmentLimit(std::uint32_t attachmentIndex, std::uint32_t attachmentUpperBound)
{
    if (attachmentIndex >= attachmentUpperBound)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "color attachment index out of bounds: " + std::to_string(attachmentIndex) +
            " specified but upper bound is " + std::to_string(attachmentUpperBound)
        );
    }
}

void DbgCommandBuffer::ValidateDescriptorSetIndex(std::uint32_t setIndex, std::uint32_t setUpperBound, const char* resourceHeapName)
{
    if (setIndex >= setUpperBound)
    {
        std::string s = "descriptor set index out of bounds: " + std::to_string(setIndex) + " specified but upper bound is " + std::to_string(setUpperBound);

        if (resourceHeapName != nullptr && *resourceHeapName != '\0')
        {
            s += " for resource heap \"";
            s += resourceHeapName;
            s += '\"';
        }

        LLGL_DBG_ERROR(ErrorType::InvalidArgument, s);
    }
}

static const char* BindFlagToString(long bindFlag)
{
    switch (bindFlag)
    {
        case BindFlags::VertexBuffer:           return "VertexBuffer";
        case BindFlags::IndexBuffer:            return "IndexBuffer";
        case BindFlags::ConstantBuffer:         return "ConstantBuffer";
        case BindFlags::StreamOutputBuffer:     return "StreamOutputBuffer";
        case BindFlags::IndirectBuffer:         return "IndirectBuffer";
        case BindFlags::Sampled:                return "Sampled";
        case BindFlags::Storage:                return "Storage";
        case BindFlags::ColorAttachment:        return "ColorAttachment";
        case BindFlags::DepthStencilAttachment: return "DepthStencilAttachment";
        case BindFlags::CombinedSampler:        return "CombinedSampler";
        case BindFlags::CopySrc:                return "CopySrc";
        case BindFlags::CopyDst:                return "CopyDst";
        default:                                return nullptr;
    }
}

static std::string BindFlagsToStringList(long bindFlags)
{
    std::string s;

    for (long i = 0; i < sizeof(bindFlags)*8; ++i)
    {
        if (((bindFlags >> i) & 0x1) != 0)
        {
            /* Append comma for list representation */
            if (!s.empty())
                s += ", ";

            const long bitmask = (bindFlags & (0x1u << i));
            if (auto flagStr = BindFlagToString(bitmask))
            {
                s += "LLGL::BindFlags::";
                s += flagStr;
            }
            else
                s += IntToHex(bitmask);
        }
    }

    return s;
}

void DbgCommandBuffer::ValidateBindFlags(long resourceFlags, long bindFlags, long validFlags, const char* resourceName)
{
    /* Determine invalid and missing bit flags */
    const long invalidFlags = (bindFlags & ~validFlags);
    const long missingFlags = ((resourceFlags & bindFlags) ^ bindFlags) & (~invalidFlags);

    if (invalidFlags != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot bind " + std::string(resourceName != nullptr ? resourceName : "resource") +
            " with the following bind flags: " + BindFlagsToStringList(missingFlags)
        );
    }

    if (missingFlags != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            std::string(resourceName != nullptr ? resourceName : "resource") +
            " was not created with the the following bind flags: " + BindFlagsToStringList(missingFlags)
        );
    }

    /* Validate resource is not bound as both input and output */
    if (HasInputBindFlags(bindFlags) && HasOutputBindFlags(bindFlags))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot bind " + std::string(resourceName != nullptr ? resourceName : "resource") +
            " as both input and output"
        );
    }
}

void DbgCommandBuffer::ValidateBindBufferFlags(DbgBuffer& bufferDbg, long bindFlags)
{
    ValidateBindFlags(bufferDbg.desc.bindFlags, bindFlags, bindFlags, GetLabelOrDefault(bufferDbg.label, "LLGL::Buffer"));
}

void DbgCommandBuffer::ValidateBindTextureFlags(DbgTexture& textureDbg, long bindFlags)
{
    ValidateBindFlags(textureDbg.desc.bindFlags, bindFlags, bindFlags, GetLabelOrDefault(textureDbg.label, "LLGL::Texture"));
}

void DbgCommandBuffer::ValidateTextureRegion(DbgTexture& textureDbg, const TextureRegion& region)
{
    /* Validate MIP-map range */
    if (region.subresource.numMipLevels == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with MIP-map count of 0"
        );
    }
    else if (region.subresource.numMipLevels > 1)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with MIP-map count greater than 1"
        );
    }
    else if (region.subresource.baseMipLevel + region.subresource.numMipLevels > textureDbg.mipLevels)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with MIP-map range [" + std::to_string(region.subresource.baseMipLevel) +
            ", +" + std::to_string(region.subresource.numMipLevels) +
            ") for texture with " + std::to_string(textureDbg.mipLevels) + " MIP-maps"
        );
    }

    /* Validate array layer range */
    if (region.subresource.numArrayLayers == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with array count of 0"
        );
    }
    else if (region.subresource.baseArrayLayer + region.subresource.numArrayLayers > textureDbg.desc.arrayLayers)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with array range [" + std::to_string(region.subresource.baseArrayLayer) +
            ", +" + std::to_string(region.subresource.numArrayLayers) +
            ") for texture with " + std::to_string(textureDbg.desc.arrayLayers) + " layers"
        );
    }

    /* Validate extent and offset */
    const auto mipExtent = textureDbg.GetMipExtent(region.subresource.baseMipLevel);

    if (region.extent.width  == 0 ||
        region.extent.height == 0 ||
        region.extent.depth  == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with zero extent (" + std::to_string(region.extent.width) +
            ", " + std::to_string(region.extent.height) + ", " + std::to_string(region.extent.depth) + ")"
        );
    }
    else if (region.offset.x < 0 ||
             region.offset.y < 0 ||
             region.offset.z < 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid texture region with negative offset (" + std::to_string(region.offset.x) +
            ", " + std::to_string(region.offset.y) + ", " + std::to_string(region.offset.z) + ")"
        );
    }
    else
    {
        if (static_cast<std::uint32_t>(region.offset.x) + region.extent.width > mipExtent.width)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid texture region with X-range [" + std::to_string(region.offset.x) + ", +" + std::to_string(region.extent.width) +
                ") out of bounds [0, " + std::to_string(mipExtent.width) + ") for MIP-level " + std::to_string(region.subresource.baseMipLevel)
            );
        }
        if (static_cast<std::uint32_t>(region.offset.y) + region.extent.height > mipExtent.height)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid texture region with Y-range [" + std::to_string(region.offset.y) + ", +" + std::to_string(region.extent.height) +
                ") out of bounds [0, " + std::to_string(mipExtent.height) + ") for MIP-level " + std::to_string(region.subresource.baseMipLevel)
            );
        }
        if (static_cast<std::uint32_t>(region.offset.z) + region.extent.depth > mipExtent.depth)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid texture region with Z-range [" + std::to_string(region.offset.z) + ", +" + std::to_string(region.extent.depth) +
                ") out of bounds [0, " + std::to_string(mipExtent.depth) + ") for MIP-level " + std::to_string(region.subresource.baseMipLevel)
            );
        }
    }
}

void DbgCommandBuffer::ValidateIndexType(const Format format)
{
    if (format != Format::R16UInt && format != Format::R32UInt)
    {
        if (auto formatName = ToString(format))
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid index buffer format: LLGL::Format::" + std::string(formatName));
        else
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "unknown index buffer format: " + std::string(IntToHex(static_cast<std::uint32_t>(format))));
    }
}

void DbgCommandBuffer::ValidateTextureBufferCopyStrides(DbgTexture& textureDbg, std::uint32_t rowStride, std::uint32_t layerStride, const Extent3D& extent)
{
    if (rowStride != 0)
    {
        const std::size_t rowSize = GetMemoryFootprint(textureDbg.desc.format, extent.width);
        if (rowStride < rowSize)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid argument for texture/buffer copy command: 'rowStride' (" + std::to_string(rowStride) + ") "
                "must be greater than or equal to the size of each row in the destination region (rowSize)"
            );
        }
    }
    if (layerStride != 0)
    {
        if (rowStride == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid argument for texture/buffer copy command: 'layerStride' (" + std::to_string(layerStride) + ") "
                "is non-zero while 'rowStride' is zero"
            );
        }
        else if (layerStride % rowStride != 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid argument for texture/buffer copy command: 'layerStride' (" + std::to_string(layerStride) + ") "
                "is not a multiple of 'rowStride' (" + std::to_string(rowStride) + ")"
            );
        }
    }
}

void DbgCommandBuffer::ValidateStageFlags(long stageFlags, long validFlags)
{
    if ((stageFlags & validFlags) == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stage is specified");
    if ((stageFlags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown shader stage flags specified");
}

void DbgCommandBuffer::ValidateBufferRange(DbgBuffer& bufferDbg, std::uint64_t offset, std::uint64_t size, const char* rangeName)
{
    if (offset + size > bufferDbg.desc.size)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            std::string(rangeName != nullptr ? rangeName : "range") + " out of bounds" +
            std::string(bufferDbg.label.empty() ? "" : " for \"" + bufferDbg.label + "\"") + ": " +
            std::to_string(offset + size) + " specified but limit is " + std::to_string(bufferDbg.desc.size)
        );
    }
    else if (size > 0)
    {
        /* Assume buffer to be initialized even if only partially as we cannot keep track of each bit inside the buffer */
        bufferDbg.initialized = true;
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

bool DbgCommandBuffer::ValidateQueryIndex(DbgQueryHeap& queryHeapDbg, std::uint32_t query)
{
    if (query >= queryHeapDbg.states.size())
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "query index out of bounds: " + std::to_string(query) +
            " specified but upper bound is " + std::to_string(queryHeapDbg.states.size())
        );
        return false;
    }
    return true;
}

DbgQueryHeap::State* DbgCommandBuffer::GetAndValidateQueryState(DbgQueryHeap& queryHeapDbg, std::uint32_t query)
{
    if (ValidateQueryIndex(queryHeapDbg, query))
        return &(queryHeapDbg.states[query]);
    else
        return nullptr;
}

void DbgCommandBuffer::ValidateRenderCondition(DbgQueryHeap& queryHeapDbg, std::uint32_t query)
{
    if (!features_.hasRenderCondition)
        LLGL_DBG_ERROR_NOT_SUPPORTED("conditional rendering");
    if (!queryHeapDbg.desc.renderCondition)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot use query heap for conditional rendering that was not created with 'renderCondition' enabled");
}

void DbgCommandBuffer::ValidateRenderTargetRange(DbgRenderTarget& renderTargetDbg, const Offset2D& offset, const Extent2D& extent)
{
    /* Validate extent and offset */
    const Extent2D resolution = renderTargetDbg.GetResolution();

    if (extent.width == 0 || extent.height == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid swap-chain region with zero extent (" + std::to_string(extent.width) + ", " + std::to_string(extent.height) + ")"
        );
    }
    else if (offset.x < 0 || offset.y < 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid swap-chain region with negative offset (" + std::to_string(offset.x) + ", " + std::to_string(offset.y) + ")"
        );
    }
    else
    {
        if (static_cast<std::uint32_t>(offset.x) + extent.width > resolution.width)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid swap-chain region with X-range [" + std::to_string(offset.x) + ", +" + std::to_string(extent.width) +
                ") out of bounds [0, " + std::to_string(resolution.width) + ")"
            );
        }
        if (static_cast<std::uint32_t>(offset.y) + extent.height > resolution.height)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "invalid swap-chain region with Y-range [" + std::to_string(offset.y) + ", +" + std::to_string(extent.height) +
                ") out of bounds [0, " + std::to_string(resolution.height) + ")"
            );
        }
    }
}

void DbgCommandBuffer::ValidateSwapBufferIndex(DbgSwapChain& swapChainDbg, std::uint32_t swapBufferIndex)
{
    if (swapBufferIndex != LLGL_CURRENT_SWAP_INDEX && swapBufferIndex >= swapChainDbg.desc.swapBuffers)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot begin render pass with swap-buffer index " + std::to_string(swapBufferIndex) +
            " for swap-chain with only " + std::to_string(swapChainDbg.desc.swapBuffers) + " buffer(s)"
        );
    }
}

void DbgCommandBuffer::ValidateStreamOutputs(std::uint32_t numBuffers)
{
    if (numBuffers > limits_.maxStreamOutputs)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "maximum number of stream-output buffers exceeded limit: " +
            std::to_string(numBuffers) + " specified but limit is " + std::to_string(limits_.maxStreamOutputs)
        );
    }
}

const BindingDescriptor* DbgCommandBuffer::GetAndValidateResourceDescFromPipeline(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t descriptor, Resource& resource)
{
    if (descriptor >= pipelineLayoutDbg.desc.bindings.size())
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "descriptor index out of bounds: " + std::to_string(descriptor) +
            " specified but upper bound is " + std::to_string(pipelineLayoutDbg.desc.bindings.size())
        );
        return nullptr;
    }
    auto* bindingDesc = &(pipelineLayoutDbg.desc.bindings[descriptor]);
    if (bindingDesc->type != resource.GetResourceType())
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "invalid resource type in pipeline for descriptor[" + std::to_string(descriptor) + "]: " +
            std::string(ToString(resource.GetResourceType())) + " specified but expected " + std::string(ToString(bindingDesc->type))
        );
        return nullptr;
    }
    return bindingDesc;
}

void DbgCommandBuffer::ValidateUniforms(const DbgPipelineLayout& pipelineLayoutDbg, std::uint32_t first, std::uint16_t dataSize)
{
    /* Validate input data size */
    if (dataSize == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot set uniforms with a data size of 0"
        );
    }
    else if (dataSize % 4 != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot set uniforms with a data size of " + std::to_string(dataSize) + "; must be a multiple of 4"
        );
    }

    /* Validate number of uniforms */
    if (first < pipelineLayoutDbg.desc.uniforms.size())
    {
        for (; first < pipelineLayoutDbg.desc.uniforms.size(); ++first)
        {
            /* Get size information for current uniform that is to be updated */
            const auto& uniformDesc = pipelineLayoutDbg.desc.uniforms[first];
            const auto uniformTypeSize = static_cast<decltype(dataSize)>(GetUniformTypeSize(uniformDesc.type, uniformDesc.arraySize));

            if (dataSize >= uniformTypeSize)
                dataSize -= uniformTypeSize;
            else
                break;
        }

        if (dataSize > 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot set uniforms with data size of " + std::to_string(dataSize) + "; exceeded limit by " +
                std::to_string(dataSize) + (dataSize == 1 ? " byte" : " bytes")
            );
        }
    }
    else
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "uniform index out of bounds: " + std::to_string(first) +
            " specified but upper bound is " + std::to_string(pipelineLayoutDbg.desc.uniforms.size())
        );
    }
}

void DbgCommandBuffer::ValidateDynamicStates()
{
    if (!bindings_.blendFactorSet)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidState,
            "blend factor is not set; missing call to <LLGL::CommandBuffer::SetBlendFactor>"
            " or PSO must be created with 'LLGL::BlendDescriptor::blendFactorDynamic' being disabled"
        );
    }
    if (!bindings_.stencilRefSet)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidState,
            "stencil reference blend factor is not set; missing call to <LLGL::CommandBuffer::SetStencilReference>"
            " or PSO must be created with 'LLGL::StencilDescriptor::referenceDynamic' being disabled"
        );
    }
}

// Returns a descriptive string for the specified binding
static std::string GetBindingDescStr(const BindingDescriptor& binding)
{
    std::string s;

    s = "slot ";
    s += std::to_string(binding.slot.index);

    if (binding.slot.set != 0)
    {
        s += ", set ";
        s += std::to_string(binding.slot.set);
    }

    if (!binding.name.empty())
    {
        s += ", name '";
        s += binding.name;
        s += "'";
    }

    return s;
}

static std::string GetPipelineBindingDescStr(const DbgPipelineState& pso, const PipelineLayoutDescriptor& layoutDesc, std::size_t bindingIndex)
{
    std::string s;

    s = "missing descriptor [";
    s += std::to_string(bindingIndex);
    s += "] in pipeline state ";
    if (!pso.label.empty())
    {
        s += "'";
        s += pso.label;
        s += "' ";
    }
    s += "for binding (";
    s += GetBindingDescStr(layoutDesc.bindings[bindingIndex]);
    s += ")";

    return s;
}

void DbgCommandBuffer::ValidateBindingTable()
{
    auto ValidateBindingTableWithLayout = [this](const DbgPipelineState& pso, const Bindings::BindingTable& table, const PipelineLayoutDescriptor& layoutDesc)
    {
        LLGL_ASSERT(table.resources.size() == layoutDesc.bindings.size());
        for_range(i, table.resources.size())
        {
            if (table.resources[i] == nullptr)
                LLGL_DBG_ERROR(ErrorType::InvalidState, GetPipelineBindingDescStr(pso, layoutDesc, i));
        }
    };

    if (auto pso = bindings_.pipelineState)
    {
        if (auto pipelineLayout = pso->pipelineLayout)
            ValidateBindingTableWithLayout(*pso, bindings_.bindingTable, pipelineLayout->desc);
    }
}

DbgPipelineState* DbgCommandBuffer::AssertAndGetGraphicsPSO()
{
    if (bindings_.pipelineState == nullptr)
    {
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no graphics pipeline is bound; missing call to <LLGL::CommandBuffer::SetPipelineState>");
        return nullptr;
    }
    else if (!bindings_.pipelineState->isGraphicsPSO)
    {
        LLGL_DBG_ERROR(ErrorType::InvalidState, "compute pipeline is bound but graphics pipeline is required");
        return nullptr;
    }
    return bindings_.pipelineState;
}

DbgPipelineState* DbgCommandBuffer::AssertAndGetComputePSO()
{
    if (bindings_.pipelineState == nullptr)
    {
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no compute pipeline is bound; missing call to <LLGL::CommandBuffer::SetPipelineState>");
        return nullptr;
    }
    else if (bindings_.pipelineState->isGraphicsPSO)
    {
        LLGL_DBG_ERROR(ErrorType::InvalidState, "graphics pipeline is bound but compute pipeline is required");
        return nullptr;
    }
    return bindings_.pipelineState;
}

void DbgCommandBuffer::AssertRecording()
{
    if (!states_.recording)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "command buffer must be in record mode; missing call to <LLGL::CommandBuffer::Begin>");
}

void DbgCommandBuffer::AssertInsideRenderPass()
{
    if (!states_.insideRenderPass)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "operation is only allowed inside a render pass; missing call to <LLGL::CommandBuffer::BeginRenderPass>");
}

void DbgCommandBuffer::AssertGraphicsPipelineBound()
{
    AssertAndGetGraphicsPSO();
}

void DbgCommandBuffer::AssertComputePipelineBound()
{
    AssertAndGetComputePSO();
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
            if (buffer->IsMappedForCPUAccess())
                LLGL_DBG_ERROR(ErrorType::InvalidState, "vertex buffer used for drawing while being mapped to CPU memory space");
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
        if (buffer->IsMappedForCPUAccess())
            LLGL_DBG_ERROR(ErrorType::InvalidState, "index buffer used for drawing while being mapped to CPU memory space");
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no index buffer is bound");
}

void DbgCommandBuffer::AssertViewportBound()
{
    if (bindings_.numViewports == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidState, "no viewports are bound");
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

void DbgCommandBuffer::AssertNullPointer(const void* ptr, const char* name)
{
    if (ptr == nullptr)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "argument '" + std::string(name) + "' must not be a null pointer"
        );
    }
}

void DbgCommandBuffer::WarnImproperVertices(const std::string& topologyName, std::uint32_t unusedVertices)
{
    LLGL_DBG_WARN(
        WarningType::ImproperArgument,
        "improper number of vertices for " + topologyName + " (" + std::to_string(unusedVertices) +
        " unused " + std::string(unusedVertices > 1 ? "vertices" : "vertex") + ")"
    );
}

void DbgCommandBuffer::ResetStates()
{
    /* Reset all counters of frame profile, bindings, and other command buffer states */
    ::memset(profile_.values, 0, sizeof(profile_.values));
    ::memset(&bindings_, 0, sizeof(bindings_));
    ::memset(&states_, 0, sizeof(states_));
}

void DbgCommandBuffer::ResetRecords()
{
    records_.swapChainFrames.clear();
}

void DbgCommandBuffer::ResetBindingTable(const DbgPipelineLayout* pipelineLayoutDbg)
{
    auto ResetBindingTableWithLayout = [](Bindings::BindingTable& table, const PipelineLayoutDescriptor& layoutDesc)
    {
        table.resourceHeap = nullptr;
        table.resources.clear();
        table.resources.resize(layoutDesc.bindings.size(), nullptr);
        table.uniforms.clear();
        table.uniforms.resize(layoutDesc.uniforms.size(), 0);
    };

    auto ResetBindingTableZero = [](Bindings::BindingTable& table)
    {
        table.resourceHeap = nullptr;
        table.resources.clear();
        table.uniforms.clear();
    };

    if (pipelineLayoutDbg != nullptr)
        ResetBindingTableWithLayout(bindings_.bindingTable, pipelineLayoutDbg->desc);
    else
        ResetBindingTableZero(bindings_.bindingTable);
}

void DbgCommandBuffer::StartTimer(const char* annotation)
{
    queryTimerPool_.Start(annotation);
}

void DbgCommandBuffer::EndTimer()
{
    queryTimerPool_.Stop();
}


} // /namespace LLGL



// ================================================================================
