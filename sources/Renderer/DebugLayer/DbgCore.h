/*
 * DbgCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_CORE_H__
#define __LLGL_DBG_CORE_H__


#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


#define LLGL_DBG_PROFILER_DO(EXPR)  \
    if (profiler_)                  \
        profiler_->EXPR

#define LLGL_DBG_DEBUGGER_DO(STMNT) \
    if (debugger_)                  \
        STMNT

#define LLGL_DBG_ERROR(TYPE, MESSAGE, SOURCE) \
    DbgPostError(debugger_, (TYPE), (MESSAGE), (SOURCE))

#define LLGL_DBG_ERROR_HERE(TYPE, MESSAGE) \
    LLGL_DBG_ERROR((TYPE), (MESSAGE), __FUNCTION__)

#define LLGL_DBG_WARN(TYPE, MESSAGE, SOURCE) \
    DbgPostWarning(debugger_, (TYPE), (MESSAGE), (SOURCE))

#define LLGL_DBG_WARN_HERE(TYPE, MESSAGE) \
    LLGL_DBG_WARN((TYPE), (MESSAGE), __FUNCTION__)

#define LLGL_DBG_ERROR_NOT_SUPPORTED(FEATURE, SOURCE) \
    LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, std::string(FEATURE) + " is not supported", (SOURCE))


inline void DbgPostError(RenderingDebugger* debugger, ErrorType type, const std::string& message, const std::string& source)
{
    if (debugger)
        debugger->PostError(type, message, source);
}

inline void DbgPostWarning(RenderingDebugger* debugger, WarningType type, const std::string& message, const std::string& source)
{
    if (debugger)
        debugger->PostWarning(type, message, source);
}


} // /namespace LLGL


#endif



// ================================================================================
