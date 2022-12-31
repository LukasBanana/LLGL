/*
 * NullCommandExecutor.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullCommandExecutor.h"
#include "NullCommand.h"

#include "../Texture/NullTexture.h"
#include "../Texture/NullSampler.h"
#include "../Texture/NullRenderTarget.h"

#include "../Buffer/NullBuffer.h"

#include "../RenderState/NullPipelineState.h"
#include "../RenderState/NullResourceHeap.h"
#include "../RenderState/NullRenderPass.h"
#include "../RenderState/NullQueryHeap.h"


namespace LLGL
{


static std::size_t ExecuteNullCommand(const NullOpcode opcode, const void* pc)
{
    switch (opcode)
    {
        case NullOpcodeBufferWrite:
        {
            auto cmd = reinterpret_cast<const NullCmdBufferWrite*>(pc);
            cmd->buffer->Write(cmd->offset, cmd + 1, cmd->size);
            return (sizeof(*cmd) + cmd->size);
        }
        //TODO...
        case NullOpcodePushDebugGroup:
        {
            auto cmd = reinterpret_cast<const NullCmdPushDebugGroup*>(pc);
            //TODO
            return (sizeof(*cmd) + cmd->length + 1);
        }
        case NullOpcodePopDebugGroup:
        {
            //TODO
            return 0;
        }
        default:
            return 0;
    }
}

void ExecuteNullVirtualCommandBuffer(const NullVirtualCommandBuffer& virtualCmdBuffer)
{
    /* Initialize program counter to execute virtual GL commands */
    for (const auto& chunk : virtualCmdBuffer)
    {
        auto pc     = chunk.data;
        auto pcEnd  = chunk.data + chunk.size;

        while (pc < pcEnd)
        {
            /* Read opcode */
            const NullOpcode opcode = *reinterpret_cast<const NullOpcode*>(pc);
            pc += sizeof(NullOpcode);

            /* Execute command and increment program counter */
            pc += ExecuteNullCommand(opcode, pc);
        }
    }
}


} // /namespace LLGL



// ================================================================================
