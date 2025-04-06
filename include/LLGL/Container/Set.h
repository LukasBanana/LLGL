/*
 * Set.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SET_H
#define LLGL_SET_H

#include <LLGL/Container/STLAllocator.h>

#include <set>
#include <utility>

namespace LLGL{

template < typename Type, typename Less = std::less< Type > >
using set = std::set< Type, Less, llgl_allocator< Type > >;

} // /namespace LLGL

#endif

// ================================================================================
