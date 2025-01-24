/*
 * Throw.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_THROW_H
#define LLGL_THROW_H

#if __EXCEPTIONS || __cpp_exceptions
    #define LLGL_COMPILER_EXCEPTIONS_ENABLED
#endif

#if (defined(LLGL_DEBUG) || defined(LLGL_ENABLE_EXCEPTIONS)) && defined(LLGL_COMPILER_EXCEPTIONS_ENABLED)
    #define LLGL_THROW(exception) throw (exception)
    #define LLGL_THROW_IF(condition, exception) if (condition) { LLGL_THROW(exception); }
#else
    #define LLGL_THROW(exception) std::abort()
    #define LLGL_THROW_IF(condition, exception)
#endif

#endif



// ================================================================================
