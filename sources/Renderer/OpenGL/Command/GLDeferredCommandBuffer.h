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

#ifdef LLGL_ENABLE_JIT_COMPILER
#   include "../../../JIT/JITProgram.h"
#endif


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
#ifdef LLGL_GL_ENABLE_OPENGL2X
class GL2XSampler;
#endif

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

        #ifdef LLGL_ENABLE_JIT_COMPILER

        // Returns the just-in-time compiled command buffer that can be executed natively, or null if not available.
        inline const std::unique_ptr<JITProgram>& GetExecutable() const
        {
            return executable_;
        }

        // Returns the maximum number of viewports that are set in this command buffer.
        inline std::uint32_t GetMaxNumViewports() const
        {
            return maxNumViewports_;
        }

        // Returns the maximum number of scissors that are set in this command buffer.
        inline std::uint32_t GetMaxNumScissors() const
        {
            return maxNumScissors_;
        }

        #endif // /LLGL_ENABLE_JIT_COMPILER

    private:

        void BindBufferBase(const GLBufferTarget bufferTarget, const GLBuffer& bufferGL, std::uint32_t slot);
        void BindBuffersBase(const GLBufferTarget bufferTarget, std::uint32_t first, std::uint32_t count, const Buffer *const *const buffers);
        void BindTexture(GLTexture& textureGL, std::uint32_t slot);
        void BindImageTexture(const GLTexture& textureGL, std::uint32_t slot);
        void BindSampler(const GLSampler& samplerGL, std::uint32_t slot);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        void BindGL2XSampler(const GL2XSampler& samplerGL2X, std::uint32_t slot);
        #endif

        void FlushMemoryBarriers();

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const GLOpcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const GLOpcode opcode, std::size_t payloadSize = 0);

    private:

        long                        flags_                  = 0;
        GLVirtualCommandBuffer      buffer_;
        GLRenderTarget*             renderTargetToResolve_  = nullptr;

        #ifdef LLGL_ENABLE_JIT_COMPILER
        std::unique_ptr<JITProgram> executable_;
        std::uint32_t               maxNumViewports_        = 0;
        std::uint32_t               maxNumScissors_         = 0;
        #endif // /LLGL_ENABLE_JIT_COMPILER

};


} // /namespace LLGL


#endif



// ================================================================================
