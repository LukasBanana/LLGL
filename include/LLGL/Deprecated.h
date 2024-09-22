/*
 * Deprecated.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DEPRECATED_H
#define LLGL_DEPRECATED_H


// Macro to support deprecation warnings before C++14

#define LLGL_DEPRECATED_VA_ARGS(...) \
    , ## __VA_ARGS__

#if defined __clang__ // Prefer Clang deprecation attribute as it provides better diagnostics than the C++14 attribute

#   define LLGL_DEPRECATED(MESSAGE, ...) \
        __attribute__((deprecated(MESSAGE LLGL_DEPRECATED_VA_ARGS(__VA_ARGS__))))

#   define LLGL_DEPRECATED_IGNORE_PUSH()                                    \
        _Pragma("clang diagnostic push")                                    \
        _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")

#   define LLGL_DEPRECATED_IGNORE_POP() \
        _Pragma("clang diagnostic pop")

#elif defined __GNUC__

#   define LLGL_DEPRECATED(MESSAGE, ...) \
        __attribute__((deprecated(MESSAGE)))

#   define LLGL_DEPRECATED_IGNORE_PUSH()                                \
        _Pragma("GCC diagnostic push")                                  \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#   define LLGL_DEPRECATED_IGNORE_POP() \
        _Pragma("GCC diagnostic pop")

#elif defined _MSC_VER

#   define LLGL_DEPRECATED(MESSAGE, ...) \
        __declspec(deprecated(MESSAGE))

#   define LLGL_DEPRECATED_IGNORE_PUSH()    \
        __pragma(warning(push))             \
        __pragma(warning(disable:4996))

#   define LLGL_DEPRECATED_IGNORE_POP() \
        __pragma(warning(pop))

#elif __cplusplus >= 201402L // C++14

#   define LLGL_DEPRECATED(MESSAGE, ...) \
        [[deprecated(MESSAGE)]]

#   define LLGL_DEPRECATED_IGNORE_PUSH()

#   define LLGL_DEPRECATED_IGNORE_POP()

#else

#   define LLGL_DEPRECATED(MESSAGE, ...)

#   define LLGL_DEPRECATED_IGNORE_PUSH()

#   define LLGL_DEPRECATED_IGNORE_POP()

#endif


#endif



// ================================================================================
