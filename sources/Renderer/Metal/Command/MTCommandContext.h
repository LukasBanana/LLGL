/*
 * MTCommandContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_CONTEXT_H
#define LLGL_MT_COMMAND_CONTEXT_H


#import <Metal/Metal.h>

#include "../RenderState/MTDescriptorCache.h"
#include "../RenderState/MTConstantsCache.h"
#include <LLGL/Constants.h>
#include <LLGL/CommandBufferFlags.h>
#include <cstdint>


namespace LLGL
{


struct Viewport;
struct Scissor;
class Resource;
class MTResourceHeap;
class MTGraphicsPSO;
class MTComputePSO;

struct MTInternalBindingTable
{
    NSUInteger tessFactorBufferSlot = 30;
};

// Metal commadn context: Manages the scheduling between render and compute command encoders.
class MTCommandContext
{

    public:

        // Resets all internal states.
        void Reset();

        // Resets the encoder scheduler with the new command buffer.
        void Reset(id<MTLCommandBuffer> cmdBuffer);

        // Ends the currently bound command encoder.
        void Flush();

        // Binds the respective command encoder with the specified descriptor.
        id<MTLRenderCommandEncoder> BindRenderEncoder(MTLRenderPassDescriptor* renderPassDesc, bool isPrimaryRenderPass = false);
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
        void SetGraphicsResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet);
        void SetBlendColor(const float blendColor[4]);
        void SetStencilRef(std::uint32_t ref, const StencilFace face);

        // Converts, binds, and stores the respective state in the internal compute encoder state.
        void SetComputePSO(MTComputePSO* pipelineState);
        void SetComputeResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet);

        // Rebinds the currently bounds resource heap to the specified compute encoder (used for tessellation encoding).
        void RebindResourceHeap(id<MTLComputeCommandEncoder> computeEncoder);

    public:

        // Returns the native command buffer currently used by this context.
        inline id<MTLCommandBuffer> GetCommandBuffer() const
        {
            return cmdBuffer_;
        }

        // Returns the current render command encoder and flushes the queued render states and render pass.
        id<MTLRenderCommandEncoder> FlushAndGetRenderEncoder();

        // Returns the current compute command encoder and flushes the stored compute states.
        id<MTLComputeCommandEncoder> FlushAndGetComputeEncoder();

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

        // Sets the specified resource in the descriptor cache.
        inline void SetResource(std::uint32_t descriptor, Resource& resource)
        {
            descriptorCache_.SetResource(descriptor, resource);
        }

        // Sets the specified uniforms in the constants cache.
        inline void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize)
        {
            constantsCache_.SetUniforms(first, data, dataSize);
        }

    public:

        // Table of all internal binding slots.
        MTInternalBindingTable bindingTable;

    private:

        void SubmitRenderEncoderState();
        void ResetRenderEncoderState();

        void SubmitComputeEncoderState();
        void ResetComputeEncoderState();

    private:

        static constexpr NSUInteger maxNumVertexBuffers         = 32;
        static constexpr NSUInteger maxNumViewportsAndScissors  = LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS;

        enum DirtyBits
        {
            // Render encoder
            DirtyBit_Viewports              = (1 << 0),
            DirtyBit_Scissors               = (1 << 1),
            DirtyBit_VertexBuffers          = (1 << 2),
            DirtyBit_GraphicsPSO            = (1 << 3),
            DirtyBit_GraphicsResourceHeap   = (1 << 4),
            DirtyBit_BlendColor             = (1 << 5),
            DirtyBit_StencilRef             = (1 << 6),

            // Compute encoder
            DirtyBit_ComputePSO             = (1 << 0),
            DirtyBit_ComputeResourceHeap    = (1 << 1),
        };

        struct MTRenderEncoderState
        {
            MTLViewport     viewports[maxNumViewportsAndScissors]       = {};
            NSUInteger      viewportCount                               = 0;
            MTLScissorRect  scissorRects[maxNumViewportsAndScissors]    = {};
            NSUInteger      scissorRectCount                            = 0;
            id<MTLBuffer>   vertexBuffers[maxNumVertexBuffers]          = {};
            NSUInteger      vertexBufferOffsets[maxNumVertexBuffers]    = {};
            NSRange         vertexBufferRange                           = { 0, 0 };

            MTGraphicsPSO*  graphicsPSO                                 = nullptr;
            MTResourceHeap* graphicsResourceHeap                        = nullptr;
            std::uint32_t   graphicsResourceSet                         = 0;

            float           blendColor[4]                               = { 0.0f, 0.0f, 0.0f, 0.0f };
            bool            blendColorDynamic                           = false;

            std::uint32_t   stencilFrontRef                             = 0;
            std::uint32_t   stencilBackRef                              = 0;
            bool            stencilRefDynamic                           = false;
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
        MTDescriptorCache               descriptorCache_;
        MTConstantsCache                constantsCache_;

        std::uint8_t                    renderDirtyBits_        = 0;
        std::uint8_t                    computeDirtyBits_       = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
