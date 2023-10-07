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
#include "../CheckedCast.h"
#include "../../Core/MacroUtils.h"
#include "../../Core/PrintfUtils.h"
#include <type_traits>


namespace LLGL
{


#define LLGL_DBG_SOURCE()                       \
    {                                           \
        if (debugger_ != nullptr)               \
            debugger_->SetSource(__FUNCTION__); \
    }

#define LLGL_DBG_ERROR(TYPE, FORMAT, ...)                                   \
    do                                                                      \
    {                                                                       \
        if (debugger_ != nullptr)                                           \
            debugger_->Errorf((TYPE), (FORMAT) LLGL_VA_ARGS(__VA_ARGS__));  \
    }                                                                       \
    while (false)

#define LLGL_DBG_WARN(TYPE, FORMAT, ...)                                        \
    do                                                                          \
    {                                                                           \
        if (debugger_ != nullptr)                                               \
            debugger_->Warningf((TYPE), (FORMAT) LLGL_VA_ARGS(__VA_ARGS__));    \
    }                                                                           \
    while (false)

#define LLGL_DBG_ERROR_NOT_SUPPORTED(FEATURE) \
    LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, UTF8String(FEATURE) + " not supported")


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

// Returns the debug wrapper of the specified instance or null if the input is null.
template <typename TDbgWrapper, typename TInstance>
TDbgWrapper* DbgGetWrapper(TInstance* obj)
{
    if (obj != nullptr)
        return LLGL_CAST(TDbgWrapper*, obj);
    else
        return nullptr;
}

// Returns the constant debug wrapper of the specified instance or null if the input is null.
template <typename TDbgWrapper, typename TInstance>
const TDbgWrapper* DbgGetWrapper(const TInstance* obj)
{
    if (obj != nullptr)
        return LLGL_CAST(const TDbgWrapper*, obj);
    else
        return nullptr;
}

// Returns the instance the specified debug object wraps or null if the input is null.
template <typename TDbgWrapper, typename TInstance>
TInstance* DbgGetInstance(TInstance* obj)
{
    if (obj != nullptr)
    {
        TDbgWrapper* objDbg = LLGL_CAST(TDbgWrapper*, obj);
        return &(objDbg->instance);
    }
    return nullptr;
}

// Returns the constant instance the specified debug object wraps or null if the input is null.
template <typename TDbgWrapper, typename TInstance>
const TInstance* DbgGetInstance(const TInstance* obj)
{
    if (obj != nullptr)
    {
        const TDbgWrapper* objDbg = LLGL_CAST(const TDbgWrapper*, obj);
        return &(objDbg->instance);
    }
    return nullptr;
}


} // /namespace LLGL


#endif



// ================================================================================
