/*
 * CheckedCast.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CHECKED_CAST_H
#define LLGL_CHECKED_CAST_H


#include "../Platform/Debug.h"

#if LLGL_ENABLE_CHECKED_CAST
#   if LLGL_ENABLE_EXCEPTIONS
#       include <typeinfo>
#   else
#       include <LLGL/TypeInfo.h>
#       include "../Core/Assertion.h"
#   endif
#endif


namespace LLGL
{


#if LLGL_ENABLE_CHECKED_CAST

template <typename TDst, typename TSrc>
inline TDst& ObjectCast(TSrc& obj)
{
    #if LLGL_ENABLE_EXCEPTIONS

    try
    {
        return dynamic_cast<TDst&>(obj);
    }
    catch (const std::bad_cast&)
    {
        LLGL_DEBUG_BREAK();
        throw;
    }

    #else // LLGL_ENABLE_EXCEPTIONS

    return dynamic_cast<TDst&>(obj);

    #endif // /LLGL_ENABLE_EXCEPTIONS
}

template <typename TDst, typename TSrc>
inline TDst ObjectCast(TSrc* obj)
{
    if (obj == nullptr)
        return nullptr;

    #if LLGL_ENABLE_EXCEPTIONS

    try
    {
        TDst objInstance = dynamic_cast<TDst>(obj);
        if (!objInstance)
            throw std::bad_cast();
        return objInstance;
    }
    catch (const std::bad_cast&)
    {
        LLGL_DEBUG_BREAK();
        throw;
    }

    #else // LLGL_ENABLE_EXCEPTIONS

    TDst objInstance = dynamic_cast<TDst>(obj);
    LLGL_ASSERT(objInstance != nullptr);
    return objInstance;

    #endif // /LLGL_ENABLE_EXCEPTIONS
}

#else // LLGL_ENABLE_CHECKED_CAST

template <typename TDst, typename TSrc>
inline TDst ObjectCast(TSrc&& obj)
{
    return static_cast<TDst>(obj);
}

#endif // /LLGL_ENABLE_CHECKED_CAST

#define LLGL_CAST(TYPE, OBJ) \
    ObjectCast<TYPE>(OBJ)


} // /namespace LLGL


#endif



// ================================================================================
