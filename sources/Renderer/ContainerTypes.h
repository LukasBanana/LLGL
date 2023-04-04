/*
 * ContainerTypes.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CONTAINER_TYPES_H
#define LLGL_CONTAINER_TYPES_H


#include <set>
#include <memory>


namespace LLGL
{


template <typename T>
using HWObjectInstance = std::unique_ptr<T>;

template <typename T>
using HWObjectContainer = std::set<HWObjectInstance<T>>;


} // /namespace LLGL


#endif



// ================================================================================
