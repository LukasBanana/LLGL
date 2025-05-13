/*
 * Map.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MAP_H
#define LLGL_MAP_H

#include <LLGL/STL/STLAllocator.h>

#include <map>
#include <utility>

namespace LLGL
{

template < typename Key, typename Value, typename Less = std::less< Key > >
using map = std::map< Key, Value, Less, llgl_allocator< std::pair< Key const, Value > > >;

} // /namespace LLGL

#endif

// ================================================================================
