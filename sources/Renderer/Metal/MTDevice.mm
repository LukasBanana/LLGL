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
    while (samples > 1)
    {
        if ([device supportsTextureSampleCount:samples])
            return samples;
        --samples;
    }
    return 4u; // Supported by all macOS and iOS devices; 1 is not supported according to Metal validation layer
}



} // /namespace LLGL



// ================================================================================
