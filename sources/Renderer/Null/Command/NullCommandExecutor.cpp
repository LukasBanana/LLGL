/*
 * NullCommandExecutor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

#include "../../CheckedCast.h"


namespace LLGL
{


static std::size_t ExecuteNullCommand(const NullOpcode opcode, const void* pc)
{
    switch (opcode)
    {
        case NullOpcodeBufferWrite:
        {
            auto cmd = static_cast<const NullCmdBufferWrite*>(pc);
            cmd->buffer->Write(cmd->offset, cmd + 1, cmd->size);
            return (sizeof(*cmd) + cmd->size);
        }
        case NullOpcodeCopySubresource:
        {
            auto cmd = static_cast<const NullCmdCopySubresource*>(pc);
            auto* dst = cmd->dstResource;
            auto* src = cmd->srcResource;
            if (dst->GetResourceType() == ResourceType::Buffer)
            {
                auto* dstBuffer = LLGL_CAST(NullBuffer*, dst);
                if (src->GetResourceType() == ResourceType::Buffer)
                {
                    auto* srcBuffer = LLGL_CAST(const NullBuffer*, src);
                    dstBuffer->CopyFromBuffer(cmd->dstX, *srcBuffer, cmd->srcX, cmd->width);
                }
                else if (src->GetResourceType() == ResourceType::Texture)
                {
                    //TODO
                }
            }
            else if (dst->GetResourceType() == ResourceType::Texture)
            {
                //TODO
            }
            return sizeof(*cmd);
        }
        case NullOpcodeGenerateMips:
        {
            auto cmd = static_cast<const NullCmdGenerateMips*>(pc);
            const TextureSubresource subresource{ cmd->baseArrayLayer, cmd->numArrayLayers, cmd->baseMipLevel, cmd->numMipLevels };
            cmd->texture->GenerateMips(&subresource);
            return sizeof(*cmd);
        }
        //TODO...
        case NullOpcodeDraw:
        {
            auto cmd = static_cast<const NullCmdDraw*>(pc);
            //TODO
            return (sizeof(*cmd) + cmd->numVertexBuffers * sizeof(const NullBuffer*));
        }
        case NullOpcodeDrawIndexed:
        {
            auto cmd = static_cast<const NullCmdDrawIndexed*>(pc);
            //TODO
            return (sizeof(*cmd) + cmd->numVertexBuffers * sizeof(const NullBuffer*));
        }
        case NullOpcodePushDebugGroup:
        {
            auto cmd = static_cast<const NullCmdPushDebugGroup*>(pc);
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
    virtualCmdBuffer.Run(ExecuteNullCommand);
}


} // /namespace LLGL



// ================================================================================
