/*
 * GLCommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMMAND_BUFFER_H
#define LLGL_GL_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>


namespace LLGL
{


struct GLRenderState;
struct GLClearValue;

class GLCommandBuffer : public CommandBuffer
{

    public:

        // Returns true if this is an immediate command buffer, otherwise it is a deferred command buffer.
        virtual bool IsImmediateCmdBuffer() const = 0;

    protected:

        // Configures the attributes of 'renderState' for the type of index buffers.
        void SetIndexFormat(GLRenderState& renderState, bool indexType16Bits, std::uint64_t offset);

};


} // /namespace LLGL


#endif



// ================================================================================
