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
#include "../MTTypes.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include <LLGL/Utils/ForRange.h>
#include <LLGL/Backend/Metal/NativeCommand.h>


namespace LLGL
{


static std::size_t ExecuteMTCommand(const MTOpcode opcode, const void* pc, MTCommandContext& context)
{
    switch (opcode)
    {
        case MTOpcodeExecute:
        {
            auto cmd = reinterpret_cast<const MTCmdExecute*>(pc);

            return sizeof(*cmd);
        }
        case MTOpcodeCopyBuffer:
        {
            auto cmd = reinterpret_cast<const MTCmdCopyBuffer*>(pc);
            auto blitEncoder = context.BindBlitEncoder();
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
            auto cmd = reinterpret_cast<const MTCmdCopyBufferFromTexture*>(pc);
            auto blitEncoder = context.BindBlitEncoder();
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
            auto cmd = reinterpret_cast<const MTCmdCopyTexture*>(pc);
            auto blitEncoder = context.BindBlitEncoder();
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
            auto cmd = reinterpret_cast<const MTCmdCopyTextureFromBuffer*>(pc);
            auto blitEncoder = context.BindBlitEncoder();
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
            auto cmd = reinterpret_cast<const MTCmdCopyTextureFromFramebuffer*>(pc);

            /* Get source texture from current drawable */
            id<MTLTexture> drawableTexture = [[cmd->sourceView currentDrawable] texture];

            /* Source and target texture formats must match for 'copyFromTexture', so create texture view on mismatch */
            id<MTLTexture> sourceTexture;
            const bool isTextureViewRequired = ([drawableTexture pixelFormat] != [cmd->destinationTexture pixelFormat]);
            if (isTextureViewRequired)
                sourceTexture = [drawableTexture newTextureViewWithPixelFormat:[cmd->destinationTexture pixelFormat]];
            else
                sourceTexture = drawableTexture;

            auto blitEncoder = context.BindBlitEncoder();
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
        case MTOpcodePauseRenderEncoder:
        {
            context.PauseRenderEncoder();
            return 0;
        }
        case MTOpcodeResumeRenderEncoder:
        {
            context.ResumeRenderEncoder();
            return 0;
        }
        case MTOpcodeGenerateMipmaps:
        {
            auto cmd = reinterpret_cast<const MTCmdGenerateMipmaps*>(pc);
            auto blitEncoder = context.BindBlitEncoder();
            [blitEncoder generateMipmapsForTexture:cmd->texture];
            return sizeof(*cmd);
        }
        case MTOpcodeSetGraphicsPSO:
        {
            auto cmd = reinterpret_cast<const MTCmdSetGraphicsPSO*>(pc);
            context.SetGraphicsPSO(cmd->graphicsPSO);
            return sizeof(*cmd);
        }
        case MTOpcodeSetComputePSO:
        {
            auto cmd = reinterpret_cast<const MTCmdSetComputePSO*>(pc);
            context.SetComputePSO(cmd->computePSO);
            return sizeof(*cmd);
        }
        case MTOpcodeSetTessellationPSO:
        {
            auto cmd = reinterpret_cast<const MTCmdSetTessellationPSO*>(pc);
            auto computeEncoder = context.BindComputeEncoder();
            context.RebindResourceHeap(computeEncoder);
            [computeEncoder setComputePipelineState: cmd->tessPipelineState];
            [computeEncoder setBuffer:cmd->tessFactorBuffer offset:0 atIndex:context.bindingTable.tessFactorBufferSlot];
            return sizeof(*cmd);
        }
        case MTOpcodeSetTessellationFactorBuffer:
        {
            auto cmd = reinterpret_cast<const MTCmdSetTessellationFactorBuffer*>(pc);
            auto renderEncoder = context.FlushAndGetRenderEncoder();
            [renderEncoder
                setTessellationFactorBuffer:    cmd->tessFactorBuffer
                offset:                         0
                instanceStride:                 cmd->instanceStride
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeSetViewports:
        {
            auto cmd = reinterpret_cast<const MTCmdSetViewports*>(pc);
            context.SetViewports(reinterpret_cast<const Viewport*>(cmd + 1), cmd->count);
            return (sizeof(*cmd) + sizeof(Viewport)*cmd->count);
        }
        case MTOpcodeSetScissorRects:
        {
            auto cmd = reinterpret_cast<const MTCmdSetScissorRects*>(pc);
            context.SetScissorRects(reinterpret_cast<const Scissor*>(cmd + 1), cmd->count);
            return (sizeof(*cmd) + sizeof(Scissor)*cmd->count);
        }
        case MTOpcodeSetBlendColor:
        {
            auto cmd = reinterpret_cast<const MTCmdSetBlendColor*>(pc);
            context.SetBlendColor(cmd->blendColor);
            return sizeof(*cmd);
        }
        case MTOpcodeSetStencilRef:
        {
            auto cmd = reinterpret_cast<const MTCmdSetStencilRef*>(pc);
            context.SetStencilRef(cmd->ref, cmd->face);
            return sizeof(*cmd);
        }
        case MTOpcodeSetVertexBuffers:
        {
            auto cmd = reinterpret_cast<const MTCmdSetVertexBuffers*>(pc);
            auto bufferIds = reinterpret_cast<const id<MTLBuffer>*>(cmd + 1);
            auto bufferOffsets = reinterpret_cast<const NSUInteger*>(bufferIds + cmd->count);
            context.SetVertexBuffers(bufferIds, bufferOffsets, cmd->count);
            return (sizeof(*cmd) + (sizeof(id) + sizeof(NSUInteger))*cmd->count);
        }
        case MTOpcodeSetGraphicsResourceHeap:
        {
            auto cmd = reinterpret_cast<const MTCmdSetResourceHeap*>(pc);
            context.SetGraphicsResourceHeap(cmd->resourceHeap, cmd->descriptorSet);
            return sizeof(*cmd);
        }
        case MTOpcodeSetComputeResourceHeap:
        {
            auto cmd = reinterpret_cast<const MTCmdSetResourceHeap*>(pc);
            context.SetComputeResourceHeap(cmd->resourceHeap, cmd->descriptorSet);
            return sizeof(*cmd);
        }
        case MTOpcodeSetResource:
        {
            auto cmd = reinterpret_cast<const MTCmdSetResource*>(pc);
            context.SetResource(cmd->descriptor, *(cmd->resource));
            return sizeof(*cmd);
        }
        case MTOpcodeBindSwapChain:
        {
            auto cmd = reinterpret_cast<const MTCmdBindRenderTarget*>(pc);
            auto swapChainMT = LLGL_CAST(MTSwapChain*, cmd->renderTarget);
            if (cmd->renderPass != nullptr)
            {
                auto clearValues = reinterpret_cast<const ClearValue*>(cmd + 1);
                context.BindRenderEncoder(swapChainMT->GetAndUpdateNativeRenderPass(*(cmd->renderPass), cmd->numClearValues, clearValues), true);
            }
            else
                context.BindRenderEncoder(swapChainMT->GetNativeRenderPass(), true);
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case MTOpcodeBindRenderTarget:
        {
            auto cmd = reinterpret_cast<const MTCmdBindRenderTarget*>(pc);
            auto renderTargetMT = LLGL_CAST(MTRenderTarget*, cmd->renderTarget);
            if (cmd->renderPass != nullptr)
            {
                auto clearValues = reinterpret_cast<const ClearValue*>(cmd + 1);
                context.BindRenderEncoder(renderTargetMT->GetAndUpdateNativeRenderPass(*(cmd->renderPass), cmd->numClearValues, clearValues), true);
            }
            else
                context.BindRenderEncoder(renderTargetMT->GetNativeRenderPass(), true);
            return (sizeof(*cmd) + sizeof(ClearValue)*cmd->numClearValues);
        }
        case MTOpcodeClearRenderPass:
        {
            auto cmd = reinterpret_cast<const MTCmdClearRenderPass*>(pc);

            /* Make new render pass descriptor with current clear values */
            auto renderPassDesc = context.CopyRenderPassDesc();

            if ((cmd->flags & ClearFlags::Color) != 0)
            {
                /* Clear color buffer */
                auto colorBuffers = reinterpret_cast<const std::uint32_t*>(cmd + 1);
                auto clearColors = reinterpret_cast<const MTLClearColor*>(colorBuffers + cmd->numAttachments);
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
            context.BindRenderEncoder(renderPassDesc);
            [renderPassDesc release];

            return (sizeof(*cmd) + (sizeof(std::uint32_t) + sizeof(MTLClearColor))*cmd->numAttachments);
        }
        case MTOpcodeDrawPatches:
        {
            auto cmd = reinterpret_cast<const MTCmdDrawPatches*>(pc);
            auto renderEncoder = context.FlushAndGetRenderEncoder();
            [renderEncoder
                drawPatches:            cmd->controlPointCount
                patchStart:             cmd->patchStart
                patchCount:             cmd->patchCount
                patchIndexBuffer:       nil
                patchIndexBufferOffset: 0
                instanceCount:          cmd->instanceCount
                baseInstance:           cmd->baseInstance
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDrawPrimitives:
        {
            auto cmd = reinterpret_cast<const MTCmdDrawPrimitives*>(pc);
            auto renderEncoder = context.FlushAndGetRenderEncoder();
            [renderEncoder
                drawPrimitives: cmd->primitiveType
                vertexStart:    cmd->vertexStart
                vertexCount:    cmd->vertexCount
                instanceCount:  cmd->instanceCount
                baseInstance:   cmd->baseInstance
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDrawIndexedPatches:
        {
            auto cmd = reinterpret_cast<const MTCmdDrawIndexedPatches*>(pc);
            auto renderEncoder = context.FlushAndGetRenderEncoder();
            [renderEncoder
                drawIndexedPatches:             cmd->controlPointCount
                patchStart:                     cmd->patchStart
                patchCount:                     cmd->patchCount
                patchIndexBuffer:               nil
                patchIndexBufferOffset:         0
                controlPointIndexBuffer:        cmd->controlPointIndexBuffer
                controlPointIndexBufferOffset:  cmd->controlPointIndexBufferOffset
                instanceCount:                  cmd->instanceCount
                baseInstance:                   cmd->baseInstance
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDrawIndexedPrimitives:
        {
            auto cmd = reinterpret_cast<const MTCmdDrawIndexedPrimitives*>(pc);
            auto renderEncoder = context.FlushAndGetRenderEncoder();
            [renderEncoder
                drawIndexedPrimitives:  cmd->primitiveType
                indexCount:             cmd->indexCount
                indexType:              cmd->indexType
                indexBuffer:            cmd->indexBuffer
                indexBufferOffset:      cmd->indexBufferOffset
                instanceCount:          cmd->instanceCount
                baseVertex:             cmd->baseVertex
                baseInstance:           cmd->baseInstance
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDispatchThreads:
        {
            auto cmd = reinterpret_cast<const MTCmdDispatchThreads*>(pc);
            auto computeEncoder = context.FlushAndGetComputeEncoder();
            [computeEncoder
                dispatchThreads:        cmd->threads
                threadsPerThreadgroup:  cmd->threadsPerThreadgroup
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDispatchThreadgroups:
        {
            auto cmd = reinterpret_cast<const MTCmdDispatchThreadgroups*>(pc);
            auto computeEncoder = context.FlushAndGetComputeEncoder();
            [computeEncoder
                dispatchThreadgroups:   cmd->threadgroups
                threadsPerThreadgroup:  cmd->threadsPerThreadgroup
            ];
            return sizeof(*cmd);
        }
        case MTOpcodeDispatchThreadgroupsIndirect:
        {
            auto cmd = reinterpret_cast<const MTCmdDispatchThreadgroupsIndirect*>(pc);
            auto computeEncoder = context.FlushAndGetComputeEncoder();
            [computeEncoder
                dispatchThreadgroupsWithIndirectBuffer: cmd->indirectBuffer
                indirectBufferOffset:                   cmd->indirectBufferOffset
                threadsPerThreadgroup:                  cmd->threadsPerThreadgroup
            ];
            return sizeof(*cmd);
        }
        case MTOpcodePushDebugGroup:
        {
            auto cmd = reinterpret_cast<const MTCmdPushDebugGroup*>(pc);
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
            auto cmd = reinterpret_cast<const MTCmdPresentDrawables*>(pc);
            auto views = reinterpret_cast<MTKView* const*>(cmd + 1);
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
    /* Initialize program counter to execute virtual Metal commands */
    for (const auto& chunk : virtualCmdBuffer)
    {
        auto pc     = chunk.data;
        auto pcEnd  = chunk.data + chunk.size;

        while (pc < pcEnd)
        {
            /* Read opcode */
            const MTOpcode opcode = *reinterpret_cast<const MTOpcode*>(pc);
            pc += sizeof(MTOpcode);

            /* Execute command and increment program counter */
            pc += ExecuteMTCommand(opcode, pc, context);
        }
    }
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
