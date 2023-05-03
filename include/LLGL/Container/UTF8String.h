/*
 * UTF8String.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_UTF8_STRING_H
#define LLGL_UTF8_STRING_H


#include <LLGL/Export.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/StringView.h>


namespace LLGL
{


/**
\brief Container class for UTF-8 encoded strings.
\remarks This class converts between \c char and \c wchar_t strings automatically, but stores strings always as UTF-8 encoded \c char strings.
*/
class LLGL_EXPORT UTF8String
{

    public:

        using value_type                = char;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using reference                 = value_type&;
        using const_reference           = const value_type&;
        using pointer                   = value_type*;
        using const_pointer             = const value_type*;
        using iterator                  = value_type*;
        using const_iterator            = const value_type*;
        using reverse_iterator          = std::reverse_iterator<iterator>;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

    public:

        static constexpr size_type npos = -1;

    public:

        //! Initialize an empty string.
        UTF8String();

        //! Initialies the UTF-8 string with a copy of the specified string.
        UTF8String(const UTF8String& rhs);

        //! Takes the ownership of the specified UTF-8 string.
        UTF8String(UTF8String&& rhs);

        //! Initializes the UTF-8 string with a copy of the specified string view.
        UTF8String(const StringView& str);

        //! Initializes the UTF-8 string with a UTF-8 encoded conversion of the specified wide string view.
        UTF8String(const WStringView& str);

        //! Initializes the UTF-8 string with a copy of the specified null-terminated string.
        UTF8String(const char* str);

        //! Initializes the UTF-8 string with a UTF-8 encoded conversion of the specified null-terminated wide string.
        UTF8String(const wchar_t* str);

        //! Initializes the UTF-8 string with a copy of another templated string class. This would usually be std::basic_string.
        template <template <class, class, class> class TString, class TChar, class Traits, class Allocator>
        UTF8String(const TString<TChar, Traits, Allocator>& str) :
            UTF8String { str.c_str() }
        {
        }

    public:

        inline bool empty() const noexcept
        {
            return (size() == 0);
        }

        inline size_type size() const noexcept
        {
            return (data_.size() - 1);
        }

        inline size_type length() const noexcept
        {
            return size();
        }

        inline size_type capacity() const noexcept
        {
            return (data_.capacity() - 1);
        }

        inline const_pointer data() const noexcept
        {
            return data_.data();
        }

        inline const_pointer c_str() const noexcept
        {
            return data_.data();
        }

    public:

        inline const_reference at(size_type pos) const
        {
            return data_[pos];
        }

        inline const_reference front() const
        {
            return *begin();
        }

        inline const_reference back() const
        {
            return *rbegin();
        }

    public:

        inline const_iterator begin() const noexcept
        {
            return data_.begin();
        }

        inline const_iterator cbegin() const noexcept
        {
            return data_.begin();
        }

        inline const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator{ end() };
        }

        inline const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{ cend() };
        }

        inline const_iterator end() const noexcept
        {
            return (data_.end() - 1);
        }

        inline const_iterator cend() const noexcept
        {
            return (data_.end() - 1);
        }

        inline const_reverse_iterator rend() const
        {
            return const_reverse_iterator{ begin() };
        }

        inline const_reverse_iterator crend() const
        {
            return const_reverse_iterator{ cbegin() };
        }

    public:

        void clear();

        int compare(const StringView& str) const;
        int compare(size_type pos1, size_type count1, const StringView& str) const;
        int compare(size_type pos1, size_type count1, const StringView& str, size_type pos2, size_type count2 = npos) const;

        int compare(const WStringView& str) const;
        int compare(size_type pos1, size_type count1, const WStringView& str) const;
        int compare(size_type pos1, size_type count1, const WStringView& str, size_type pos2, size_type count2 = npos) const;

        UTF8String substr(size_type pos = 0, size_type cout = npos) const;

    public:

        //! Convert this string to a NUL-terminated UTF-16 string.
        SmallVector<wchar_t> to_utf16() const;

    public:

        UTF8String& operator = (const UTF8String& rhs);
        UTF8String& operator = (UTF8String&& rhs);

        UTF8String& operator += (const UTF8String& rhs);
        UTF8String& operator += (const StringView& rhs);
        UTF8String& operator += (const WStringView& rhs);
        UTF8String& operator += (const char* rhs);
        UTF8String& operator += (const wchar_t* rhs);
        UTF8String& operator += (char chr);
        UTF8String& operator += (wchar_t chr);

        inline const_reference operator [] (size_type pos) const
        {
            return data_[pos];
        }

        //! Conversion operator to a null-terminated string.
        inline operator const_pointer () const
        {
            return c_str();
        }

        //! Conversion operator to a string view.
        inline operator StringView () const
        {
            return StringView{ data(), size() };
        }

    private:

        SmallVector<char> data_;

};


} // /namespace LLGL


#endif



// ================================================================================
