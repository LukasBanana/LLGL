/*
 * MTFence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
