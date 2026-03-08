/*
 * D3D9CommandExecutor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9CommandExecutor.h"
#include "D3D9Command.h"

#include "../Texture/D3D9Texture.h"
#include "../Texture/D3D9EmulatedSampler.h"
#include "../Texture/D3D9RenderTarget.h"

#include "../Buffer/D3D9VertexBuffer.h"
#include "../Buffer/D3D9IndexBuffer.h"

#include "../RenderState/D3D9PipelineState.h"
#include "../RenderState/D3D9ResourceHeap.h"
#include "../RenderState/D3D9RenderPass.h"
#include "../RenderState/D3D9QueryHeap.h"
#include "../RenderState/D3D9StateManager.h"

#include "../../CheckedCast.h"

#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static std::size_t ExecuteD3D9Command(const D3D9Opcode opcode, const void* pc, IDirect3DDevice9* device, D3D9StateManager* stateMngr)
{
    switch (opcode)
    {
        case D3D9OpcodeBeginScene:
        {
            device->BeginScene();
            return 0;
        }
        case D3D9OpcodeEndScene:
        {
            device->EndScene();
            return 0;
        }
        case D3D9OpcodeSetRenderTargets:
        {
            auto cmd = static_cast<const D3D9CmdSetRenderTargets*>(pc);
            auto* renderTargets = reinterpret_cast<IDirect3DSurface9* const *>(cmd + 1);
            for_range(i, cmd->count)
                device->SetRenderTarget(i, renderTargets[i]);
            return (sizeof(*cmd) + cmd->count * sizeof(IDirect3DSurface9*));
        }
        case D3D9OpcodeSetViewport:
        {
            auto cmd = static_cast<const D3D9CmdSetViewport*>(pc);
            device->SetViewport(&(cmd->viewport));
            return sizeof(*cmd);
        }
        case D3D9OpcodeSetScissorRect:
        {
            auto cmd = static_cast<const D3D9CmdSetScissorRect*>(pc);
            device->SetScissorRect(&(cmd->scissorRect));
            return sizeof(*cmd);
        }
        case D3D9OpcodeClear:
        {
            auto cmd = static_cast<const D3D9CmdClear*>(pc);
            device->Clear(0, nullptr, cmd->flags, cmd->color, cmd->z, cmd->stencil);
            return sizeof(*cmd);
        }
        case D3D9OpcodeSetIndices:
        {
            auto cmd = static_cast<const D3D9CmdSetIndices*>(pc);
            device->SetIndices(cmd->indexBuffer);
            return sizeof(*cmd);
        }
        case D3D9OpcodeSetStreamSource:
        {
            auto cmd = static_cast<const D3D9CmdSetStreamSource*>(pc);
            device->SetStreamSource(cmd->stream, cmd->vertexBuffer, cmd->offset, cmd->stride);
            return sizeof(*cmd);
        }
        case D3D9OpcodeBindProgrammablePSO:
        {
            auto cmd = static_cast<const D3D9CmdBindProgrammablePSO*>(pc);
            device->SetVertexDeclaration(cmd->vertexDeclaration);
            device->SetVertexShader(cmd->vertexShader);
            device->SetPixelShader(cmd->pixelShader);
            return sizeof(*cmd);
        }
        case D3D9OpcodeBindFixedFunctionPSO:
        {
            auto cmd = static_cast<const D3D9CmdBindFixedFunctionPSO*>(pc);
            device->SetVertexDeclaration(cmd->vertexDeclaration);
            device->SetVertexShader(nullptr);
            device->SetPixelShader(nullptr);
            return sizeof(*cmd);
        }
        case D3D9OpcodeSetRenderStates:
        {
            //TODO: this needs to use a state manager to avoid unnecessary state changes
            auto cmd = static_cast<const D3D9CmdSetRenderStates*>(pc);
            auto* renderStates = reinterpret_cast<const D3D9CmdSetRenderStates::RenderState*>(cmd + 1);
            for_range(i, cmd->numRenderStates)
                stateMngr->SetRenderState(renderStates[i].type, renderStates[i].value);
            return sizeof(*cmd) + sizeof(D3D9CmdSetRenderStates::RenderState) * cmd->numRenderStates;
        }
        case D3D9OpcodeBufferWrite:
        {
            auto cmd = static_cast<const D3D9CmdBufferWrite*>(pc);
            cmd->dstBuffer->Write(cmd->dstOffset, cmd + 1, cmd->dataSize);
            return sizeof(*cmd) + cmd->dataSize;
        }

        //TODO...

        case D3D9OpcodeDraw:
        {
            auto cmd = static_cast<const D3D9CmdDraw*>(pc);
            device->DrawPrimitive(cmd->primitiveType, cmd->startVertex, cmd->primitiveCount);
            return sizeof(*cmd);
        }
        case D3D9OpcodeDrawIndexed:
        {
            auto cmd = static_cast<const D3D9CmdDrawIndexed*>(pc);
            device->DrawIndexedPrimitive(cmd->primitiveType, cmd->baseVertexIndex, cmd->minVertexIndex, cmd->numVertices, cmd->startIndex, cmd->primitiveCount);
            return sizeof(*cmd);
        }
        default:
            return 0;
    }
}

void ExecuteD3D9VirtualCommandBuffer(const D3D9VirtualCommandBuffer& virtualCmdBuffer, D3D9StateManager* stateMngr)
{
    virtualCmdBuffer.Run(ExecuteD3D9Command, stateMngr->GetDevice(), stateMngr);
}


} // /namespace LLGL



// ================================================================================
