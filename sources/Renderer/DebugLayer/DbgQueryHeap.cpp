/*
 * DbgQueryHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgQueryHeap.h"
#include "DbgCore.h"


namespace LLGL
{


DbgQueryHeap::DbgQueryHeap(QueryHeap& instance, const QueryHeapDescriptor& desc) :
    QueryHeap        { desc.type },
    instance         { instance  },
    desc             { desc      }
{
    states.resize(desc.numQueries, State::Uninitialized);
}

void DbgQueryHeap::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
