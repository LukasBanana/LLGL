/*
 * NullQueryHeap.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullQueryHeap.h"


namespace LLGL
{


NullQueryHeap::NullQueryHeap(const QueryHeapDescriptor& desc) :
    QueryHeap { desc.type },
    desc      { desc      }
{
}

void NullQueryHeap::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
