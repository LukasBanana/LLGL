/*
 * STL Allocator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STL_ALLOCATOR_H
#define LLGL_STL_ALLOCATOR_H

#ifndef LLGL_CUSTOM_ALLOCATOR
#include <memory>
#endif

namespace LLGL{

#ifndef LLGL_CUSTOM_ALLOCATOR

template < typename Type >
using llgl_allocator = std::allocator< Type >;

#else

// Is up to the user to implement a custom allocator 'llgl_allocator'

#endif

} // /namespace LLGL

#endif



// ================================================================================
