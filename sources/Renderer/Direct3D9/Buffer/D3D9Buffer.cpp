/*
 * D3D9Buffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Buffer.h"
#include "../../../Core/Assertion.h"
#include "../D3D9Core.h"
#include <string.h>


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


/*
 * ======= Protected: =======
 */

DWORD D3D9Buffer::GetUsageFlags(long /*bindFlags*/, long /*cpuAccessFlags*/, long miscFlags)
{
    DWORD usageFlags = D3DUSAGE_WRITEONLY;

    if ((miscFlags & MiscFlags::DynamicUsage) != 0)
        usageFlags |= D3DUSAGE_DYNAMIC;

    return usageFlags;
}


} // /namespace LLGL



// ================================================================================
