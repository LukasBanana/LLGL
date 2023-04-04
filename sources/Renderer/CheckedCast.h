/*
 * CheckedCast.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CHECKED_CAST_H
#define LLGL_CHECKED_CAST_H


#include "../Platform/Debug.h"

#ifdef LLGL_ENABLE_CHECKED_CAST
#   include <typeinfo>
#endif


namespace LLGL
{


#ifdef LLGL_ENABLE_CHECKED_CAST

template <typename TDst, typename TSrc>
inline TDst& ObjectCast(TSrc& obj)
{
    try
    {
        return dynamic_cast<TDst&>(obj);
    }
    catch (const std::bad_cast&)
    {
        LLGL_DEBUG_BREAK();
        throw;
    }
}

template <typename TDst, typename TSrc>
inline TDst ObjectCast(TSrc* obj)
{
    if (obj == nullptr)
        return nullptr;
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
}

#else

template <typename TDst, typename TSrc>
inline TDst ObjectCast(TSrc&& obj)
{
    return static_cast<TDst>(obj);
}

#endif

#define LLGL_CAST(TYPE, OBJ) ObjectCast<TYPE>(OBJ)


} // /namespace LLGL


#endif



// ================================================================================
