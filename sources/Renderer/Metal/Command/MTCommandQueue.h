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
#include "MTCommandContext.h"


namespace LLGL
{


class MTCommandQueue final : public CommandQueue
{

    public:

        #include <LLGL/Backend/CommandQueue.inl>

    public:

        MTCommandQueue(id<MTLDevice> device);
        ~MTCommandQueue();

    public:

        // Returns the native MTLCommandQueue object.
        inline id<MTLCommandQueue> GetNative() const
        {
            return native_;
        }

        // Submits the specified Metal command buffer.
        void SubmitCommandBuffer(id<MTLCommandBuffer> cmdBuffer);

    private:

        id<MTLCommandQueue>     native_                 = nil;
        id<MTLCommandBuffer>    lastSubmittedCmdBuffer_ = nil;
        MTCommandContext        context_;

};


} // /namespace LLGL


#endif



// ================================================================================
