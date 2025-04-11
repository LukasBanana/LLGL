/*
 * Vector.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VECTOR_H
#define LLGL_VECTOR_H

#ifndef LLGL_REPLACE_STD_VECTOR
#include <vector>
#endif

#ifdef LLGL_REPLACE_STD_VECTOR
template < typename Type >
using vector = LLGL_CUSTOM_VECTOR;
#else
template < typename Type >
using vector = std::vector< Type >;
#endif

#endif

// ================================================================================
