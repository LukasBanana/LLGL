/*
 * D3D9CommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_COMMAND_BUFFER_H
#define LLGL_D3D9_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/Container/SmallVector.h>
#include "D3D9CommandOpcode.h"
#include "D3D9Command.h"
#include "../Direct3D9.h"


namespace LLGL
{


class D3D9VertexBuffer;
class D3D9IndexBuffer;
class D3D9StateManager;
class D3D9ConstantsCache;

class D3D9CommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D9CommandBuffer(D3D9StateManager* stateMngr, const CommandBufferDescriptor& desc);

    public:

        // Executes the internal virtual command buffer.
        void ExecuteVirtualCommands();

    public:

        const CommandBufferDescriptor desc;

    private:

        struct RenderState
        {
            D3DPRIMITIVETYPE    primitiveType       = D3DPT_POINTLIST;
            UINT                indexBufferOffset   = 0;
        };

    private:

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const D3D9Opcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const D3D9Opcode opcode, std::size_t payloadSize = 0);

        void FlushConstantsCache();

        void AllocSetStreamSourceCommand(UINT stream, IDirect3DVertexBuffer9* vertexBuffer, UINT stride, UINT offset);

        void AllocDrawCommand(UINT startVertex, UINT numVertices);
        void AllocDrawIndexedCommand(INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex);

        D3D9CmdSetRenderStates::D3DRenderState* AllocSetRenderStatesCommand(UINT count);

        inline bool IsPrimary() const
        {
            return ((desc.flags & CommandBufferFlags::Secondary) == 0);
        }

        inline bool IsImmediateSubmit() const
        {
            return ((desc.flags & CommandBufferFlags::ImmediateSubmit) != 0);
        }

        inline const D3D9VirtualCommandBuffer* GetVcmdBuffer() const
        {
            return &(buffer_);
        }

    private:

        IDirect3DDevice9*           device_                 = nullptr;
        D3D9StateManager*           stateMngr_              = nullptr;
        D3D9VirtualCommandBuffer    buffer_;
        RenderState                 renderState_;
        D3D9ConstantsCache*         boundConstantsCache_    = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
