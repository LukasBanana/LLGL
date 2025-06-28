/*
 * D3D9Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Buffer.h"


namespace LLGL
{


D3D9Buffer::D3D9Buffer(long bindFlags) :
    Buffer { bindFlags }
{
}

void D3D9Buffer::SetDebugName(const char* name)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
