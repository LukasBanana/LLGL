/*
 * StaticAssertions.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STATIC_ASSERTIONS_H
#define LLGL_STATIC_ASSERTIONS_H


#include <type_traits>


#define LLGL_ASSERT_STDLAYOUT_STRUCT(IDENT) \
    static_assert(std::is_standard_layout<IDENT>::value, "LLGL::" #IDENT " must have standard layout")


#endif



// ================================================================================
