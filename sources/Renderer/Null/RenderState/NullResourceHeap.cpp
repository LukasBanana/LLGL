/*
 * NullResourceHeap.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullResourceHeap.h"
#include "NullPipelineLayout.h"
#include "../../CheckedCast.h"
#include <LLGL/Misc/ForRange.h>
#include <algorithm>


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutNull = LLGL_CAST(const NullPipelineLayout*, pipelineLayout);
    return std::max(1u, static_cast<std::uint32_t>(pipelineLayoutNull->desc.bindings.size()));
}

static std::uint32_t GetNumResourceViews(const ResourceHeapDescriptor& desc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    if (desc.numResourceViews > 0)
        return desc.numResourceViews;
    else
        return static_cast<std::uint32_t>(initialResourceViews.size());
}

NullResourceHeap::NullResourceHeap(const ResourceHeapDescriptor& desc, const ArrayView<ResourceViewDescriptor>& initialResourceViews) :
    numBindings_   { GetNumPipelineLayoutBindings(desc.pipelineLayout)        },
    resourceViews_ { initialResourceViews.begin(), initialResourceViews.end() }
{
    if (desc.numResourceViews > 0)
        resourceViews_.resize(desc.numResourceViews);
}

std::uint32_t NullResourceHeap::WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    /* Copy input resource views into resource heap via STL copy algorithm, since the descriptors are non-POD structs */
    std::uint32_t numWritten = 0;
    if (resourceViews.size() + firstDescriptor < resourceViews_.size())
    {
        for_range(i, resourceViews.size())
        {
            const auto& resourceView = resourceViews[i];
            if (resourceView.resource != nullptr)
            {
                resourceViews_[firstDescriptor + i] = resourceView;
                ++numWritten;
            }
        }
    }
    return numWritten;
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
    return (static_cast<std::uint32_t>(resourceViews_.size()) / numBindings_);
}


} // /namespace LLGL



// ================================================================================
