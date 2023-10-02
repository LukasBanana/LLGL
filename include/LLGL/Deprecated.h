/*
 * Deprecated.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DEPRECATED_H
#define LLGL_DEPRECATED_H


// Macro to support deprecation warnings before C++14

#if defined __clang__ // Prefer Clang deprecation attribute as it provides better diagnostics than the C++14 attribute
#   define LLGL_DEPRECATED(MESSAGE, SUGGESTION) __attribute__((deprecated(MESSAGE, SUGGESTION)))
#elif __cplusplus >= 201402L // C++14
#   define LLGL_DEPRECATED(MESSAGE, SUGGESTION) [[deprecated(MESSAGE)]]
#elif defined __GNUC__
#   define LLGL_DEPRECATED(MESSAGE, SUGGESTION) __attribute__((deprecated(MESSAGE)))
#elif defined _MSC_VER
#   define LLGL_DEPRECATED(MESSAGE, SUGGESTION) __declspec(deprecated(MESSAGE))
#else
#   define LLGL_DEPRECATED(MESSAGE, SUGGESTION)
#endif


#endif



// ================================================================================
