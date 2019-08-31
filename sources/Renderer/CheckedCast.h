/*
 * CheckedCast.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CHECKED_CAST_H
#define LLGL_CHECKED_CAST_H


#ifdef LLGL_ENABLE_CHECKED_CAST
#   include <typeinfo>
#endif


namespace LLGL
{


#ifdef LLGL_ENABLE_CHECKED_CAST

template <typename DstT, typename SrcT>
DstT& CheckedCast(SrcT& obj)
{
    try
    {
        return dynamic_cast<DstT&>(obj);
    }
    catch (const std::bad_cast&)
    {
        #if defined _WIN32 && defined _DEBUG
        __debugbreak();
        #endif
        throw;
    }
}

template <typename DstT, typename SrcT>
DstT CheckedCast(SrcT* obj)
{
    try
    {
        DstT casted = dynamic_cast<DstT>(obj);
        if (!casted)
            throw std::bad_cast();
        return casted;
    }
    catch (const std::bad_cast&)
    {
        #if defined _WIN32 && defined _DEBUG
        __debugbreak();
        #endif
        throw;
    }
}

#define LLGL_CAST(TYPE, OBJ) CheckedCast<TYPE>(OBJ)

#else

#define LLGL_CAST(TYPE, OBJ) static_cast<TYPE>(OBJ)

#endif


} // /namespace LLGL


#endif



// ================================================================================
