/*
 * MTTessFactorBuffer.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTTessFactorBuffer.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


MTTessFactorBuffer::MTTessFactorBuffer(id<MTLDevice> device, NSUInteger initialSize) :
    device_ { device }
{
}

MTTessFactorBuffer::~MTTessFactorBuffer()
{
    [native_ release];
}

void MTTessFactorBuffer::Grow(NSUInteger size)
{
    if (size > 0 && (native_ == nil || size > [native_ length]))
    {
        /* Release previous native buffer */
        if (native_ != nil)
            [native_ release];

        /* Allocate new native buffer */
        const NSUInteger alignment = sizeof(MTLQuadTessellationFactorsHalf) * 256;
        size = GetAlignedSize(size*2, alignment);

        native_ = [device_ newBufferWithLength:size options:MTLResourceStorageModePrivate];
    }
}


} // /namespace LLGL



// ================================================================================
