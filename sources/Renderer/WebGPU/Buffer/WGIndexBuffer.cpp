/*
 * WGIndexBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGIndexBuffer.h"
#include "../WGTypes.h"


namespace LLGL
{


WGIndexBuffer::WGIndexBuffer(WGPUDevice device, const BufferDescriptor& bufferDesc) :
    WGBuffer     { device, bufferDesc                          },
    indexFormat_ { WGTypes::ToWGIndexFormat(bufferDesc.format) }
{
}


} // /namespace LLGL



// ================================================================================
