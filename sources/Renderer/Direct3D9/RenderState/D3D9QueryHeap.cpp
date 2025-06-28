/*
 * D3D9QueryHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9QueryHeap.h"


namespace LLGL
{


D3D9QueryHeap::D3D9QueryHeap(const QueryHeapDescriptor& desc) :
    QueryHeap { desc.type },
    desc      { desc      }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9QueryHeap::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
