/*
 * D3D11CommandBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11CommandBuffer.h"


namespace LLGL
{


D3D11CommandBuffer::D3D11CommandBuffer(bool isSecondaryCmdBuffer) :
    isSecondaryCmdBuffer_ { isSecondaryCmdBuffer }
{
}


} // /namespace LLGL



// ================================================================================
