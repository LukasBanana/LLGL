/*
 * MTDirectCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_DIRECT_COMMAND_BUFFER_H
#define LLGL_MT_DIRECT_COMMAND_BUFFER_H


#include "MTCommandBuffer.h"
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


class MTDirectCommandBuffer final : public MTCommandBuffer
{

    public:

        #include "MTCommandBuffer.inl"

    public:

        MTDirectCommandBuffer(id<MTLDevice> device, MTCommandQueue& cmdQueue, const CommandBufferDescriptor& desc);
        ~MTDirectCommandBuffer();

    public:

        // Returns false.
        bool IsMultiSubmitCmdBuffer() const override;

        // Returns the native MTLCommandBuffer object.
        inline id<MTLCommandBuffer> GetNative() const
        {
            return cmdBuffer_;
        }

        // Returns true if this is an immediate command buffer.
        inline bool IsImmediateCmdBuffer() const
        {
            return ((GetFlags() & CommandBufferFlags::ImmediateSubmit) != 0);
        }

    private:

        void QueueDrawable(id<MTLDrawable> drawable);
        void PresentDrawables();

        // Returns the texture of the current drawable from the active framebuffer.
        id<MTLTexture> GetCurrentDrawableTexture() const;

        void FillBufferByte1(MTBuffer& bufferMT, const NSRange& range, std::uint8_t value);
        void FillBufferByte4(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Emulated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);
        void FillBufferByte4Accelerated(MTBuffer& bufferMT, const NSRange& range, std::uint32_t value);

        id<MTLRenderCommandEncoder> DispatchTessellationAndGetRenderEncoder(NSUInteger numPatches, NSUInteger numInstances = 1);

        // Dispatches the specified amount of local threads in as large threadgroups as possible.
        void DispatchThreads1D(
            id<MTLComputeCommandEncoder>    computeEncoder,
            id<MTLComputePipelineState>     computePSO,
            NSUInteger                      numThreads
        );

    private:

        id<MTLCommandBuffer>            cmdBuffer_              = nil;
        dispatch_semaphore_t            cmdBufferSemaphore_     = nil;

        MTCommandQueue&                 cmdQueue_;
        MTCommandContext                context_;

        SmallVector<id<MTLDrawable>, 2> drawables_;

};


} // /namespace LLGL


#endif



// ================================================================================
