/*
 * DbgResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgResourceHeap.h"
#include "DbgPipelineLayout.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutDbg = LLGL_CAST(const DbgPipelineLayout*, pipelineLayout);
    return static_cast<std::uint32_t>(pipelineLayoutDbg->desc.bindings.size());
}

DbgResourceHeap::DbgResourceHeap(ResourceHeap& instance, const ResourceHeapDescriptor& desc) :
    instance    { instance                                                        },
    desc        { desc                                                            },
    numBindings { std::max(1u, GetNumPipelineLayoutBindings(desc.pipelineLayout)) }
{
}

void DbgResourceHeap::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

std::uint32_t DbgResourceHeap::GetNumDescriptorSets() const
{
    return instance.GetNumDescriptorSets();
}

std::uint32_t DbgResourceHeap::GetNumDescriptorSetsSafe() const
{
    return static_cast<std::uint32_t>(desc.resourceViews.size() / numBindings);
}


} // /namespace LLGL



// ================================================================================
