/*
 * UTF8String.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Container/UTF8String.h>
#include <algorithm>
#include <iterator>
#include "Exception.h"
#include "Assertion.h"


namespace LLGL
{


static std::size_t GetUTF16CharCount(const StringView& s)
{
    return s.size();
}

static SmallVector<wchar_t> ConvertToUTF16WCharArray(const StringView& s)
{
    /* Allocate buffer for UTF-16 string */
    const std::size_t len = GetUTF16CharCount(s);

    SmallVector<wchar_t> utf16;
    utf16.reserve(len + 1);

    /* Encode UTF-16 string */
    for (auto it = s.begin(); it != s.end();)
    {
        int c = static_cast<unsigned char>(*it++);

        /* Check for bit pattern 0xxxxxxx */
        if ((c & 0x80) == 0x00)
        {
            /* Read one byte */
            utf16.push_back(c);
        }
        /* Check for bit pattern 110xxxxx */
        else if ((c & 0xE0) == 0xC0)
        {
            /* Read two bytes */
            wchar_t w0 = static_cast<wchar_t>(c & 0x1F); c = *it++;
            wchar_t w1 = static_cast<wchar_t>(c & 0x3F);
            utf16.push_back(w0 << 5 | w1);
        }
        /* Check for bit pattern 1110xxxx */
        else if ((c & 0xF0) == 0xE0)
        {
            /* Read three bytes */
            wchar_t w0 = static_cast<wchar_t>(c & 0x0F); c = *it++;
            wchar_t w1 = static_cast<wchar_t>(c & 0x3F); c = *it++;
            wchar_t w2 = static_cast<wchar_t>(c & 0x3F);
            utf16.push_back(w0 << 12 | w1 << 6 | w2);
        }
        else
            LLGL_TRAP("UTF8 character bigger than two bytes");
    }

    utf16.push_back(L'\0');

    return utf16;
}

static std::size_t GetUTF8CharCount(int c)
{
    if (c < 0x0080)
    {
        /* U+0000 ... U+007F */
        return 1;
    }
    else if (c < 0x07FF)
    {
        /* U+0080 ... U+07FF */
        return 2;
    }
    else if (c < 0xFFFF)
    {
        /* U+0800 ... U+FFFF */
        return 3;
    }
    else
    {
        /* U+10000 ... U+10FFFF */
        return 4;
    }
}

static std::size_t GetUTF8CharCount(const WStringView& s)
{
    std::size_t len = 0;

    for (int c : s)
        len += GetUTF8CharCount(c);

    return len;
}

// Appends a unicode character encoded in UTF-8 to the specified string buffer and returns a pointer to the next character in that buffer.
// see https://en.wikipedia.org/wiki/UTF-8
static void AppendUTF8Character(SmallVector<char>& str, int code)
{
    if (code < 0x0080)
    {
        /* U+0000 ... U+007F */
        str.push_back(static_cast<char>(code));                         // 0ccccccc
    }
    else if (code < 0x07FF)
    {
        /* U+0080 ... U+07FF */
        str.reserve(str.size() + 2);
        str.push_back(static_cast<char>(0xC0 | ((code >>  6) & 0x1F))); // 110ccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
    else if (code < 0xFFFF)
    {
        /* U+0800 ... U+FFFF */
        str.reserve(str.size() + 3);
        str.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F))); // 1110cccc
        str.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
    else
    {
        /* U+10000 ... U+10FFFF */
        str.reserve(str.size() + 4);
        str.push_back(static_cast<char>(0xF0 | ((code >> 18) & 0x07))); // 11110ccc
        str.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
}

LLGL_EXPORT SmallVector<char> ConvertWStringViewToUTF8CharArray(const WStringView& s)
{
    /* Allocate buffer for UTF-16 string */
    const auto len = GetUTF8CharCount(s);

    SmallVector<char> utf8;
    utf8.reserve(len + 1);

    /* Encode UTF-8 string */
    for (int c : s)
        AppendUTF8Character(utf8, c);

    utf8.push_back('\0');

    return utf8;
}

static SmallVector<char> ConvertStringViewToCharArray(const StringView& str)
{
    SmallVector<char> data;
    data.reserve(str.size() + 1);
    data.insert(data.end(), str.begin(), str.end());
    data.push_back('\0');
    return data;
}


/*
 * UTF8String class
 */

UTF8String::UTF8String() :
    data_ { '\0' }
{
}

UTF8String::UTF8String(const UTF8String& rhs) :
    data_ { rhs.data_ }
{
}

UTF8String::UTF8String(UTF8String&& rhs) :
    data_ { std::move(rhs.data_) }
{
    rhs.clear();
}

UTF8String::UTF8String(const StringView& str) :
    data_ { ConvertStringViewToCharArray(str) }
{
}

UTF8String::UTF8String(const WStringView& str) :
    data_ { ConvertWStringViewToUTF8CharArray(str) }
{
}

UTF8String::UTF8String(const char* str) :
    UTF8String { StringView{ str } }
{
}

UTF8String::UTF8String(const wchar_t* str) :
    UTF8String { WStringView{ str } }
{
}

UTF8String& UTF8String::operator = (const UTF8String& rhs)
{
    data_ = rhs.data_;
    return *this;
}

UTF8String& UTF8String::operator = (UTF8String&& rhs)
{
    data_ = std::move(rhs.data_);
    rhs.clear();
    return *this;
}

UTF8String& UTF8String::operator += (const UTF8String& rhs)
{
    return append(rhs.begin(), rhs.end());
}

UTF8String& UTF8String::operator += (const StringView& rhs)
{
    return append(rhs.begin(), rhs.end());
}

UTF8String& UTF8String::operator += (const WStringView& rhs)
{
    auto utf8String = ConvertWStringViewToUTF8CharArray(rhs);
    return append(utf8String.begin(), utf8String.end());
}

UTF8String& UTF8String::operator += (const char* rhs)
{
    const StringView rhsView{ rhs };
    return append(rhsView.begin(), rhsView.end());
}

UTF8String& UTF8String::operator += (const wchar_t* rhs)
{
    auto utf8String = ConvertWStringViewToUTF8CharArray(rhs);
    return append(utf8String.begin(), utf8String.end());
}

UTF8String& UTF8String::operator += (char chr)
{
    return append(1u, chr);
}

UTF8String& UTF8String::operator += (wchar_t chr)
{
    if (GetUTF8CharCount(chr) > 1)
    {
        wchar_t str[] = { chr, L'\0' };
        auto utf8String = ConvertWStringViewToUTF8CharArray(str);
        return append(utf8String.begin(), utf8String.end());
    }
    return append(1u, static_cast<char>(chr));
}

void UTF8String::clear()
{
    data_ = { '\0' };
}

int UTF8String::compare(const StringView& str) const
{
    return StringView{ data(), size() }.compare(str);
}

int UTF8String::compare(size_type pos1, size_type count1, const StringView& str) const
{
    return StringView{ data(), size() }.compare(pos1, count1, str);
}

int UTF8String::compare(size_type pos1, size_type count1, const StringView& str, size_type pos2, size_type count2) const
{
    return StringView{ data(), size() }.compare(pos1, count1, str, pos2, count2);
}

int UTF8String::compare(const WStringView& str) const
{
    auto utf8String = ConvertWStringViewToUTF8CharArray(str);
    return compare(StringView{ utf8String.data(), utf8String.size() });
}

int UTF8String::compare(size_type pos1, size_type count1, const WStringView& str) const
{
    auto utf8String = ConvertWStringViewToUTF8CharArray(str);
    return compare(pos1, count1, StringView{ utf8String.data(), utf8String.size() });
}

int UTF8String::compare(size_type pos1, size_type count1, const WStringView& str, size_type pos2, size_type count2) const
{
    auto utf8String = ConvertWStringViewToUTF8CharArray(str.substr(pos2, count2));
    return compare(pos1, count1, StringView{ utf8String.data(), utf8String.size() });
}

UTF8String UTF8String::substr(size_type pos, size_type count) const
{
    if (pos > size())
        LLGL_TRAP("start position for UTF8 string out of range");
    count = std::min(count, size() - pos);
    return StringView{ &(data_[pos]), count };
}

void UTF8String::resize(size_type size, char ch)
{
    if (size != this->size())
    {
        /* Remove NUL-terminator temporarily to avoid unnecessary reallocations and copy operations of the internal container */
        data_.pop_back();
        data_.reserve(size + 1);
        data_.resize(size, ch);
        data_.push_back('\0');
    }
}

UTF8String& UTF8String::append(size_type count, char ch)
{
    resize(size() + count, ch);
    return *this;
}

UTF8String& UTF8String::append(const char* first, const char* last)
{
    /* Remove NUL-terminator temporarily to avoid unnecessary reallocations and copy operations of the internal container */
    const difference_type dist = std::distance(first, last);
    if (dist > 0)
    {
        data_.reserve(data_.size() + static_cast<size_type>(dist));
        data_.pop_back();
        data_.insert(data_.end(), first, last);
        data_.push_back('\0');
    }
    return *this;
}

SmallVector<wchar_t> UTF8String::to_utf16() const
{
    return ConvertToUTF16WCharArray(StringView{ c_str(), size() });
}


} // /namespace LLGL



// ================================================================================
