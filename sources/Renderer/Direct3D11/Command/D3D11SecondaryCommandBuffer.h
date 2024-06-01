/*
 * D3D11SecondaryCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SECONDARY_COMMAND_BUFFER_H
#define LLGL_D3D11_SECONDARY_COMMAND_BUFFER_H


#include "D3D11CommandBuffer.h"
#include "D3D11CommandOpcode.h"
#include "../../VirtualCommandBuffer.h"


namespace LLGL
{


using D3D11VirtualCommandBuffer = VirtualCommandBuffer<D3D11Opcode>;

class D3D11SecondaryCommandBuffer final : public D3D11CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        D3D11SecondaryCommandBuffer(const CommandBufferDescriptor& desc);

        // Returns the internal command buffer as raw byte buffer.
        inline const D3D11VirtualCommandBuffer& GetVirtualCommandBuffer() const
        {
            return buffer_;
        }

    private:

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const D3D11Opcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const D3D11Opcode opcode, std::size_t payloadSize = 0);

    private:

        D3D11VirtualCommandBuffer buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
