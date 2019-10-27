/*
 * Helper.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <fstream>
#include <codecvt>


namespace LLGL
{


LLGL_EXPORT std::string ReadFileString(const char* filename)
{
    // Read file content into string
    std::ifstream file { filename };

    if (!file.good())
        throw std::runtime_error("failed to open file: " + std::string(filename));

    return std::string
    {
        ( std::istreambuf_iterator<char>(file) ),
        ( std::istreambuf_iterator<char>() )
    };
}

LLGL_EXPORT std::vector<char> ReadFileBuffer(const char* filename)
{
    // Read file content into buffer
    std::ifstream file { filename, (std::ios_base::binary | std::ios_base::ate) };

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


} // /namespace LLGL



// ================================================================================
