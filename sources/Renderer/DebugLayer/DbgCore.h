/*
 * DbgCore.h
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_CORE_H
#define LLGL_DBG_CORE_H


#include <LLGL/RenderingProfiler.h>
#include <LLGL/RenderingDebugger.h>
#include <LLGL/Container/Strings.h>


namespace LLGL
{


#define LLGL_DBG_SOURCE \
    DbgSetSource(debugger_, __FUNCTION__)

#define LLGL_DBG_ERROR(TYPE, MESSAGE) \
    DbgPostError(debugger_, (TYPE), (MESSAGE))

#define LLGL_DBG_WARN(TYPE, MESSAGE) \
    DbgPostWarning(debugger_, (TYPE), (MESSAGE))

#define LLGL_DBG_ERROR_NOT_SUPPORTED(FEATURE) \
    LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, UTF8String(FEATURE) + " not supported")


inline void DbgSetSource(RenderingDebugger* debugger, const char* source)
{
    if (debugger)
        debugger->SetSource(source);
}

inline void DbgPostError(RenderingDebugger* debugger, ErrorType type, const StringView& message)
{
    if (debugger)
        debugger->PostError(type, message);
}

inline void DbgPostWarning(RenderingDebugger* debugger, WarningType type, const StringView& message)
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
