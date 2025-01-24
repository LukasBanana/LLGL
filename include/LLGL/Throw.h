/*
 * Throw.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_THROW_H
#define LLGL_THROW_H

#if __EXCEPTIONS || __cpp_exceptions == 199711
    #define LLGL_COMPILER_EXCEPTIONS_ENABLED 1
#endif

#if (LLGL_DEBUG || LLGL_ENABLE_EXCEPTIONS) && LLGL_COMPILER_EXCEPTIONS_ENABLED
    #define LLGL_THROW(EXCEPTION) throw (EXCEPTION)
    #define LLGL_THROW_IF(CONDITION, EXCEPTION) if (CONDITION) { LLGL_THROW(EXCEPTION); }
    #define LLGL_VERIFY_OR_THROW(CONDITION, EXCEPTION) LLGL_THROW_IF(!(CONDITION), EXCEPTION)
#else
    #include <cstdlib>

    #define LLGL_THROW(EXCEPTION) std::abort()
    #define LLGL_THROW_IF(CONDITION, EXCEPTION)
#endif

#endif



// ================================================================================
