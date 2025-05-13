/*
 * Set.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SET_H
#define LLGL_SET_H

#include <LLGL/STL/STLAllocator.h>

#include <set>
#include <utility>

namespace LLGL
{

namespace STL
{

template < typename Type, typename Less = std::less< Type > >
using set = std::set< Type, Less, allocator< Type > >;

}

} // /namespace LLGL

#endif

// ================================================================================
