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


namespace LLGL
{


//! Concatenates the two UTF-8 string.
inline UTF8String operator + (const UTF8String& lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const UTF8String& lhs, const StringView& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const UTF8String& lhs, const WStringView& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const UTF8String& lhs, const char* rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const UTF8String& lhs, const wchar_t* rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const StringView& lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const WStringView& lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const char* lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

inline UTF8String operator + (const wchar_t* lhs, const UTF8String& rhs)
{
    UTF8String result = lhs;
    result += rhs;
    return result;
}

//! Returns true if the two strings are equal.
inline bool operator == (const UTF8String& lhs, const UTF8String& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const UTF8String& lhs, const StringView& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const UTF8String& lhs, const WStringView& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const UTF8String& lhs, const char* rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const UTF8String& lhs, const wchar_t* rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const StringView& lhs, const StringView& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const StringView& lhs, const UTF8String& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const StringView& lhs, const char* rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const WStringView& lhs, const WStringView& rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const WStringView& lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) == 0);
}

inline bool operator == (const WStringView& lhs, const wchar_t* rhs)
{
    return (lhs.compare(rhs) == 0);
}

inline bool operator == (const char* lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) == 0);
}

inline bool operator == (const char* lhs, const StringView& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) == 0);
}

inline bool operator == (const wchar_t* lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) == 0);
}

inline bool operator == (const wchar_t* lhs, const WStringView& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) == 0);
}

//! Returns true if the two strings are not equal.
inline bool operator != (const UTF8String& lhs, const UTF8String& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const UTF8String& lhs, const StringView& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const UTF8String& lhs, const WStringView& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const UTF8String& lhs, const char* rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const UTF8String& lhs, const wchar_t* rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const StringView& lhs, const StringView& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const StringView& lhs, const UTF8String& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const StringView& lhs, const char* rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const WStringView& lhs, const WStringView& rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const WStringView& lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) != 0);
}

inline bool operator != (const WStringView& lhs, const wchar_t* rhs)
{
    return (lhs.compare(rhs) != 0);
}

inline bool operator != (const char* lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) != 0);
}

inline bool operator != (const char* lhs, const StringView& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) != 0);
}

inline bool operator != (const wchar_t* lhs, const UTF8String& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) != 0);
}

inline bool operator != (const wchar_t* lhs, const WStringView& rhs)
{
    /* Flip compare operators */
    return (rhs.compare(lhs) != 0);
}


} // /namespace LLGL


#endif



// ================================================================================
