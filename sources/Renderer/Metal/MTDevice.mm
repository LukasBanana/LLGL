/*
 * MTDevice.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTDevice.h"


namespace LLGL
{


NSUInteger MTDevice::FindSuitableSampleCount(id<MTLDevice> device, NSUInteger samples)
{
    while (samples > 1u)
    {
        if ([device supportsTextureSampleCount:samples])
            return samples;
        --samples;
    }
    return 4u; // Supported by all macOS and iOS devices; 1 is not supported according to Metal validation layer
}

NSUInteger MTDevice::FindSuitableSampleCountOr1(id<MTLDevice> device, NSUInteger samples)
{
    if (samples > 1u)
        return MTDevice::FindSuitableSampleCount(device, samples);
    else
        return 1u;
}


} // /namespace LLGL



// ================================================================================
