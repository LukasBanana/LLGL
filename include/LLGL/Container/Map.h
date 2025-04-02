/*
 * Map.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MAP_H
#define LLGL_MAP_H

#ifndef LLGL_REPLACE_STD_MAP
#include <map>
#endif

#ifdef LLGL_REPLACE_STD_MAP
template < typename Key, typename Value, typename Less >
using map = LLGL_CUSTOM_MAP;
#else
template < typename Key, typename Value, typename Less = std::less< Key > >
using map = std::map< Key, Value, Less >;
#endif

#endif

// ================================================================================
