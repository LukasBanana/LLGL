/*
 * ContainerTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_CONTAINER_TYPES_H__
#define __LLGL_CONTAINER_TYPES_H__


#include <set>
#include <memory>


namespace LLGL
{


template <typename T>
using HWObjectContainer = std::set<std::unique_ptr<T>>;


} // /namespace LLGL


#endif



// ================================================================================
