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
    #define LLGL_THROW(EXCEPTION) throw (EXCEPTION)
    #define LLGL_THROW_IF(CONDITION, EXCEPTION) if (CONDITION) { LLGL_THROW(EXCEPTION); }
#else
    #define LLGL_THROW(EXCEPTION) std::abort()
    #define LLGL_THROW_IF(CONDITION, EXCEPTION)
#endif

#endif



// ================================================================================
