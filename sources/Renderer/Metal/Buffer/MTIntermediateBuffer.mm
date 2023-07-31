/*
 * MTIntermediateBuffer.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTIntermediateBuffer.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


MTIntermediateBuffer::MTIntermediateBuffer(
    id<MTLDevice>       device,
    MTLResourceOptions  options,
    NSUInteger          alignment,
    NSUInteger          initialSize)
:
    device_    { device    },
    options_   { options   },
    alignment_ { alignment }
{
}

MTIntermediateBuffer::~MTIntermediateBuffer()
{
    [native_ release];
}

void MTIntermediateBuffer::Resize(NSUInteger size)
{
    /* Release previous native buffer */
    if (native_ != nil)
        [native_ release];

    /* Allocate new native buffer */
    size = GetAlignedSize(size, alignment_);
    native_ = [device_ newBufferWithLength:size options:options_];
}

void MTIntermediateBuffer::Grow(NSUInteger size)
{
    if (size > 0)
    {
        if (native_ == nil)
            Resize(size);
        else if (size > [native_ length])
            Resize(size + size/2);
    }
}


} // /namespace LLGL



// ================================================================================
