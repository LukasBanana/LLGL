/*
 * Set.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SET_H
#define LLGL_SET_H

#ifndef LLGL_REPLACE_STD_SET
#include <set>
#endif

#ifdef LLGL_REPLACE_STD_SET
template < typename Type >
using set = LLGL_CUSTOM_SET;
#else
template < typename Type >
using set = std::set< Type >;
#endif

#endif

// ================================================================================
