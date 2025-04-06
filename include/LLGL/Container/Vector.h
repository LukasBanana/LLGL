/*
 * Vector.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VECTOR_H
#define LLGL_VECTOR_H

#include <LLGL/Container/STLAllocator.h>

#include <vector>

namespace LLGL{

template < typename Type >
using vector = std::vector< Type, llgl_allocator< Type > >;

} // /namespace LLGL

#endif

// ================================================================================
