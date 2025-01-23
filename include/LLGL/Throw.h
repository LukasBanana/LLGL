#ifndef LLGL_THROW_H
#define LLGL_THROW_H

#if defined(LLGL_DEBUG) || defined(LLGL_ENABLE_EXCEPTIONS)
    #define LLGL_THROW(exception) throw (exception)
    #define LLGL_THROW_IF(condition, exception) if (condition) { LLGL_THROW(exception); }
#else
    #define LLGL_THROW(exception) std::abort()
    #define LLGL_THROW_IF(condition, exception)
#endif

#endif