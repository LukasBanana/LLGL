/*
 * Stack.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STACK_H
#define LLGL_STACK_H

#include <LLGL/STL/STLAllocator.h>

#include <stack>
#include <deque>

namespace LLGL
{

namespace STL
{

template < typename Type, typename Container = std::deque< Type, allocator< Type > > >
using stack = std::stack< Type, Container >;

}

} // /namespace LLGL

#endif

// ================================================================================
