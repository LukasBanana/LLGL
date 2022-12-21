/*
 * UTF8String.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Container/UTF8String.h>


namespace LLGL
{


static std::size_t GetUTF16CharCount(const StringView& s)
{
    return s.size();
}

static SmallVector<wchar_t> ConvertToUTF16WCharArray(const StringView& s)
{
    /* Allocate buffer for UTF-16 string */
    const auto len = GetUTF16CharCount(s);

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
            auto w0 = static_cast<wchar_t>(c & 0x1F); c = *it++;
            auto w1 = static_cast<wchar_t>(c & 0x3F);
            utf16.push_back(w0 << 5 | w1);
        }
        /* Check for bit pattern 1110xxxx */
        else if ((c & 0xF0) == 0xE0)
        {
            /* Read three bytes */
            auto w0 = static_cast<wchar_t>(c & 0x0F); c = *it++;
            auto w1 = static_cast<wchar_t>(c & 0x3F); c = *it++;
            auto w2 = static_cast<wchar_t>(c & 0x3F);
            utf16.push_back(w0 << 12 | w1 << 6 | w2);
        }
        else
            throw std::runtime_error("UTF8 character bigger than two bytes");
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
        str.push_back(static_cast<char>(0xC0 | ((code >>  6) & 0x1F))); // 110ccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
    else if (code < 0xFFFF)
    {
        /* U+0800 ... U+FFFF */
        str.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F))); // 1110cccc
        str.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
    else
    {
        /* U+10000 ... U+10FFFF */
        str.push_back(static_cast<char>(0xF0 | ((code >> 18) & 0x07))); // 11110ccc
        str.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ((code >>  6) & 0x3F))); // 10cccccc
        str.push_back(static_cast<char>(0x80 | ( code        & 0x3F))); // 10cccccc
    }
}

LLGL_EXPORT SmallVector<char> ConvertToUTF8CharArray(const WStringView& s)
{
    /* Allocate buffer for UTF-16 string */
    const auto len = GetUTF8CharCount(s);

    SmallVector<char> utf8;
    utf8.reserve(len + 1);

    /* Encode UTF-8 string */
    auto utf8ptr = &utf8[0];
    for (int c : s)
        AppendUTF8Character(utf8, c);

    utf8.push_back('\0');

    return utf8;
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
}

static SmallVector<char> ConvertStringViewToCharArray(const StringView& str)
{
    SmallVector<char> data;
    data.reserve(str.size() + 1);
    data.insert(data.end(), str.begin(), str.end());
    data.push_back('\0');
    return data;
}

UTF8String::UTF8String(const StringView& str) :
    data_ { ConvertStringViewToCharArray(str) }
{
}

UTF8String::UTF8String(const WStringView& str) :
    data_ { ConvertToUTF8CharArray(str) }
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
    return *this;
}

UTF8String& UTF8String::operator += (const UTF8String& rhs)
{
    data_.insert(end(), rhs.begin(), rhs.end());
    return *this;
}

UTF8String& UTF8String::operator += (const StringView& rhs)
{
    data_.insert(end(), rhs.begin(), rhs.end());
    return *this;
}

UTF8String& UTF8String::operator += (const WStringView& rhs)
{
    auto utf8String = ConvertToUTF8CharArray(rhs);
    data_.insert(end(), utf8String.begin(), utf8String.end());
    return *this;
}

UTF8String& UTF8String::operator += (const char* rhs)
{
    const StringView rhsView{ rhs };
    data_.insert(end(), rhsView.begin(), rhsView.end());
    return *this;
}

UTF8String& UTF8String::operator += (const wchar_t* rhs)
{
    auto utf8String = ConvertToUTF8CharArray(rhs);
    data_.insert(end(), utf8String.begin(), utf8String.end());
    return *this;
}

UTF8String& UTF8String::operator += (char chr)
{
    data_.push_back(chr);
    return *this;
}

UTF8String& UTF8String::operator += (wchar_t chr)
{
    if (GetUTF8CharCount(chr) > 1)
    {
        wchar_t str[] = { chr, L'\0' };
        auto utf8String = ConvertToUTF8CharArray(str);
        data_.insert(end(), utf8String.begin(), utf8String.end());
    }
    else
        data_.push_back(static_cast<char>(chr));
    return *this;
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
    auto utf8String = ConvertToUTF8CharArray(str);
    return compare(StringView{ utf8String.data(), utf8String.size() });
}

int UTF8String::compare(size_type pos1, size_type count1, const WStringView& str) const
{
    auto utf8String = ConvertToUTF8CharArray(str);
    return compare(pos1, count1, StringView{ utf8String.data(), utf8String.size() });
}

int UTF8String::compare(size_type pos1, size_type count1, const WStringView& str, size_type pos2, size_type count2) const
{
    auto utf8String = ConvertToUTF8CharArray(str.substr(pos2, count2));
    return compare(pos1, count1, StringView{ utf8String.data(), utf8String.size() });
}

SmallVector<wchar_t> UTF8String::ToWCharArray() const
{
    return ConvertToUTF16WCharArray(StringView{ c_str(), size() });
}


} // /namespace LLGL



// ================================================================================
