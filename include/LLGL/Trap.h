/*
 * Trap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TRAP_H
#define LLGL_TRAP_H


#include <LLGL/Export.h>


//! Shortcut to throw a runtime error exception.
#define LLGL_THROW_RUNTIME_ERROR(...) \
    LLGL::Trap(LLGL::Exception::RuntimeError, __FUNCTION__, __VA_ARGS__)

//! Shortcut to throw an out of range exception.
#define LLGL_THROW_OUT_OF_RANGE(...) \
    LLGL::Trap(LLGL::Exception::OutOfRange, __FUNCTION__, __VA_ARGS__)

//! Shortcut to throw a bad cast exception.
#define LLGL_THROW_BAD_CAST(...) \
    LLGL::Trap(LLGL::Exception::BadCast, __FUNCTION__, __VA_ARGS__)

//! Shortcut to throw an invalid argument exception.
#define LLGL_THROW_INVALID_ARGUMENT(...) \
    LLGL::Trap(LLGL::Exception::InvalidArgument, __FUNCTION__, __VA_ARGS__)

/**
\brief Helper macro to trap execution when the condition fails.
\remarks LLGL only throws exceptions if it was built with \c LLGL_ENABLE_EXCEPTIONS.
\see LLGL::Trap
*/
#define LLGL_VERIFY_EXT(EXCEPTION, CONDITION)                                                       \
    if (!(CONDITION))                                                                               \
    {                                                                                               \
        LLGL::Trap(LLGL::Exception::EXCEPTION, __FUNCTION__, "assertion failed: %s", #CONDITION);   \
    }

//! Shortcut for LLGL_VERIFY_EXT(RuntimeError, CONDITION).
#define LLGL_VERIFY(CONDITION) \
    LLGL_VERIFY_EXT(RuntimeError, CONDITION)


namespace LLGL
{


/**
\brief Enumeration of all exception classes Trap() can throw.
\remarks LLGL only throws exceptions if it was built with \c LLGL_ENABLE_EXCEPTIONS.
\see Trap
*/
enum class Exception
{
    //! Refers to std::runtime_error.
    RuntimeError,

    //! Refers to std::out_of_range.
    OutOfRange,

    //! Refers to std::bad_cast.
    BadCast,

    //! Refers to std::invalid_argument.
    InvalidArgument,
};


/**
\brief Primary function to trap execution from an unrecoverable state.
\param[in] exception Specifies what type of exception this function should throw if exceptions are enabled.
\param[in] origin Specifies the origin where execution is trapped. This can be the special preprocessor macro \c __FUNCTION__ for instance.
\param[in] format Specifies the formatted STL::string as used with \c printf.
\remarks This might either throw an exception, abort execution, or break the debugger depending on the configuration LLGL was built with.
LLGL does not handle exceptions of any kind but can throw exceptions (if built with \c LLGL_ENABLE_EXCEPTIONS)
to let the client programmer exit the application gracefully.
Otherwise, this function simply aborts execution and dumps the callstack to the standard error pipe.
*/
[[noreturn]]
LLGL_EXPORT void Trap(Exception exception, const char* origin, const char* format, ...);


} // /namespace LLGL


#endif



// ================================================================================
