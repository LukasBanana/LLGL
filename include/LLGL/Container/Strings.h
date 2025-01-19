/*
 * Strings.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STRINGS_H
#define LLGL_STRINGS_H


#include <LLGL/Container/UTF8String.h>
#include <LLGL/Container/StringView.h>
#include <LLGL/Container/StringLiteral.h>


namespace LLGL
{


//! Concatenates the two UTF-8 string.
inline UTF8String operator + (const UTF8String& lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

template <typename TChar, typename Traits>
inline UTF8String operator + (const UTF8String& lhs, const BasicStringView<TChar, Traits>& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

template <typename TChar>
inline UTF8String operator + (const UTF8String& lhs, const TChar* rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

template <typename TChar, typename Traits>
inline UTF8String operator + (const BasicStringView<TChar, Traits>& lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

template <typename TChar>
inline UTF8String operator + (const TChar* lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

#define LLGL_DEFINE_STRING_COMPARE_OPERATORS(OP, OP_REV)            \
    inline bool operator OP (                                       \
        const UTF8String& lhs,                                      \
        const UTF8String& rhs)                                      \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits>                      \
    inline bool operator OP (                                       \
        const UTF8String&                       lhs,                \
        const BasicStringView<TChar, Traits>&   rhs)                \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar>                                       \
    inline bool operator OP (                                       \
        const UTF8String&   lhs,                                    \
        const TChar*        rhs)                                    \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits>                      \
    inline bool operator OP (                                       \
        const BasicStringView<TChar, Traits>& lhs,                  \
        const BasicStringView<TChar, Traits>& rhs)                  \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits>                      \
    inline bool operator OP (                                       \
        const BasicStringView<TChar, Traits>&   lhs,                \
        const UTF8String&                       rhs)                \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits>                      \
    inline bool operator OP (                                       \
        const BasicStringView<TChar, Traits>&   lhs,                \
        const TChar*                            rhs)                \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar>                                       \
    inline bool operator OP (                                       \
        const TChar*        lhs,                                    \
        const UTF8String&   rhs)                                    \
    {                                                               \
        /* Flip compare operands */                                 \
        return (rhs.compare(lhs) OP_REV 0);                         \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits>                      \
    inline bool operator OP (                                       \
        const TChar*                            lhs,                \
        const BasicStringView<TChar, Traits>&   rhs)                \
    {                                                               \
        /* Flip compare operands */                                 \
        return (rhs.compare(lhs) OP_REV 0);                         \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits, typename Allocator>  \
    inline bool operator OP (                                       \
        const BasicStringLiteral<TChar, Traits, Allocator>& lhs,    \
        const BasicStringLiteral<TChar, Traits, Allocator>& rhs)    \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits, typename Allocator>  \
    inline bool operator OP (                                       \
        const BasicStringLiteral<TChar, Traits, Allocator>& lhs,    \
        const TChar*                                        rhs)    \
    {                                                               \
        return (lhs.compare(rhs) OP 0);                             \
    }                                                               \
                                                                    \
    template <typename TChar, typename Traits, typename Allocator>  \
    inline bool operator OP (                                       \
        const TChar*                                        lhs,    \
        const BasicStringLiteral<TChar, Traits, Allocator>& rhs)    \
    {                                                               \
        /* Flip compare operands */                                 \
        return (rhs.compare(lhs) OP_REV 0);                         \
    }

LLGL_DEFINE_STRING_COMPARE_OPERATORS( ==, == );
LLGL_DEFINE_STRING_COMPARE_OPERATORS( !=, != );
LLGL_DEFINE_STRING_COMPARE_OPERATORS( < , >  );
LLGL_DEFINE_STRING_COMPARE_OPERATORS( <=, >= );
LLGL_DEFINE_STRING_COMPARE_OPERATORS( > , <  );
LLGL_DEFINE_STRING_COMPARE_OPERATORS( >=, <= );

#undef LLGL_DEFINE_STRING_COMPARE_OPERATORS


} // /namespace LLGL


#endif



// ================================================================================
