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

        static constexpr size_type npos = size_type(-1);

    public:

        //! Initialize an empty STL::string.
        UTF8String();

        //! Initialies the UTF-8 STL::string with a copy of the specified STL::string.
        UTF8String(const UTF8String& rhs);

        //! Takes the ownership of the specified UTF-8 STL::string.
        UTF8String(UTF8String&& rhs);

        //! Initializes the UTF-8 STL::string with a copy of the specified STL::string view.
        UTF8String(const StringView& str);

        //! Initializes the UTF-8 STL::string with a UTF-8 encoded conversion of the specified wide STL::string view.
        UTF8String(const WStringView& str);

        //! Initializes the UTF-8 STL::string with a copy of the specified null-terminated STL::string.
        UTF8String(const char* str);

        //! Initializes the UTF-8 STL::string with a UTF-8 encoded conversion of the specified null-terminated wide STL::string.
        UTF8String(const wchar_t* str);

        //! Initializes the UTF-8 STL::string with a copy of another templated STL::string class. This would usually be std::basic_STL::string.
        template <template <class, class, class> class TString, class TChar, class Traits, class Allocator>
        UTF8String(const TString<TChar, Traits, Allocator>& str) :
            UTF8String { str.c_str() }
        {
        }

        /**
        \brief Initializes the UTF-8 STL::string by moving the ownership of the internal data container.
        \param[in] data The input data must already be in UTF-8 format. This constructor does not perform any checks on this input container!
        */
        explicit UTF8String(SmallVector<char>&& data);

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

        void resize(size_type size, char ch = '\0');

        UTF8String& append(size_type count, char ch);
        UTF8String& append(const char* first, const char* last);

    public:

        //! Convert this STL::string to a NUL-terminated UTF-16 STL::string.
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

        //! Conversion operator to a null-terminated STL::string.
        inline operator const_pointer () const
        {
            return c_str();
        }

        //! Conversion operator to a STL::string view.
        inline operator StringView () const
        {
            return StringView{ data(), size() };
        }

    public:

        /**
        \brief Prints a formatted UTF-8 STL::string with a variable number of arguments.
        \remarks This follows the same formatting syntax as \c std::printf.
        \see Report::Printf
        \see Log::Printf
        */
        static UTF8String Printf(const char* format, ...);

    private:

        SmallVector<char> data_;

};


} // /namespace LLGL


#endif



// ================================================================================
