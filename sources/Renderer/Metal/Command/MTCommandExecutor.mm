/*
 * MTCommandExecutor.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#import <MetalKit/MetalKit.h>

#include "MTCommandExecutor.h"
#include "MTCommandContext.h"
#include "MTCommand.h"
#include "MTMultiSubmitCommandBuffer.h"

#include "../MTSwapChain.h"
#include "../Texture/MTRenderTarget.h"
#include "../RenderState/MTGraphicsPSO.h"
#include "../RenderState/MTComputePSO.h"
#include "../RenderState/MTResourceHeap.h"
#include "../MTTypes.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include <LLGL/Utils/ForRange.h>
#include <LLGL/Backend/Metal/NativeCommand.h>
#include <LLGL/TypeInfo.h>


namespace LLGL
{


static std::size_t ExecuteMTCommand(const MTOpcode opcode, const void* pc, MTCommandContext& context)
{
    switch (opcode)
    {
        case MTOpcodeExecute:
        {
            auto* cmd = static_cast<const MTCmdExecute*>(pc);
            ExecuteMTMultiSubmitCommandBuffer(*(cmd->commandBuffer), context);
            return sizeof(*cmd);
        }
        case MTOpcodeCopyBuffer:
        {
            auto* cmd = static_cast<const MTCmdCopyBuffer*>(pc);
            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            [blitEncoder
                copyFromBuffer:     cmd->sourceBuffer
                sourceOffset:       cmd->sourceOffset
                toBuffer:           cmd->destinationBuffer
                destinationOffset:  cmd->destinationOffset
                size:               cmd->size
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeCopyBufferFromTexture:
        {
            auto* cmd = static_cast<const MTCmdCopyBufferFromTexture*>(pc);
            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            for_range(arrayLayer, cmd->layerCount)
            {
                [blitEncoder
                    copyFromTexture:            cmd->sourceTexture
                    sourceSlice:                cmd->sourceSlice + arrayLayer
                    sourceLevel:                cmd->sourceLevel
                    sourceOrigin:               cmd->sourceOrigin
                    sourceSize:                 cmd->sourceSize
                    toBuffer:                   cmd->destinationBuffer
                    destinationOffset:          cmd->destinationOffset + cmd->destinationBytesPerImage * arrayLayer
                    destinationBytesPerRow:     cmd->destinationBytesPerRow
                    destinationBytesPerImage:   cmd->destinationBytesPerImage
                ];
            }
            return sizeof(*cmd);
        }
        case MTOpcodeCopyTexture:
        {
            auto* cmd = static_cast<const MTCmdCopyTexture*>(pc);
            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            [blitEncoder
                copyFromTexture:    cmd->sourceTexture
                sourceSlice:        cmd->sourceSlice
                sourceLevel:        cmd->sourceLevel
                sourceOrigin:       cmd->sourceOrigin
                sourceSize:         cmd->sourceSize
                toTexture:          cmd->destinationTexture
                destinationSlice:   cmd->destinationSlice
                destinationLevel:   cmd->destinationLevel
                destinationOrigin:  cmd->destinationOrigin
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeCopyTextureFromBuffer:
        {
            auto* cmd = static_cast<const MTCmdCopyTextureFromBuffer*>(pc);
            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            for_range(arrayLayer, cmd->layerCount)
            {
                [blitEncoder
                    copyFromBuffer:         cmd->sourceBuffer
                    sourceOffset:           cmd->sourceOffset + cmd->sourceBytesPerImage * arrayLayer
                    sourceBytesPerRow:      cmd->sourceBytesPerRow
                    sourceBytesPerImage:    cmd->sourceBytesPerImage
                    sourceSize:             cmd->sourceSize
                    toTexture:              cmd->destinationTexture
                    destinationSlice:       cmd->destinationSlice + arrayLayer
                    destinationLevel:       cmd->destinationLevel
                    destinationOrigin:      cmd->destinationOrigin
                ];
            }
            return sizeof(*cmd);
        }
        case MTOpcodeCopyTextureFromFramebuffer:
        {
            auto* cmd = static_cast<const MTCmdCopyTextureFromFramebuffer*>(pc);

            /* Get source texture from current drawable */
            id<MTLTexture> drawableTexture = [[context.GetCurrentDrawableView() currentDrawable] texture];

            /* Source and target texture formats must match for 'copyFromTexture', so create texture view on mismatch */
            id<MTLTexture> sourceTexture;
            const bool isTextureViewRequired = ([drawableTexture pixelFormat] != [cmd->destinationTexture pixelFormat]);
            if (isTextureViewRequired)
                sourceTexture = [drawableTexture newTextureViewWithPixelFormat:[cmd->destinationTexture pixelFormat]];
            else
                sourceTexture = drawableTexture;

            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            [blitEncoder
                copyFromTexture:    sourceTexture
                sourceSlice:        0
                sourceLevel:        0
                sourceOrigin:       cmd->sourceOrigin
                sourceSize:         cmd->sourceSize
                toTexture:          cmd->destinationTexture
                destinationSlice:   cmd->destinationSlice
                destinationLevel:   cmd->destinationLevel
                destinationOrigin:  cmd->destinationOrigin
            ];

            /* Decrement reference counter for temporary texture */
            if (isTextureViewRequired)
                [sourceTexture release];
            return sizeof(*cmd);
        }
        case MTOpcodeGenerateMipmaps:
        {
            auto* cmd = static_cast<const MTCmdGenerateMipmaps*>(pc);
            id<MTLBlitCommandEncoder> blitEncoder = context.BindBlitEncoder();
            [blitEncoder generateMipmapsForTexture:cmd->texture];
            return sizeof(*cmd);
        }
        case MTOpcodeSetGraphicsPSO:
        {
            auto* cmd = static_cast<const MTCmdSetGraphicsPSO*>(pc);
            context.SetGraphicsPSO(cmd->graphicsPSO);
            return sizeof(*cmd);
        }
        case MTOpcodeSetComputePSO:
        {
            auto* cmd = static_cast<const MTCmdSetComputePSO*>(pc);
            context.SetComputePSO(cmd->computePSO);
            return sizeof(*cmd);
        }
        case MTOpcodeSetViewports:
        {
            auto* cmd = static_cast<const MTCmdSetViewports*>(pc);
            context.SetViewports(reinterpret_cast<const Viewport*>(cmd + 1), cmd->count);
            return (sizeof(*cmd) + sizeof(Viewport)*cmd->count);
        }
        case MTOpcodeSetScissorRects:
        {
            auto* cmd = static_cast<const MTCmdSetScissorRects*>(pc);
            context.SetScissorRects(reinterpret_cast<const Scissor*>(cmd + 1), cmd->count);
            return (sizeof(*cmd) + sizeof(Scissor)*cmd->count);
        }
        case MTOpcodeSetBlendColor:
        {
            auto* cmd = static_cast<const MTCmdSetBlendColor*>(pc);
            context.SetBlendColor(cmd->blendColor);
            return sizeof(*cmd);
        }
        case MTOpcodeSetStencilRef:
        {
            auto* cmd = static_cast<const MTCmdSetStencilRef*>(pc);
            context.SetStencilRef(cmd->ref, cmd->face);
            return sizeof(*cmd);
        }
        case MTOpcodeSetUniforms:
        {
            auto* cmd = static_cast<const MTCmdSetUniforms*>(pc);
            context.SetUniforms(cmd->first, cmd + 1, cmd->dataSize);
            return (sizeof(*cmd) + cmd->dataSize);
        }
        case MTOpcodeSetVertexBuffers:
        {
            auto* cmd = static_cast<const MTCmdSetVertexBuffers*>(pc);
            auto* bufferIds = reinterpret_cast<const id<MTLBuffer>*>(cmd + 1);
            auto* bufferOffsets = reinterpret_cast<const NSUInteger*>(bufferIds + cmd->count);
            context.SetVertexBuffers(bufferIds, bufferOffsets, cmd->count);
            return (sizeof(*cmd) + (sizeof(id) + sizeof(NSUInteger))*cmd->count);
        }
        case MTOpcodeSetIndexBuffer:
        {
            auto* cmd = static_cast<const MTCmdSetIndexBuffer*>(pc);
            context.SetIndexStream(cmd->buffer, cmd->offset, cmd->indexType16Bits);
            return sizeof(*cmd);
        }
        case MTOpcodeSetResourceHeap:
        {
            auto* cmd = static_cast<const MTCmdSetResourceHeap*>(pc);
            if (MTPipelineState* boundPipelineState = context.GetBoundPipelineState())
            {
                if (boundPipelineState->IsGraphicsPSO())
                {
                    if (cmd->resourceHeap->HasGraphicsResources())
                        context.SetGraphicsResourceHeap(cmd->resourceHeap, cmd->descriptorSet);
                }
                else
                {
                    if (cmd->resourceHeap->HasComputeResources())
                        context.SetComputeResourceHeap(cmd->resourceHeap, cmd->descriptorSet);
                }
            }
            return sizeof(*cmd);
        }
        case MTOpcodeSetResource:
        {
            auto* cmd = static_cast<const MTCmdSetResource*>(pc);
            context.SetResource(cmd->descriptor, *(cmd->resource));
            return sizeof(*cmd);
        }
        case MTOpcodeBeginRenderPass:
        {
            auto* cmd = static_cast<const MTCmdBeginRenderPass*>(pc);
            auto* clearValues = reinterpret_cast<const ClearValue*>(cmd + 1);
            context.BeginRenderPass(cmd->renderTarget, cmd->renderPass, cmd->numClearValues, clearValues);
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case MTOpcodeEndRenderPass:
        {
            context.EndRenderPass();
            return 0;
        }
        case MTOpcodeClearRenderPass:
        {
            auto* cmd = static_cast<const MTCmdClearRenderPass*>(pc);

            /* Make new render pass descriptor with current clear values */
            MTLRenderPassDescriptor* renderPassDesc = context.CopyRenderPassDesc();

            if ((cmd->flags & ClearFlags::Color) != 0)
            {
                /* Clear color buffer */
                auto* colorBuffers = reinterpret_cast<const std::uint32_t*>(cmd + 1);
                auto* clearColors = reinterpret_cast<const MTLClearColor*>(colorBuffers + cmd->numAttachments);
                for_range(i, cmd->numColorAttachments)
                {
                    renderPassDesc.colorAttachments[colorBuffers[i]].loadAction = MTLLoadActionClear;
                    renderPassDesc.colorAttachments[colorBuffers[i]].clearColor = clearColors[i];
                }
            }

            if ((cmd->flags & ClearFlags::Depth) != 0)
            {
                /* Clear depth buffer */
                renderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
                renderPassDesc.depthAttachment.clearDepth = cmd->clearDepth;
            }

            if ((cmd->flags & ClearFlags::Stencil) != 0)
            {
                /* Clear stencil buffer */
                renderPassDesc.stencilAttachment.loadAction     = MTLLoadActionClear;
                renderPassDesc.stencilAttachment.clearStencil   = cmd->clearStencil;
            }

            /* Begin with new render pass to clear buffers */
            context.UpdateRenderPass(renderPassDesc);
            [renderPassDesc release];

            return (sizeof(*cmd) + (sizeof(std::uint32_t) + sizeof(MTLClearColor))*cmd->numAttachments);
        }
        case MTOpcodeDraw:
        {
            auto* cmd = static_cast<const MTCmdDraw*>(pc);
            const NSUInteger numPatchControlPoints = context.GetNumPatchControlPoints();
            if (numPatchControlPoints > 0)
            {
                const NSUInteger patchStart = (static_cast<NSUInteger>(cmd->vertexStart) / numPatchControlPoints);
                const NSUInteger patchCount = (static_cast<NSUInteger>(cmd->vertexCount) / numPatchControlPoints);

                id<MTLRenderCommandEncoder> renderEncoder = context.DispatchTessellationAndGetRenderEncoder(patchCount, cmd->instanceCount);
                [renderEncoder
                    drawPatches:            numPatchControlPoints
                    patchStart:             patchStart
                    patchCount:             patchCount
                    patchIndexBuffer:       nil
                    patchIndexBufferOffset: 0
                    instanceCount:          cmd->instanceCount
                    baseInstance:           cmd->baseInstance
                ];
            }
            else
            {
                id<MTLRenderCommandEncoder> renderEncoder = context.FlushAndGetRenderEncoder();
                if (cmd->baseInstance != 0)
                {
                    /* Supported since iOS 9.0 */
                    [renderEncoder
                        drawPrimitives: context.GetPrimitiveType()
                        vertexStart:    cmd->vertexStart
                        vertexCount:    cmd->vertexCount
                        instanceCount:  cmd->instanceCount
                        baseInstance:   cmd->baseInstance
                    ];
                }
                else if (cmd->instanceCount != 1)
                {
                    /* Supported since iOS 8.0 */
                    [renderEncoder
                        drawPrimitives: context.GetPrimitiveType()
                        vertexStart:    cmd->vertexStart
                        vertexCount:    cmd->vertexCount
                        instanceCount:  cmd->instanceCount
                    ];
                }
                else
                {
                    /* Supported since iOS 8.0 */
                    [renderEncoder
                        drawPrimitives: context.GetPrimitiveType()
                        vertexStart:    cmd->vertexStart
                        vertexCount:    cmd->vertexCount
                    ];
                }
            }
            return sizeof(*cmd);
        }
        case MTOpcodeDrawIndexed:
        {
            auto* cmd = static_cast<const MTCmdDrawIndexed*>(pc);
            const NSUInteger numPatchControlPoints = context.GetNumPatchControlPoints();
            if (numPatchControlPoints > 0)
            {
                const NSUInteger patchStart = (static_cast<NSUInteger>(cmd->firstIndex) / numPatchControlPoints);
                const NSUInteger patchCount = (static_cast<NSUInteger>(cmd->indexCount) / numPatchControlPoints);

                id<MTLRenderCommandEncoder> renderEncoder = context.DispatchTessellationAndGetRenderEncoder(patchCount, cmd->instanceCount);
                [renderEncoder
                    drawIndexedPatches:             numPatchControlPoints
                    patchStart:                     patchStart
                    patchCount:                     patchCount
                    patchIndexBuffer:               nil
                    patchIndexBufferOffset:         0
                    controlPointIndexBuffer:        context.GetIndexBuffer()
                    controlPointIndexBufferOffset:  context.GetIndexBufferOffset(cmd->firstIndex)
                    instanceCount:                  cmd->instanceCount
                    baseInstance:                   cmd->baseInstance
                ];
            }
            else
            {
                id<MTLRenderCommandEncoder> renderEncoder = context.FlushAndGetRenderEncoder();
                if (cmd->baseVertex != 0 || cmd->baseInstance != 0)
                {
                    /* Supported since iOS 9.0 */
                    [renderEncoder
                        drawIndexedPrimitives:  context.GetPrimitiveType()
                        indexCount:             cmd->indexCount
                        indexType:              context.GetIndexType()
                        indexBuffer:            context.GetIndexBuffer()
                        indexBufferOffset:      context.GetIndexBufferOffset(cmd->firstIndex)
                        instanceCount:          cmd->instanceCount
                        baseVertex:             cmd->baseVertex
                        baseInstance:           cmd->baseInstance
                    ];
                }
                else if (cmd->instanceCount != 1)
                {
                    /* Supported since iOS 8.0 */
                    [renderEncoder
                        drawIndexedPrimitives:  context.GetPrimitiveType()
                        indexCount:             cmd->indexCount
                        indexType:              context.GetIndexType()
                        indexBuffer:            context.GetIndexBuffer()
                        indexBufferOffset:      context.GetIndexBufferOffset(cmd->firstIndex)
                        instanceCount:          cmd->instanceCount
                    ];
                }
                else
                {
                    /* Supported since iOS 8.0 */
                    [renderEncoder
                        drawIndexedPrimitives:  context.GetPrimitiveType()
                        indexCount:             cmd->indexCount
                        indexType:              context.GetIndexType()
                        indexBuffer:            context.GetIndexBuffer()
                        indexBufferOffset:      context.GetIndexBufferOffset(cmd->firstIndex)
                    ];
                }
            }
            return sizeof(*cmd);
        }
        case MTOpcodeDispatchThreadgroups:
        {
            auto* cmd = static_cast<const MTCmdDispatchThreads*>(pc);
            id<MTLComputeCommandEncoder> computeEncoder = context.FlushAndGetComputeEncoder();
            [computeEncoder
                dispatchThreadgroups:   cmd->threadgroups
                threadsPerThreadgroup:  context.GetThreadsPerThreadgroup() // current PSO parameter
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDispatchThreadgroupsIndirect:
        {
            auto* cmd = static_cast<const MTCmdDispatchThreadsIndirect*>(pc);
            id<MTLComputeCommandEncoder> computeEncoder = context.FlushAndGetComputeEncoder();
            [computeEncoder
                dispatchThreadgroupsWithIndirectBuffer: cmd->indirectBuffer
                indirectBufferOffset:                   cmd->indirectBufferOffset
                threadsPerThreadgroup:                  context.GetThreadsPerThreadgroup() // current PSO parameter
            ];
            return sizeof(*cmd);
        }
        case MTOpcodePushDebugGroup:
        {
            auto* cmd = static_cast<const MTCmdPushDebugGroup*>(pc);
            #ifdef LLGL_GLEXT_DEBUG
            [context.GetCommandBuffer() pushDebugGroup:[NSString stringWithUTF8String:reinterpret_cast<const char*>(cmd + 1)]];
            #endif
            return (sizeof(*cmd) + cmd->length + 1);
        }
        case MTOpcodePopDebugGroup:
        {
            #ifdef LLGL_GLEXT_DEBUG
            [context.GetCommandBuffer() popDebugGroup];
            #endif
            return 0;
        }
        case MTOpcodePresentDrawables:
        {
            auto* cmd = static_cast<const MTCmdPresentDrawables*>(pc);
            auto* views = reinterpret_cast<MTKView* const*>(cmd + 1);
            for_range(i, cmd->count)
                [context.GetCommandBuffer() presentDrawable:views[i].currentDrawable];
            return (sizeof(*cmd) + sizeof(id)*cmd->count);
        }
        case MTOpcodeFlush:
        {
            context.Flush();
            return 0;
        }
        default:
            return 0;
    }
}

static void ExecuteMTCommandsEmulated(const MTVirtualCommandBuffer& virtualCmdBuffer, MTCommandContext& context)
{
    virtualCmdBuffer.Run(ExecuteMTCommand, context);
}

void ExecuteMTMultiSubmitCommandBuffer(const MTMultiSubmitCommandBuffer& cmdBuffer, MTCommandContext& context)
{
    /* Emulate execution of Metal commands */
    ExecuteMTCommandsEmulated(cmdBuffer.GetVirtualCommandBuffer(), context);
}

void ExecuteMTCommandBuffer(const MTCommandBuffer& cmdBuffer, MTCommandContext& context)
{
    /* Is this a multi-submit command buffer? */
    if (cmdBuffer.IsMultiSubmitCmdBuffer())
    {
        /* Execute multi-submit command buffer */
        auto& multiSubmitCmdBufferGL = LLGL_CAST(const MTMultiSubmitCommandBuffer&, cmdBuffer);
        ExecuteMTMultiSubmitCommandBuffer(multiSubmitCmdBufferGL, context);
    }
}

void ExecuteNativeMTCommand(const Metal::NativeCommand& cmd, MTCommandContext& context)
{
    switch (cmd.type)
    {
        case Metal::NativeCommandType::ClearCache:
        {
            context.Reset();
        }
        break;

        case Metal::NativeCommandType::TessFactorBuffer:
        {
            context.bindingTable.tessFactorBufferSlot = cmd.tessFactorBuffer.slot;
        }
        break;

        default:
        break;
    }
}


} // /namespace LLGL



// ================================================================================
