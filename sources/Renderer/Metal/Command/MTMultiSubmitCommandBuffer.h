/*
 * MTMultiSubmitCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_MULTI_SUBMIT_COMMAND_BUFFER_H
#define LLGL_MT_MULTI_SUBMIT_COMMAND_BUFFER_H


#include "MTCommandBuffer.h"
#include "MTCommandOpcode.h"
#include "../../VirtualCommandBuffer.h"


namespace LLGL
{


using MTVirtualCommandBuffer = VirtualCommandBuffer<MTOpcode>;

class MTMultiSubmitCommandBuffer final : public MTCommandBuffer
{

    public:

        #include "MTCommandBuffer.inl"

    public:

        MTMultiSubmitCommandBuffer(id<MTLDevice> device, const CommandBufferDescriptor& desc);
        ~MTMultiSubmitCommandBuffer();

    public:

        // Returns true.
        bool IsMultiSubmitCmdBuffer() const override;

        // Returns the internal virtual command buffer.
        inline const MTVirtualCommandBuffer& GetVirtualCommandBuffer() const
        {
            return buffer_;
        }

    private:

        void BindRenderEncoderForTessellation(NSUInteger numPatches, NSUInteger numInstances = 1);
        void BindRenderEncoder();
        void BindComputeEncoder();
        void BindBlitEncoder();

        void QueueDrawable(MTKView* view);
        void PresentDrawables();

        void DispatchThreads1D(id<MTLComputePipelineState> computePSO, NSUInteger numThreads);

        void GenerateMipmapsForTexture(id<MTLTexture> texture);

        void SetNativeVertexBuffers(NSUInteger count, const id<MTLBuffer>* buffers, const NSUInteger* offsets);

        void FlushContext();

        void ReleaseIntermediateResources();

        // Allocates only an opcode for empty commands.
        void AllocOpcode(const MTOpcode opcode);

        // Allocates a new command and stores the specified opcode.
        template <typename TCommand>
        TCommand* AllocCommand(const MTOpcode opcode, std::size_t payloadSize = 0);

    private:

        MTVirtualCommandBuffer          buffer_;
        MTOpcode                        lastOpcode_             = MTOpcodeNop;

        MTEncoderState                  encoderState_           = MTEncoderState::None;
        SmallVector<MTKView*, 2>        views_;
        SmallVector<id<MTLTexture>, 2>  intermediateTextures_;

        bool                            isInsideRenderPass_     = false;

};


} // /namespace LLGL


#endif



// ================================================================================
