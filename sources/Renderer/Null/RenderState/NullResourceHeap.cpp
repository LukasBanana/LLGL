/*
 * NullResourceHeap.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullResourceHeap.h"
#include "NullPipelineLayout.h"
#include "../../CheckedCast.h"


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutNull = LLGL_CAST(const NullPipelineLayout*, pipelineLayout);
    return std::max(1u, static_cast<std::uint32_t>(pipelineLayoutNull->desc.bindings.size()));
}

NullResourceHeap::NullResourceHeap(const ResourceHeapDescriptor& desc) :
    desc         { desc                                              },
    numBindings_ { GetNumPipelineLayoutBindings(desc.pipelineLayout) }
{
}

void NullResourceHeap::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

std::uint32_t NullResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(desc.resourceViews.size() / numBindings_);
}


} // /namespace LLGL



// ================================================================================
