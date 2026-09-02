/*
 * CiStringView.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CI_STRING_VIEW_H
#define LLGL_CI_STRING_VIEW_H


#include <LLGL/Container/StringView.h>
#include <string>
#include <cctype>


namespace LLGL
{


template <typename TChar>
struct CiCharTraits : public std::char_traits<TChar>
{
    static TChar to_upper(TChar ch)
    {
        auto chUnsigned = static_cast<typename std::make_unsigned<TChar>::type>(ch);
        return static_cast<TChar>(std::toupper(static_cast<int>(chUnsigned)));
    }
    
    static bool eq(TChar c1, TChar c2)
    {
        return to_upper(c1) == to_upper(c2);
    }
    
    static bool lt(TChar c1, TChar c2)
    {
         return to_upper(c1) < to_upper(c2);
    }
    
    static int compare(const TChar* s1, const TChar* s2, std::size_t n)
    {
        while (n-- != 0)
        {
            if (to_upper(*s1) < to_upper(*s2))
                return -1;
            if (to_upper(*s1) > to_upper(*s2))
                return 1;
            ++s1;
            ++s2;
        }
        return 0;
    }
    
    static const TChar* find(const TChar* s, std::size_t n, TChar a)
    {
        const TChar ua = to_upper(a);
        while (n-- != 0) 
        {
            if (to_upper(*s) == ua)
                return s;
            ++s;
        }
        return nullptr;
    }
};

template <typename TChar>
using BasicCiStringView = BasicStringView<TChar, CiCharTraits<TChar>>;

using CiStringView = BasicCiStringView<char>;
using CiWStringView = BasicCiStringView<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
