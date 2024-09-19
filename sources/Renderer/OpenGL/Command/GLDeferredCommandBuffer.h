/*
 * GLDeferredCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_DEFERRED_COMMAND_BUFFER_H
#define LLGL_GL_DEFERRED_COMMAND_BUFFER_H


#include "GLCommandBuffer.h"
#include "GLCommandOpcode.h"
#include "../../VirtualCommandBuffer.h"
#include <memory>
#include <vector>


namespace LLGL
{


class GLBuffer;
class GLBufferArray;
class GLTexture;
class GLSampler;
class GLRenderTarget;
class GLSwapChain;
class GLStateManager;
class GLRenderPass;
class GLShaderPipeline;
class GLEmulatedSampler;

using GLVirtualCommandBuffer = VirtualCommandBuffer<GLOpcode>;

class GLDeferredCommandBuffer final : public GLCommandBuffer
{

    public:

        #include "GLCommandBuffer.inl"

    public:

        GLDeferredCommandBuffer(long flags, std::size_t initialBufferSize = 1024);

    public:

        // Returns false.
        bool IsImmediateCmdBuffer() const override;

        // Returns true if this is a primary command buffer.
        bool IsPrimary() const;

        // Returns the internal command buffer as raw byte buffer.
        inline const GLVirtualCommandBuffer& GetVirtualCommandBuffer() const
        {
            return buffer_;
        }

        // Returns the flags this command buffer was created with (see CommandBufferDescriptor::flags).
        inline long GetFlags() const
        {
            return flags_;
        }

    private:

        void BindBufferBase(const GLBufferTarget bufferTarget, const GLBuffer& bufferGL, std::uint32_t slot);
        void BindBuffersBase(const GLBufferTarget bufferTarget, std::uint32_t first, std::uint32_t count, const Buffer *const *const buffers);
        void BindTexture(GLTexture& textureGL, std::uint32_t slot);
        void BindImageTexture(const GLTexture& textureGL, std::uint32_t slot);
        void BindSampler(const GLSampler& samplerGL, std::uint32_t slot);
        void BindEmulatedSampler(const GLEmulatedSampler& emulatedSamplerGL, std::uint32_t slot);

        void FlushMemoryBarriers();

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const GLOpcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const GLOpcode opcode, std::size_t payloadSize = 0);

    private:

        long                    flags_                  = 0;
        GLVirtualCommandBuffer  buffer_;
        GLRenderTarget*         renderTargetToResolve_  = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
