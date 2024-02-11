/*
 * DbgQueryHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgQueryHeap.h"
#include "../DbgCore.h"


namespace LLGL
{


DbgQueryHeap::DbgQueryHeap(QueryHeap& instance, const QueryHeapDescriptor& desc) :
    QueryHeap { desc.type            },
    instance  { instance             },
    desc      { desc                 },
    label     { LLGL_DBG_LABEL(desc) }
{
    states.resize(desc.numQueries, State::Uninitialized);
}

void DbgQueryHeap::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
