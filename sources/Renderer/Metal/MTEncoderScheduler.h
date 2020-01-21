/*
 * MTEncoderScheduler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_ENCODER_SCHEDULER_H
#define LLGL_MT_ENCODER_SCHEDULER_H


#import <Metal/Metal.h>

#include <LLGL/StaticLimits.h>
#include <LLGL/CommandBufferFlags.h>
#include <cstdint>


namespace LLGL
{


struct Viewport;
struct Scissor;
class MTResourceHeap;
class MTGraphicsPSO;
class MTComputePSO;

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
        void SetGraphicsPSO(MTGraphicsPSO* pipelineState);
        void SetGraphicsResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t firstSet);
        void SetBlendColor(const float* blendColor);
        void SetStencilRef(std::uint32_t ref, const StencilFace face);

        // Converts, binds, and stores the respective state in the internal compute encoder state.
        void SetComputePSO(MTComputePSO* pipelineState);
        void SetComputeResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t firstSet);

        // Rebinds the currently bounds resource heap to the specified compute encoder (used for tessellation encoding).
        void RebindResourceHeap(id<MTLComputeCommandEncoder> computeEncoder);

    public:

        // Returns the current render command encoder and flushes the queued render states and render pass.
        id<MTLRenderCommandEncoder> GetRenderEncoderAndFlushState();

        // Returns the current compute command encoder and flushes the stored compute states.
        id<MTLComputeCommandEncoder> GetComputeEncoderAndFlushState();

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

        void SubmitComputeEncoderState();
        void ResetComputeEncoderState();

    private:

        static const NSUInteger g_maxNumVertexBuffers = 32;

        struct MTRenderEncoderState
        {
            MTLViewport     viewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]      = {};
            NSUInteger      viewportCount                                       = 0;
            MTLScissorRect  scissorRects[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS]   = {};
            NSUInteger      scissorRectCount                                    = 0;
            id<MTLBuffer>   vertexBuffers[g_maxNumVertexBuffers];
            NSUInteger      vertexBufferOffsets[g_maxNumVertexBuffers];
            NSRange         vertexBufferRange                                   = { 0, 0 };

            MTGraphicsPSO*  graphicsPSO                                         = nullptr;
            MTResourceHeap* graphicsResourceHeap                                = nullptr;
            std::uint32_t   graphicsResourceSet                                 = 0;

            float           blendColor[4]                                       = { 0.0f, 0.0f, 0.0f, 0.0f };
            bool            blendColorDynamic                                   = false;

            std::uint32_t   stencilFrontRef                                     = 0;
            std::uint32_t   stencilBackRef                                      = 0;
            bool            stencilRefDynamic                                   = false;
        };

        struct MTComputeEncoderState
        {
            MTComputePSO*   computePSO          = nullptr;
            MTResourceHeap* computeResourceHeap = nullptr;
            std::uint32_t   computeResourceSet  = 0;
        };

    private:

        id<MTLCommandBuffer>            cmdBuffer_              = nil;

        id<MTLRenderCommandEncoder>     renderEncoder_  	    = nil;
        id<MTLComputeCommandEncoder>    computeEncoder_         = nil;
        id<MTLBlitCommandEncoder>       blitEncoder_            = nil;

        MTLRenderPassDescriptor*        renderPassDesc_         = nullptr;
        MTRenderEncoderState            renderEncoderState_;
        MTComputeEncoderState           computeEncoderState_;

        bool                            isRenderEncoderPaused_  = false;

        union
        {
            std::uint8_t bits;
            struct
            {
                std::uint8_t viewports              : 1;
                std::uint8_t scissors               : 1;
                std::uint8_t vertexBuffers          : 1;
                std::uint8_t graphicsPSO            : 1;
                std::uint8_t graphicsResourceHeap   : 1;
                std::uint8_t blendColor             : 1;
                std::uint8_t stencilRef             : 1;
            };
        }
        renderDirtyBits_;

        union
        {
            std::uint8_t bits;
            struct
            {
                std::uint8_t computePSO             : 1;
                std::uint8_t computeResourceHeap    : 1;
            };
        }
        computeDirtyBits_;

};


} // /namespace LLGL


#endif



// ================================================================================
