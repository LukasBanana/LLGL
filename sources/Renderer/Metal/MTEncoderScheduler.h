/*
 * MTEncoderScheduler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_ENCODER_SCHEDULER_H
#define LLGL_MT_ENCODER_SCHEDULER_H


#import <Metal/Metal.h>

#include "../StaticLimits.h"
#include <cstdint>


namespace LLGL
{


struct Viewport;
struct Scissor;
class MTResourceHeap;
class MTGraphicsPipeline;
class MTComputePipeline;

class MTEncoderScheduler
{

    public:

        // Resets the encoder scheduler with the new command buffer.
        void Reset(id<MTLCommandBuffer> cmdBuffer);

        // Ends the currently bound command encoder.
        void Flush();

        // Binds the respective command encoder with the specified descriptor.
        id<MTLRenderCommandEncoder> BindRenderEncoder(MTLRenderPassDescriptor* renderPassDesc, bool primaryRenderPass = false);
        id<MTLComputeCommandEncoder> BindComputeEncoder();
        id<MTLBlitCommandEncoder> BindBlitEncoder();

        // Interrupts the render command encoder (if active).
        void PauseRenderEncoder();
        void ResumeRenderEncoder();
    
        // Retunrs a copy of the current render pass descriptor or null if there is none.
        MTLRenderPassDescriptor* CopyRenderPassDesc();

    public:

        // Converts, binds, and stores the respective state in the internal render encoder state.
        void SetViewports(const Viewport* viewports, NSUInteger viewportCount);
        void SetScissorRects(const Scissor* scissors, NSUInteger scissorCount);
        void SetVertexBuffer(id<MTLBuffer> buffer, NSUInteger offset);
        void SetVertexBuffers(const id<MTLBuffer>* buffers, const NSUInteger* offsets, NSUInteger bufferCount);
        void SetGraphicsPipeline(MTGraphicsPipeline* graphicsPipeline);
        void SetGraphicsResourceHeap(MTResourceHeap* resourceHeap);

    public:
    
        // Returns the current render command encoder and flushes the queued render pass.
        id<MTLRenderCommandEncoder> GetRenderEncoderAndFlushRenderPass();

        // Returns the current render command encoder.
        inline id<MTLRenderCommandEncoder> GetRenderEncoder() const
        {
            return renderEncoder_;
        }

        // Returns the current compute command encoder.
        inline id<MTLComputeCommandEncoder> GetComputeEncoder() const
        {
            return computeEncoder_;
        }

        // Returns the current blit command encoder.
        inline id<MTLBlitCommandEncoder> GetBlitEncoder() const
        {
            return blitEncoder_;
        }
    
    private:
    
        void SubmitRenderEncoderState();
        void ResetRenderEncoderState();

    private:

        static const NSUInteger g_maxNumVertexBuffers = 32;

        struct MTRenderEncoderState
        {
            MTLViewport         viewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]      = {};
            NSUInteger          viewportCount                                       = 0;
            MTLScissorRect      scissorRects[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]   = {};
            NSUInteger          scissorRectCount                                    = 0;
            id<MTLBuffer>       vertexBuffers[g_maxNumVertexBuffers];
            NSUInteger          vertexBufferOffsets[g_maxNumVertexBuffers];
            NSRange             vertexBufferRange                                   = { 0, 0 };
            MTGraphicsPipeline* graphicsPipeline                                    = nullptr;
            MTResourceHeap*     resourceHeap                                        = nullptr;
        };

    private:
    
        id<MTLCommandBuffer>            cmdBuffer_              = nil;
    
        id<MTLRenderCommandEncoder>     renderEncoder_          = nil;
        id<MTLComputeCommandEncoder>    computeEncoder_         = nil;
        id<MTLBlitCommandEncoder>       blitEncoder_            = nil;
    
        MTLRenderPassDescriptor*        renderPassDesc_         = nullptr;
        MTRenderEncoderState            renderEncoderState_;

        bool                            isRenderEncoderPaused_  = false;

        union
        {
            std::uint8_t bits;
            struct
            {
                std::uint8_t viewports          : 1;
                std::uint8_t scissors           : 1;
                std::uint8_t vertexBuffers      : 1;
                std::uint8_t graphicsPipeline   : 1;
                std::uint8_t resourceHeap       : 1;
            };
        }
        dirtyBits_;

};


} // /namespace LLGL


#endif



// ================================================================================
