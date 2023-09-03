/*
 * MTDevice.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_DEVICE_H
#define LLGL_MT_DEVICE_H


#import <MetalKit/MetalKit.h>


namespace LLGL
{


// Metal device wrapper. Currently only used for Metal utility functions.
class MTDevice
{

    public:

        // Returns the most suitable sample count for the Metal device
        static NSUInteger FindSuitableSampleCount(id<MTLDevice> device, NSUInteger samples);

        /*
        Returns the most suitable sample count unless the requested input is 1 in which case the return value is also 1.
        Use the other function if a multisample texture must be created, in which case the return value cannot be less than or equal to 1.
        */
        static NSUInteger FindSuitableSampleCountOr1(id<MTLDevice> device, NSUInteger samples);

};


} // /namespace LLGL


#endif



// ================================================================================
