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

LLGL_EXPORT UniformType MakeUniformMatrixType(UniformType baseType, std::uint32_t elements)
{
    switch (baseType)
    {
        case UniformType::Float1:
            switch (elements)
            {
                case 1: return UniformType::Float1;
                case 2: return UniformType::Float2;
                case 3: return UniformType::Float3;
                case 4: return UniformType::Float4;
            }
            break;

        case UniformType::Float2:
            switch (elements)
            {
                case 1: return UniformType::Float2;
                case 2: return UniformType::Float2x2;
                case 3: return UniformType::Float2x3;
                case 4: return UniformType::Float2x4;
            }
            break;

        case UniformType::Float3:
            switch (elements)
            {
                case 1: return UniformType::Float3;
                case 2: return UniformType::Float3x2;
                case 3: return UniformType::Float3x3;
                case 4: return UniformType::Float3x4;
            }
            break;

        case UniformType::Float4:
            switch (elements)
            {
                case 1: return UniformType::Float4;
                case 2: return UniformType::Float4x2;
                case 3: return UniformType::Float4x3;
                case 4: return UniformType::Float4x4;
            }
            break;

        case UniformType::Double1:
            switch (elements)
            {
                case 1: return UniformType::Double1;
                case 2: return UniformType::Double2;
                case 3: return UniformType::Double3;
                case 4: return UniformType::Double4;
            }
            break;

        case UniformType::Double2:
            switch (elements)
            {
                case 1: return UniformType::Double2;
                case 2: return UniformType::Double2x2;
                case 3: return UniformType::Double2x3;
                case 4: return UniformType::Double2x4;
            }
            break;

        case UniformType::Double3:
            switch (elements)
            {
                case 1: return UniformType::Double3;
                case 2: return UniformType::Double3x2;
                case 3: return UniformType::Double3x3;
                case 4: return UniformType::Double3x4;
            }
            break;

        case UniformType::Double4:
            switch (elements)
            {
                case 1: return UniformType::Double4;
                case 2: return UniformType::Double4x2;
                case 3: return UniformType::Double4x3;
                case 4: return UniformType::Double4x4;
            }
            break;

        default:
            break;
    }
    return UniformType::Undefined;
}

LLGL_EXPORT UniformType MakeUniformVectorType(UniformType baseType, std::uint32_t elements)
{
    switch (baseType)
    {
        case UniformType::Float1:
            switch (elements)
            {
                case 1: return UniformType::Float1;
                case 2: return UniformType::Float2;
                case 3: return UniformType::Float3;
                case 4: return UniformType::Float4;
            }
            break;

        case UniformType::Double1:
            switch (elements)
            {
                case 1: return UniformType::Double1;
                case 2: return UniformType::Double2;
                case 3: return UniformType::Double3;
                case 4: return UniformType::Double4;
            }
            break;

        case UniformType::Int1:
            switch (elements)
            {
                case 1: return UniformType::Int1;
                case 2: return UniformType::Int2;
                case 3: return UniformType::Int3;
                case 4: return UniformType::Int4;
            }
            break;

        case UniformType::UInt1:
            switch (elements)
            {
                case 1: return UniformType::UInt1;
                case 2: return UniformType::UInt2;
                case 3: return UniformType::UInt3;
                case 4: return UniformType::UInt4;
            }
            break;

        case UniformType::Bool1:
            switch (elements)
            {
                case 1: return UniformType::Bool1;
                case 2: return UniformType::Bool2;
                case 3: return UniformType::Bool3;
                case 4: return UniformType::Bool4;
            }
            break;

        default:
            break;
    }
    return UniformType::Undefined;
}


} // /namespace LLGL



// ================================================================================
