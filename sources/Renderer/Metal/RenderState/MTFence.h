/*
 * MTFence.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_FENCE_H
#define LLGL_MT_FENCE_H


#import <Metal/Metal.h>

#include <LLGL/Fence.h>


namespace LLGL
{


//TODO: currently unused
class MTFence final : public Fence
{

    public:

        MTFence(id<MTLDevice> device);
        ~MTFence();

        inline id<MTLFence> GetNative() const
        {
            return native_;
        }

    private:

        id<MTLFence> native_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
