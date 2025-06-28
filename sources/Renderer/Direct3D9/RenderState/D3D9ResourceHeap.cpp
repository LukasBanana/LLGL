/*
 * D3D9ResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9ResourceHeap.h"
#include "D3D9PipelineLayout.h"
#include "../../CheckedCast.h"
#include "../../ResourceUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static std::uint32_t GetNumPipelineLayoutBindings(const PipelineLayout* pipelineLayout)
{
    auto pipelineLayoutD3D9 = LLGL_CAST(const D3D9PipelineLayout*, pipelineLayout);
    return std::max(1u, static_cast<std::uint32_t>(pipelineLayoutD3D9->desc.heapBindings.size()));
}

D3D9ResourceHeap::D3D9ResourceHeap(const ResourceHeapDescriptor& desc, const ArrayView<ResourceViewDescriptor>& initialResourceViews) :
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

std::uint32_t D3D9ResourceHeap::WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
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

void D3D9ResourceHeap::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

std::uint32_t D3D9ResourceHeap::GetNumDescriptorSets() const
{
    return (static_cast<std::uint32_t>(resourceViews_.size()) / numBindings_);
}


} // /namespace LLGL



// ================================================================================
