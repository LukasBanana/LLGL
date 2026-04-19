/*
 * WGResourceReflectionTable.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RESOURCE_REFLECTION_TABLE_H
#define LLGL_WG_RESOURCE_REFLECTION_TABLE_H


#include "WGResourceReflection.h"
#include <vector>
#include <map>
#include <string>
#include <cstddef>


namespace LLGL
{


struct WGResourceReflectionTable
{
    std::vector<WGResourceReflection>   resourceTypes;
    std::map<std::string, std::size_t>  resources;
    std::map<std::string, std::size_t>  typeAliases;

    std::size_t RegisterResourceType(WGResourceReflection&& resource);
    const WGResourceReflection* FindResource(const char* name) const;
};


} // /namespace LLGL


#endif



// ================================================================================
