/*
 * Stack.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STACK_H
#define LLGL_STACK_H

#ifndef LLGL_REPLACE_STD_STACK
#include <stack>
#endif

#ifdef LLGL_REPLACE_STD_STACK
template < typename Type >
using stack = LLGL_CUSTOM_STACK;
#else
template < typename Type >
using stack = std::stack< Type >;
#endif

#endif

// ================================================================================
