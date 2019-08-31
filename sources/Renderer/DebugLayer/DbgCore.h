/*
 * DbgCore.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_CORE_H
#define LLGL_DBG_CORE_H


#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>


namespace LLGL
{


#define LLGL_DBG_SOURCE \
    DbgSetSource(debugger_, __FUNCTION__)

#define LLGL_DBG_ERROR(TYPE, MESSAGE) \
    DbgPostError(debugger_, (TYPE), (MESSAGE))

#define LLGL_DBG_WARN(TYPE, MESSAGE) \
    DbgPostWarning(debugger_, (TYPE), (MESSAGE))

#define LLGL_DBG_ERROR_NOT_SUPPORTED(FEATURE) \
    LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, std::string(FEATURE) + " not supported")


inline void DbgSetSource(RenderingDebugger* debugger, const char* source)
{
    if (debugger)
        debugger->SetSource(source);
}

inline void DbgPostError(RenderingDebugger* debugger, ErrorType type, const std::string& message)
{
    if (debugger)
        debugger->PostError(type, message);
}

inline void DbgPostWarning(RenderingDebugger* debugger, WarningType type, const std::string& message)
{
    if (debugger)
        debugger->PostWarning(type, message);
}

// Sets the name of the specified debug layer object.
template <typename T>
inline void DbgSetObjectName(T& obj, const char* name)
{
    /* Set or clear label */
    if (name != nullptr)
        obj.label = name;
    else
        obj.label.clear();

    /* Forward call to instance */
    obj.instance.SetName(name);
}


} // /namespace LLGL


#endif



// ================================================================================
