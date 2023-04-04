/*
 * StringUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STRING_UTILS_H
#define LLGL_STRING_UTILS_H


#include <LLGL/Export.h>
#include <string>
#include <vector>


namespace LLGL
{


/* ----- Templates ----- */

// Returns the specified integral value as hexadecimal string.
template <typename T>
std::string IntToHex(T value, const char* prefix = "0x")
{
    static_assert(std::is_integral<T>::value, "IntToHex<T>: template parameter 'T' must be an integral type");

    std::string hex;
    hex.resize(sizeof(T)*2);

    constexpr char alphabet[] = "0123456789ABCDEF";
    for (auto pos = hex.size(); pos > 0; value /= 0xF)
        hex[--pos] = alphabet[value & 0xF];

    if (prefix != nullptr && *prefix != '\0')
        hex = prefix + hex;

    return hex;
}

// Returns the length of the specified null-terminated string.
template <typename T>
inline std::size_t StrLength(const T* s)
{
    std::size_t len = 0;
    while (*s++ != 0)
        ++len;
    return len;
}


/* ----- Functions ----- */

// Reads the specified text file into a string.
LLGL_EXPORT std::string ReadFileString(const char* filename);

// Reads the specified binary file into a buffer.
LLGL_EXPORT std::vector<char> ReadFileBuffer(const char* filename);

// Converts the UTF16 input string to UTF8 string.
LLGL_EXPORT std::string ToUTF8String(const std::wstring& utf16);
LLGL_EXPORT std::string ToUTF8String(const wchar_t* utf16);

// Converts the UTF8 input string to UTF16 string.
LLGL_EXPORT std::wstring ToUTF16String(const std::string& utf8);
LLGL_EXPORT std::wstring ToUTF16String(const char* utf8);


} // /namespace LLGL


#endif



// ================================================================================
