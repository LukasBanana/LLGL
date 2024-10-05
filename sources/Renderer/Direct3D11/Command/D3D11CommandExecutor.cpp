/*
 * D3D11CommandExecutor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11CommandExecutor.h"
#include "D3D11Command.h"
#include "D3D11CommandOpcode.h"
#include "D3D11CommandContext.h"
#include "D3D11SecondaryCommandBuffer.h"

#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"

#include <algorithm>
#include <string.h>


namespace LLGL
{


static std::size_t ExecuteD3D11Command(const D3D11Opcode opcode, const void* pc, D3D11CommandContext& context)
{
    switch (opcode)
    {
        case D3D11OpcodeSetVertexBuffer:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetVertexBuffer*>(pc);
            context.SetVertexBuffer(*(cmd->buffer));
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetVertexBufferArray:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetVertexBufferArray*>(pc);
            context.SetVertexBufferArray(*(cmd->bufferArray));
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetIndexBuffer:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetIndexBuffer*>(pc);
            context.SetIndexBuffer(*(cmd->buffer), cmd->format, cmd->offset);
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetPipelineState:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetPipelineState*>(pc);
            context.SetPipelineState(cmd->pipelineState);
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetResourceHeap:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetResourceHeap*>(pc);
            context.SetResourceHeap(*(cmd->resourceHeap), cmd->descriptorSet);
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetResource:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetResource*>(pc);
            context.SetResource(cmd->descriptor, *(cmd->resource));
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetBlendFactor:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetBlendFactor*>(pc);
            context.GetStateManager().SetBlendFactor(cmd->color);
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetStencilRef:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetStencilRef*>(pc);
            context.GetStateManager().SetStencilRef(cmd->stencilRef);
            return sizeof(*cmd);
        }
        case D3D11OpcodeSetUniforms:
        {
            auto cmd = reinterpret_cast<const D3D11CmdSetUniforms*>(pc);
            context.SetUniforms(cmd->first, cmd + 1, cmd->dataSize);
            return (sizeof(*cmd) + cmd->dataSize);
        }
        case D3D11OpcodeDraw:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDraw*>(pc);
            context.Draw(cmd->vertexCount, cmd->startVertexLocation);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawIndexed:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawIndexed*>(pc);
            context.DrawIndexed(cmd->indexCount, cmd->startIndexLocation, cmd->baseVertexLocation);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawInstanced:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawInstanced*>(pc);
            context.DrawInstanced(cmd->vertexCountPerInstance, cmd->instanceCount, cmd->startVertexLocation, cmd->startInstanceLocation);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawIndexedInstanced:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawIndexedInstanced*>(pc);
            context.DrawIndexedInstanced(cmd->indexCountPerInstance, cmd->instanceCount, cmd->startIndexLocation, cmd->baseVertexLocation, cmd->startInstanceLocation);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawInstancedIndirect:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawInstancedIndirect*>(pc);
            context.DrawInstancedIndirect(cmd->bufferForArgs, cmd->alignedByteOffsetForArgs);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawInstancedIndirectN:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawInstancedIndirect*>(pc);
            context.DrawInstancedIndirectN(cmd->bufferForArgs, cmd->alignedByteOffsetForArgs, cmd->numCommands, cmd->stride);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawIndexedInstancedIndirect:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawInstancedIndirect*>(pc);
            context.DrawIndexedInstancedIndirect(cmd->bufferForArgs, cmd->alignedByteOffsetForArgs);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawIndexedInstancedIndirectN:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDrawInstancedIndirect*>(pc);
            context.DrawIndexedInstancedIndirectN(cmd->bufferForArgs, cmd->alignedByteOffsetForArgs, cmd->numCommands, cmd->stride);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDrawAuto:
        {
            context.DrawAuto();
            return 0;
        }
        case D3D11OpcodeDispatch:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDispatch*>(pc);
            context.Dispatch(cmd->threadGroupCountX, cmd->threadGroupCountY, cmd->threadGroupCountZ);
            return sizeof(*cmd);
        }
        case D3D11OpcodeDispatchIndirect:
        {
            auto cmd = reinterpret_cast<const D3D11CmdDispatchIndirect*>(pc);
            context.DispatchIndirect(cmd->bufferForArgs, cmd->alignedByteOffsetForArgs);
            return sizeof(*cmd);
        }
        default:
            return 0;
    }
}

static void ExecuteD3D11CommandsEmulated(const D3D11VirtualCommandBuffer& virtualCmdBuffer, D3D11CommandContext& context)
{
    virtualCmdBuffer.Run(ExecuteD3D11Command, context);
}

void ExecuteD3D11SecondaryCommandBuffer(const D3D11SecondaryCommandBuffer& cmdBuffer, D3D11CommandContext& context)
{
    /* Emulate execution of GL commands */
    ExecuteD3D11CommandsEmulated(cmdBuffer.GetVirtualCommandBuffer(), context);
}

void ExecuteD3D11CommandBuffer(const D3D11CommandBuffer& cmdBuffer, D3D11CommandContext& context)
{
    /* Is this a secondary command buffer? */
    if (cmdBuffer.IsSecondaryCmdBuffer())
    {
        /* Execute secondary command buffer */
        auto& secondaryCmdBufferD3D = LLGL_CAST(const D3D11SecondaryCommandBuffer&, cmdBuffer);
        ExecuteD3D11SecondaryCommandBuffer(secondaryCmdBufferD3D, context);
    }
}


} // /namespace LLGL



// ================================================================================
