/*
 * StringUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "StringUtils.h"
#include <fstream>
#include <codecvt>
#include <locale>
#include <stdio.h>


namespace LLGL
{


LLGL_EXPORT std::string ReadFileString(const char* filename)
{
    // Read file content into string
    std::ifstream file{ filename };
    if (file.good())
    {
        return std::string
        {
            ( std::istreambuf_iterator<char>(file) ),
            ( std::istreambuf_iterator<char>() )
        };
    }
    return "";
}

LLGL_EXPORT std::vector<char> ReadFileBuffer(const char* filename)
{
    // Read file content into buffer
    std::ifstream file{ filename, (std::ios_base::binary | std::ios_base::ate) };

    if (!file.good())
        throw std::runtime_error("failed to open file: " + std::string(filename));

    auto fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}

LLGL_EXPORT std::string ToUTF8String(const std::wstring& utf16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(utf16);
}

LLGL_EXPORT std::string ToUTF8String(const wchar_t* utf16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(utf16);
}

LLGL_EXPORT std::wstring ToUTF16String(const std::string& utf8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(utf8);
}

LLGL_EXPORT std::wstring ToUTF16String(const char* utf8)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(utf8);
}

void StringPrintf(std::string& str, const char* format, va_list args1, va_list args2)
{
    const int len = ::vsnprintf(nullptr, 0, format, args1);
    if (len > 0)
    {
        /*
        Since C++11 we can override the last character with '\0' ourselves,
        so it's safe to let ::vsnprintf override std::string from [0, size()] inclusive.
        */
        const std::size_t formatLen = static_cast<std::size_t>(len);
        const std::size_t appendOff = str.size();
        str.resize(appendOff + formatLen);
        ::vsnprintf(&str[appendOff], formatLen + 1, format, args2);
    }
}


} // /namespace LLGL



// ================================================================================
