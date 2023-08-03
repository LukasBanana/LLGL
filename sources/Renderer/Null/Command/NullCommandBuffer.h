/*
 * NullCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_COMMAND_BUFFER_H
#define LLGL_NULL_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/Container/SmallVector.h>
#include "NullCommandOpcode.h"
#include "../../VirtualCommandBuffer.h"


namespace LLGL
{


class NullBuffer;

using NullVirtualCommandBuffer = VirtualCommandBuffer<NullOpcode>;

class NullCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        NullCommandBuffer(const CommandBufferDescriptor& desc);

    public:

        // Executes the internal virtual command buffer.
        void ExecuteVirtualCommands();

    public:

        const CommandBufferDescriptor desc;

    private:

        struct RenderState
        {
            SmallVector<Viewport>           viewports;
            SmallVector<Scissor>            scissors;
            SmallVector<const NullBuffer*>  vertexBuffers;
            const NullBuffer*               indexBuffer         = nullptr;
            Format                          indexBufferFormat   = Format::Undefined;
            std::uint64_t                   indexBufferOffset   = 0;
        };

    private:

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const NullOpcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const NullOpcode opcode, std::size_t payloadSize = 0);

        void AllocDrawCommand(const DrawIndirectArguments& args);
        void AllocDrawIndexedCommand(const DrawIndexedIndirectArguments& args);

    private:

        NullVirtualCommandBuffer    buffer_;
        RenderState                 renderState_;

};


} // /namespace LLGL


#endif



// ================================================================================
