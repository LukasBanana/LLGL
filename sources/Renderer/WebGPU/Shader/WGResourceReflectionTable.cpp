/*
 * WGResourceReflectionTable.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGResourceReflectionTable.h"


namespace LLGL
{


std::size_t WGResourceReflectionTable::RegisterResourceType(WGResourceReflection&& resource)
{
    const std::size_t outIndex = resourceTypes.size();
    resourceTypes.push_back(std::move(resource));
    return outIndex;
}

const WGResourceReflection* WGResourceReflectionTable::FindResource(const char* name) const
{
    auto it = resources.find(name);
    return (it != resources.end() ? &(resourceTypes[it->second]) : nullptr);
}


} // /namespace LLGL



// ================================================================================
