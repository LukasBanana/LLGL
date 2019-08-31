/*
 * ContainerTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
