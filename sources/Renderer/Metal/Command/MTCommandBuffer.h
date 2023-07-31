/*
 * MTCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_BUFFER_H
#define LLGL_MT_COMMAND_BUFFER_H


#import <MetalKit/MetalKit.h>

#include <LLGL/CommandBuffer.h>
#include <LLGL/StaticLimits.h>
#include "MTCommandContext.h"
#include "../Buffer/MTIntermediateBuffer.h"
#include "../Buffer/MTStagingBufferPool.h"


namespace LLGL
{


class MTBuffer;
class MTTexture;
class MTSampler;
class MTSwapChain;
class MTRenderTarget;
class MTPipelineState;
class MTDescriptorCache;
class MTConstantsCache;
class MTCommandQueue;
class MTGraphicsPSO;
class MTComputePSO;

class MTCommandBuffer : public CommandBuffer
{

    public:

        /* ----- Input Assembly ------ */

        void SetIndexBuffer(Buffer& buffer) override final;
        void SetIndexBuffer(Buffer& buffer, const Format format, std::uint64_t offset = 0) override final;

        /* ----- Resources ----- */

        void SetResource(std::uint32_t descriptor, Resource& resource) override final;

        /* ----- Pipeline States ----- */

        void SetUniforms(std::uint32_t first, const void* data, std::uint16_t dataSize) override final;

    public:

        /*
        Returns true if this is a multi-submit command buffer (MTMultiSubmitCommandBuffer),
        otherwise it is a direct command buffer (MTDirectCommandBuffer).
        */
        virtual bool IsMultiSubmitCmdBuffer() const = 0;

    public:

        // Returns the flags this command buffer was created with.
        inline long GetFlags() const
        {
            return flags_;
        }

        // Returns true if this is a primary command buffer.
        inline bool IsPrimary() const
        {
            return ((GetFlags() & CommandBufferFlags::Secondary) == 0);
        }

    protected:

        // Active command encoder state enumeration.
        enum class MTEncoderState
        {
            None,
            Render,
            Compute,
            Blit,
        };

    protected:

        MTCommandBuffer(id<MTLDevice> device, long flags);

        NSUInteger GetMaxLocalThreads(id<MTLComputePipelineState> computePSO) const;

        void SetSwapChain(MTSwapChain* swapChainMT);

        void SetGraphicsPSORenderState(MTGraphicsPSO& graphicsPSO);
        void SetComputePSORenderState(MTComputePSO& computePSO);

        void ResetRenderStates();
        void ResetStagingPool();

        void WriteStagingBuffer(
            const void*     data,
            NSUInteger      dataSize,
            id<MTLBuffer>&  outSrcBuffer,
            NSUInteger&     outSrcOffset
        );

        // Returns the Metal view of the current drawable from the active framebuffer.
        MTKView* GetCurrentDrawableView() const;

        // Grows the internal tessellation-factor buffer to fit the specified number of patches and instances, then returns the native Metal buffer.
        id<MTLBuffer> GetTessFactorBufferAndGrow(NSUInteger numPatchesAndInstances);

    protected:

        inline MTLPrimitiveType GetPrimitiveType() const
        {
            return primitiveType_;
        }

        inline MTLIndexType GetIndexType() const
        {
            return indexType_;
        }

        inline id<MTLBuffer> GetIndexBuffer() const
        {
            return indexBuffer_;
        }

        inline NSUInteger GetIndexBufferOffset(NSUInteger firstIndex) const
        {
            return (indexBufferOffset_ + indexTypeSize_ * firstIndex);
        }

        inline NSUInteger GetNumPatchControlPoints() const
        {
            return numPatchControlPoints_;
        }

        inline const MTLSize& GetThreadsPerThreadgroup() const
        {
            return threadsPerThreadgroup_;
        }

        inline MTPipelineState* GetBoundPipelineState() const
        {
            return boundPipelineState_;
        }

        inline id<MTLComputePipelineState> GetTessPipelineState() const
        {
            return tessPipelineState_;
        }

    private:

        void SetIndexStream(id<MTLBuffer> indexBuffer, NSUInteger offset, bool indexType16Bits);

        void SetPipelineRenderState(MTPipelineState& pipelineStateMT);

    private:

        id<MTLDevice>                   device_                 = nil;
        long                            flags_                  = 0;

        MTStagingBufferPool             stagingBufferPool_;
        SmallVector<id<MTLDrawable>, 2> queuedDrawables_;

        MTLPrimitiveType                primitiveType_          = MTLPrimitiveTypeTriangle;
        id<MTLBuffer>                   indexBuffer_            = nil;
        NSUInteger                      indexBufferOffset_      = 0;
        MTLIndexType                    indexType_              = MTLIndexTypeUInt32;
        NSUInteger                      indexTypeSize_          = 4;
        NSUInteger                      numPatchControlPoints_  = 0;

        MTLSize                         threadsPerThreadgroup_  = MTLSizeMake(1, 1, 1);
        MTSwapChain*                    boundSwapChain_         = nullptr;
        MTPipelineState*                boundPipelineState_     = nullptr;
        MTDescriptorCache*              descriptorCache_        = nullptr;
        MTConstantsCache*               constantsCache_         = nullptr;

        // Tessellator stage objects
        MTIntermediateBuffer            tessFactorBuffer_;
        NSUInteger                      tessFactorSize_         = 0;
        id<MTLComputePipelineState>     tessPipelineState_      = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
