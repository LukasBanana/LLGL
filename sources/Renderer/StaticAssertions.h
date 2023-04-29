/*
 * StaticAssertions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STATIC_ASSERTIONS_H
#define LLGL_STATIC_ASSERTIONS_H


#include <type_traits>


#define LLGL_ASSERT_STDLAYOUT_STRUCT(IDENT) \
    static_assert(std::is_standard_layout<IDENT>::value, "LLGL::" #IDENT " must have standard layout")

#define LLGL_ASSERT_POD_TYPE(IDENT) \
    static_assert(std::is_pod<IDENT>::value, "LLGL::" #IDENT " must be a plain-old-data (POD) type");


#endif



// ================================================================================
