/*
 * CompilerExtensions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMPILER_EXTENSIONS_H
#define LLGL_COMPILER_EXTENSIONS_H


#if __cplusplus >= 202002L // C++20
#   define likely(COND)     (COND) [[likely]]
#   define unlikely(COND)   (COND) [[unlikely]]
#elif defined __GNUC__ // GNU extensions
#   define likely(COND)     (__builtin_expect(!!(COND), 1))
#   define unlikely(COND)   (__builtin_expect(!!(COND), 0))
#else
#   define likely(COND)     (COND)
#   define unlikely(COND)   (COND)
#endif


#endif



// ================================================================================
