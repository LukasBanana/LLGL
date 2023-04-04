/*
 * MTCommandQueue.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMMAND_QUEUE_H
#define LLGL_MT_COMMAND_QUEUE_H


#import <MetalKit/MetalKit.h>

#include <LLGL/CommandQueue.h>


namespace LLGL
{


class MTCommandQueue final : public CommandQueue
{

    public:

        /* ----- Common ----- */

        MTCommandQueue(id<MTLDevice> device);
        ~MTCommandQueue();

        /* ----- Command Buffers ----- */

        void Submit(CommandBuffer& commandBuffer) override;

        /* ----- Queries ----- */

        bool QueryResult(
            QueryHeap&      queryHeap,
            std::uint32_t   firstQuery,
            std::uint32_t   numQueries,
            void*           data,
            std::size_t     dataSize
        ) override;

        /* ----- Fences ----- */

        void Submit(Fence& fence) override;

        bool WaitFence(Fence& fence, std::uint64_t timeout) override;
        void WaitIdle() override;
    
    public:

        // Returns the native MTLCommandQueue object.
        inline id<MTLCommandQueue> GetNative() const
        {
            return native_;
        }

    private:

        id<MTLCommandQueue>     native_                 = nil;
        id<MTLCommandBuffer>    lastSubmittedCmdBuffer_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
