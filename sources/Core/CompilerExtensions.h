/*
 * CompilerExtensions.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_COMPILER_EXTENSIONS_H
#define LLGL_COMPILER_EXTENSIONS_H


#if __cplusplus >= 202002L // C++20
#   define likely(COND)         (COND) [[likely]]
#   define unlikely(COND)       (COND) [[unlikely]]
#elif defined __GNUC__ // GNU extensions
#   define likely(COND)         (__builtin_expect(!!(COND), 1))
#   define unlikely(COND)       (__builtin_expect(!!(COND), 0))
#else
#   define likely(COND)         (COND)
#   define unlikely(COND)       (COND)
#endif

#if __cplusplus >= 201703L // C++17
#   define LLGL_MAYBE_UNUSED    [[maybe_unused]]
#elif defined __GNUC__ || defined __clang__ // GNU/Clang extensions
#   define LLGL_MAYBE_UNUSED    __attribute__((unused))
#else
#   define LLGL_MAYBE_UNUSED    
#endif

#if __cplusplus >= 201703L // C++17
#   define LLGL_NODISCARD       [[nodiscard]]
#elif defined __GNUC__ || defined __clang__ // GNU/Clang extensions
#   define LLGL_NODISCARD       __attribute__((warn_unused_result))
#elif _MSC_VER >= 1700 // MSVC extensions
#   define LLGL_NODISCARD       _Check_return_
#else
#   define LLGL_NODISCARD
#endif


#endif



// ================================================================================
