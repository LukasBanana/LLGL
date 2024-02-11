/*
 * NullResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullResourceHeap.h"
#include "NullPipelineLayout.h"
#include "../../CheckedCast.h"
#include "../../ResourceUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutNull = LLGL_CAST(const NullPipelineLayout*, pipelineLayout);
    return std::max(1u, static_cast<std::uint32_t>(pipelineLayoutNull->desc.bindings.size()));
}

NullResourceHeap::NullResourceHeap(const ResourceHeapDescriptor& desc, const ArrayView<ResourceViewDescriptor>& initialResourceViews) :
    numBindings_   { GetNumPipelineLayoutBindings(desc.pipelineLayout)        },
    resourceViews_ { initialResourceViews.begin(), initialResourceViews.end() }
{
    const std::uint32_t numResourceViews = GetNumResourceViewsOrThrow(numBindings_, desc, initialResourceViews);
    if (numResourceViews > 0)
        resourceViews_.resize(numResourceViews);

    if (!initialResourceViews.empty())
        WriteResourceViews(0, initialResourceViews);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
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

void NullResourceHeap::SetDebugName(const char* name)
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
