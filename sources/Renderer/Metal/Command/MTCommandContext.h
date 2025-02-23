/*
 * MTCommandContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_CONTEXT_H
#define LLGL_MT_COMMAND_CONTEXT_H


#import <MetalKit/MetalKit.h>

#include "../Buffer/MTIntermediateBuffer.h"
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
class RenderTarget;
class MTResourceHeap;
class MTGraphicsPSO;
class MTComputePSO;
class MTPipelineState;
class MTSwapChain;
class MTRenderPass;

struct MTInternalBindingTable
{
    NSUInteger tessFactorBufferSlot = 30;
};

// Active command encoder state enumeration.
enum class MTEncoderState
{
    None,
    Render,
    Compute,
    Blit,
};

// Metal command context: Manages the scheduling between render and compute command encoders.
class MTCommandContext
{

    public:

        // Initializes internal buffers with the Metal device.
        MTCommandContext(id<MTLDevice> device);

        // Resets all internal states.
        void Reset();

        // Resets the encoder scheduler with the new command buffer.
        void Reset(id<MTLCommandBuffer> cmdBuffer);

        // Ends the currently bound command encoder.
        void Flush();

        void BeginRenderPass(
            RenderTarget*       renderTarget,
            const MTRenderPass* renderPassMT,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );
        void UpdateRenderPass(MTLRenderPassDescriptor* renderPassDesc);
        void EndRenderPass();

        // Binds the respective command encoder with the specified descriptor.
        id<MTLRenderCommandEncoder> BindRenderEncoder();
        id<MTLComputeCommandEncoder> BindComputeEncoder();
        id<MTLBlitCommandEncoder> BindBlitEncoder();

        // Returns the current render command encoder and flushes the queued render states and render pass.
        id<MTLRenderCommandEncoder> FlushAndGetRenderEncoder();
        id<MTLComputeCommandEncoder> FlushAndGetComputeEncoder();

        // Returns a copy of the current render pass descriptor or null if there is none.
        MTLRenderPassDescriptor* CopyRenderPassDesc();

        // Returns a retained pointer to the current render pass descriptor or null if the context is outside a render pass.
        MTLRenderPassDescriptor* RetainRenderPassDescOrNull();

        // Dispatches the specified amount of local threads in as large threadgroups as possible.
        void DispatchThreads1D(
            id<MTLComputeCommandEncoder>    computeEncoder,
            id<MTLComputePipelineState>     computePSO,
            NSUInteger                      numThreads
        );

        // Dispatches the current tessellation compute shader and returns the respective render encoder.
        id<MTLRenderCommandEncoder> DispatchTessellationAndGetRenderEncoder(NSUInteger numPatches, NSUInteger numInstances = 1);

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
        void SetVisibilityBuffer(id<MTLBuffer> buffer, MTLVisibilityResultMode mode, NSUInteger offset);

        // Converts, binds, and stores the respective state in the internal compute encoder state.
        void SetComputePSO(MTComputePSO* pipelineState);
        void SetComputeResourceHeap(MTResourceHeap* resourceHeap, std::uint32_t descriptorSet);

        // Rebinds the currently bounds resource heap to the specified compute encoder (used for tessellation encoding).
        void RebindResourceHeap(id<MTLComputeCommandEncoder> computeEncoder);

        // State cache.
        void SetIndexStream(id<MTLBuffer> indexBuffer, NSUInteger offset, bool indexType16Bits);

        // Grows the internal tessellation-factor buffer to fit the specified number of patches and instances, then returns the native Metal buffer.
        id<MTLBuffer> GetTessFactorBufferAndGrow(NSUInteger numPatchesAndInstances);

        // Returns the Metal view of the current drawable from the active framebuffer.
        MTKView* GetCurrentDrawableView() const;

    public:

        // Returns the native command buffer currently used by this context.
        inline id<MTLCommandBuffer> GetCommandBuffer() const
        {
            return cmdBuffer_;
        }

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

        // Returns true if this command context is currently inside a render pass.
        inline bool IsInsideRenderPass() const
        {
            return contextState_.isInsideRenderPass;
        }

        // Returns the current native index type.
        inline MTLIndexType GetIndexType() const
        {
            return contextState_.indexType;
        }

        // Returns the current native index buffer.
        inline id<MTLBuffer> GetIndexBuffer() const
        {
            return contextState_.indexBuffer;
        }

        // Returns the byte offset for the specified index.
        inline NSUInteger GetIndexBufferOffset(NSUInteger firstIndex) const
        {
            return (contextState_.indexBufferOffset + contextState_.indexTypeSize * firstIndex);
        }

        inline MTLPrimitiveType GetPrimitiveType() const
        {
            return contextState_.primitiveType;
        }

        inline NSUInteger GetNumPatchControlPoints() const
        {
            return contextState_.numPatchControlPoints;
        }

        inline NSUInteger GetTessFactorSize() const
        {
            return contextState_.tessFactorSize;
        }

        inline const MTLSize& GetThreadsPerThreadgroup() const
        {
            return contextState_.threadsPerThreadgroup;
        }

        inline MTPipelineState* GetBoundPipelineState() const
        {
            return contextState_.boundPipelineState;
        }

    public:

        // Table of all internal binding slots.
        MTInternalBindingTable bindingTable;

    private:

        void BeginRenderPassWithDescriptor(MTLRenderPassDescriptor* renderPassDesc, MTSwapChain* swapChainMT);
        void BindRenderEncoderWithDescriptor(MTLRenderPassDescriptor* renderPassDesc);
        void PauseRenderEncoder();
        void ResumeRenderEncoder();

        void SubmitRenderEncoderState();
        void ResetRenderEncoderState();

        void SubmitComputeEncoderState();
        void ResetComputeEncoderState();

        void ResetContextState();

        NSUInteger GetMaxLocalThreads(id<MTLComputePipelineState> computePSO) const;

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
            DirtyBit_VisibilityResultMode   = (1 << 7),

            // Compute encoder
            DirtyBit_ComputePSO             = (1 << 0),
            DirtyBit_ComputeResourceHeap    = (1 << 1),
        };

        struct MTRenderEncoderState
        {
            MTLViewport             viewports[maxNumViewportsAndScissors]       = {};
            NSUInteger              viewportCount                               = 0;
            MTLScissorRect          scissorRects[maxNumViewportsAndScissors]    = {};
            NSUInteger              scissorRectCount                            = 0;
            bool                    isScissorTestEnabled                        = false;
            id<MTLBuffer>           vertexBuffers[maxNumVertexBuffers]          = {};
            NSUInteger              vertexBufferOffsets[maxNumVertexBuffers]    = {};
            NSRange                 vertexBufferRange                           = { 0, 0 };

            MTGraphicsPSO*          graphicsPSO                                 = nullptr;
            MTResourceHeap*         graphicsResourceHeap                        = nullptr;
            std::uint32_t           graphicsResourceSet                         = 0;

            float                   blendColor[4]                               = { 0.0f, 0.0f, 0.0f, 0.0f };
            bool                    blendColorDynamic                           = false;

            std::uint32_t           stencilFrontRef                             = 0;
            std::uint32_t           stencilBackRef                              = 0;
            bool                    stencilRefDynamic                           = false;

            MTLVisibilityResultMode visResultMode                               = MTLVisibilityResultModeDisabled;
            NSUInteger              visResultOffset                             = 0;
        };

        struct MTComputeEncoderState
        {
            MTComputePSO*   computePSO          = nullptr;
            MTResourceHeap* computeResourceHeap = nullptr;
            std::uint32_t   computeResourceSet  = 0;
        };

        // Context for state that is detached from Metal commands,
        // e.g. index buffer is provided per draw call in Metal, but LLGL uses a state cache via SetIndexBuffer().
        struct MTContextState
        {
            MTEncoderState              encoderState            = MTEncoderState::None;
            bool                        isInsideRenderPass      = false;

            id<MTLBuffer>               indexBuffer             = nil;
            NSUInteger                  indexBufferOffset       = 0;
            MTLIndexType                indexType               = MTLIndexTypeUInt32;
            NSUInteger                  indexTypeSize           = 4;

            MTPipelineState*            boundPipelineState      = nullptr;
            MTLPrimitiveType            primitiveType           = MTLPrimitiveTypeTriangle;
            MTLSize                     threadsPerThreadgroup   = MTLSizeMake(1, 1, 1);

            NSUInteger                  numPatchControlPoints   = 0;
            NSUInteger                  tessFactorSize          = 0;
            id<MTLComputePipelineState> tessPipelineState       = nil;

            id<MTLBuffer>               visBuffer               = nil;
        };

    private:

        id<MTLCommandBuffer>            cmdBuffer_              = nil;

        id<MTLRenderCommandEncoder>     renderEncoder_  	    = nil;
        id<MTLComputeCommandEncoder>    computeEncoder_         = nil;
        id<MTLBlitCommandEncoder>       blitEncoder_            = nil;

        MTLRenderPassDescriptor*        renderPassDesc_         = nullptr;
        MTRenderEncoderState            renderEncoderState_;
        MTComputeEncoderState           computeEncoderState_;
        MTContextState                  contextState_;

        bool                            isRenderEncoderPaused_  = false;
        MTDescriptorCache               descriptorCache_;
        MTConstantsCache                constantsCache_;
        MTIntermediateBuffer            tessFactorBuffer_;
        const NSUInteger                maxThreadgroupSizeX_    = 1;

        std::uint32_t                   renderDirtyBits_        = 0;
        std::uint32_t                   computeDirtyBits_       = 0;

        MTSwapChain*                    boundSwapChain_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
