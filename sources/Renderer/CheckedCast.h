/*
 * CheckedCast.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_CHECKED_CAST_H__
#define __LLGL_CHECKED_CAST_H__


#ifdef LLGL_DEBUG
#include <typeinfo>
#endif


namespace LLGL
{


#ifdef LLGL_DEBUG

template <typename To, typename From>
To CheckedCast(From obj)
{
    return dynamic_cast<To>(obj);
}

template <typename To, typename From>
To CheckedCast(From* obj)
{
    To casted = dynamic_cast<To>(obj);
    if (!casted)
        throw std::bad_cast();
    return casted;
}

#define LLGL_CAST(TYPE, OBJ) CheckedCast<TYPE>(OBJ)

#else

#define LLGL_CAST(TYPE, OBJ) static_cast<TYPE>(OBJ)

#endif


} // /namespace LLGL


#endif



// ================================================================================
