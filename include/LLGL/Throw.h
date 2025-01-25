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

#if LLGL_ENABLE_EXCEPTIONS && LLGL_COMPILER_EXCEPTIONS_ENABLED
    #define LLGL_VERIFY_OR_THROW(CONDITION, EXCEPTION) if (!(CONDITION)) { throw (EXCEPTION); }
#else
    #include <cstdlib>

    #define LLGL_VERIFY_OR_THROW(CONDITION, EXCEPTION) std::abort()
#endif

#endif



// ================================================================================
