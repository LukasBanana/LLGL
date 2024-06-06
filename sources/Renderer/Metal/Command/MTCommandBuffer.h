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
#include <LLGL/Constants.h>
#include "MTCommandContext.h"
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

        static constexpr NSUInteger maxNumCommandBuffersInFlight = 3;

        MTCommandBuffer(id<MTLDevice> device, long flags);

        void ResetRenderStates();
        void ResetStagingPool();

        void WriteStagingBuffer(
            const void*     data,
            NSUInteger      dataSize,
            id<MTLBuffer>&  outSrcBuffer,
            NSUInteger&     outSrcOffset
        );

    private:

        id<MTLDevice>                   device_                 = nil;
        long                            flags_                  = 0;

        NSUInteger                      currentStagingPool_     = 0;
        MTStagingBufferPool             stagingBufferPools_[MTCommandBuffer::maxNumCommandBuffersInFlight];
        SmallVector<id<MTLDrawable>, 2> queuedDrawables_;

};


} // /namespace LLGL


#endif



// ================================================================================
