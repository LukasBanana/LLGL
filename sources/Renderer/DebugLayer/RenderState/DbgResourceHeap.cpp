/*
 * DbgResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgResourceHeap.h"
#include "DbgPipelineLayout.h"
#include "../DbgCore.h"
#include "../../CheckedCast.h"


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutDbg = LLGL_CAST(const DbgPipelineLayout*, pipelineLayout);
    return std::max(1u, static_cast<std::uint32_t>(pipelineLayoutDbg->desc.heapBindings.size()));
}

DbgResourceHeap::DbgResourceHeap(ResourceHeap& instance, const ResourceHeapDescriptor& desc) :
    instance    { instance                                          },
    desc        { desc                                              },
    label       { LLGL_DBG_LABEL(desc)                              },
    numBindings { GetNumPipelineLayoutBindings(desc.pipelineLayout) }
{
}

void DbgResourceHeap::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

std::uint32_t DbgResourceHeap::GetNumDescriptorSets() const
{
    return instance.GetNumDescriptorSets();
}

std::uint32_t DbgResourceHeap::GetNumDescriptorSetsSafe() const
{
    return static_cast<std::uint32_t>(desc.numResourceViews / numBindings);
}


} // /namespace LLGL



// ================================================================================
