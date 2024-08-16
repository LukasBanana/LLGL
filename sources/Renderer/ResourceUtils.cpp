/*
 * ResourceUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ResourceUtils.h"
#include "../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


LLGL_EXPORT std::uint32_t GetNumResourceViewsOrThrow(
    std::uint32_t                               numBindings,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Resource heaps cannot have pipeline layout with no bindings */
    if (numBindings == 0)
        LLGL_TRAP("cannot create resource heap without bindings in pipeline layout");

    /* Resource heaps cannot be empty */
    const std::uint32_t numResourceViews = (desc.numResourceViews > 0 ? desc.numResourceViews : static_cast<std::uint32_t>(initialResourceViews.size()));
    if (numResourceViews == 0)
        LLGL_TRAP("cannot create empty resource heap");

    /* Number of resources must be a multiple of bindings */
    if (numResourceViews % numBindings != 0)
    {
        LLGL_TRAP(
            "cannot create resource heap because number of resources (%u) is not a multiple of bindings (%u)",
            numResourceViews, numBindings
        );
    }

    return numResourceViews;
}

LLGL_EXPORT StaticSamplerBorderColor GetStaticSamplerBorderColor(const float (&color)[4])
{
    if (color[3] > 0.5f)
    {
        if (color[0] <= 0.5f && color[1] <= 0.5f && color[2] <= 0.5f)
            return StaticSamplerBorderColor::OpaqueBlack;
        if (color[0] > 0.5f && color[1] > 0.5f && color[2] > 0.5f)
            return StaticSamplerBorderColor::OpaqueWhite;
    }
    return StaticSamplerBorderColor::TransparentBlack;
}

static std::uint32_t GetNumExpandedHeapDescriptors(const ArrayView<BindingDescriptor>& bindingDescs)
{
    std::uint32_t n = 0;
    for (const BindingDescriptor& binding : bindingDescs)
        n += std::max<std::uint32_t>(1u, binding.arraySize);
    return n;
}

LLGL_EXPORT DynamicVector<BindingDescriptor> GetExpandedHeapDescriptors(const ArrayView<BindingDescriptor>& bindingDescs)
{
    /* Determine final number of binding descriptors */
    const std::size_t numDescriptors = GetNumExpandedHeapDescriptors(bindingDescs);

    DynamicVector<BindingDescriptor> expandedBindingDescs;
    expandedBindingDescs.reserve(numDescriptors);

    for (const BindingDescriptor& binding : bindingDescs)
    {
        for_range(i, std::max<std::uint32_t>(1u, binding.arraySize))
        {
            expandedBindingDescs.push_back(binding);
            expandedBindingDescs.back().slot.index += i;
        }
    }

    return expandedBindingDescs;
}


} // /namespace LLGL



// ================================================================================
