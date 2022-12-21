/*
 * StringView.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_STRING_VIEW_H
#define LLGL_STRING_VIEW_H


#include <LLGL/Export.h>
#include <string>
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <stdexcept>


namespace LLGL
{


/**
\brief Constant string view container template. Keeps pointer to string and its length.
\remarks This does not provide a null-terminated string.
\tparam TChar Specifies the string character type.
\tparam Traits Specifies the characger traits. By default std::char_traits<TChar>.
*/
template <typename TChar, typename Traits = std::char_traits<TChar>>
class LLGL_EXPORT BasicStringView
{

    public:

        using value_type                = const TChar;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using const_reference           = const TChar&;
        using reference                 = const_reference;
        using const_pointer             = const TChar*;
        using pointer                   = const_pointer;
        using const_iterator            = const TChar*;
        using iterator                  = const_iterator;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
        using reverse_iterator          = const_reverse_iterator;

    public:

        static const size_type npos = -1;

    public:

        BasicStringView() = default;
        BasicStringView(const BasicStringView&) = default;
        BasicStringView(BasicStringView&&) = default;
        BasicStringView& operator = (const BasicStringView&) = default;
        BasicStringView& operator = (BasicStringView&&) = default;

        //! Initializes the string view with the specified null-terminated string.
        BasicStringView(const_pointer str) :
            data_ { str                                },
            size_ { BasicStringView::StringLength(str) }
        {
        }

        //! Initializes the string view with a pointer to the data and size.
        BasicStringView(const_pointer str, size_type len) :
            data_ { str },
            size_ { len }
        {
        }

        //! Initializes the string view with another templated string class. This would usually be std::basic_string.
        template <template <class, class, class> class TString, class Allocator>
        BasicStringView(const TString<TChar, Traits, Allocator>& str) :
            data_ { str.data() },
            size_ { str.size() }
        {
        }

    public:

        bool empty() const
        {
            return (size_ == 0);
        }

        size_type size() const
        {
            return size_;
        }

        size_type length() const
        {
            return size_;
        }

        const_pointer data() const
        {
            return data_;
        }

        const_reference at(size_type pos) const
        {
            return data_[pos];
        }

        const_reference front() const
        {
            return data_[0];
        }

        const_reference back() const
        {
            return data_[size_ - 1];
        }

    public:

        const_iterator begin() const
        {
            return data_;
        }

        const_iterator cbegin() const
        {
            return data_;
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator{ end() };
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{ cend() };
        }

        const_iterator end() const
        {
            return data_ + size_;
        }

        const_iterator cend() const
        {
            return data_ + size_;
        }

        const_reverse_iterator rend() const
        {
            return const_reverse_iterator{ begin() };
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator{ cbegin() };
        }

    public:

        BasicStringView substr(size_type pos = 0, size_type count = npos) const
        {
            if (pos > size())
                throw std::out_of_range("LLGL::BasicStringView::substr: pos is out of range");
            return BasicStringView{ data() + pos, (std::min)(count, size() - pos) };
        }

        int compare(const BasicStringView& str) const
        {
            size_type n = (std::min)(size(), str.size());
            int result = Traits::compare(data(), str.data(), n);
            if (result == 0)
            {
                if (size() < str.size())
                    return -1;
                if (size() > str.size())
                    return +1;
                return 0;
            }
            return result;
        }

        int compare(size_type pos1, size_type count1, const BasicStringView& str) const
        {
            return substr(pos1, count1).compare(str);
        }

        int compare(size_type pos1, size_type count1, const BasicStringView& str, size_type pos2, size_type count2 = npos) const
        {
            return substr(pos1, count1).compare(str.substr(pos2, count2));
        }

    public:

        const_reference operator [] (size_type pos) const
        {
            return data_[pos];
        }

    private:

        const_pointer   data_ = nullptr;
        size_type       size_ = 0;

    private:

        // Custom implementation of ::strlen for generic string characters.
        static size_type StringLength(const_pointer str)
        {
            if (str != nullptr)
            {
                const_pointer s;
                for (s = str; *s; ++s) {}
                return (s - str);
            }
            return 0;
        }

};


//! ANSI character string view (char).
using StringView = BasicStringView<char>;

//! Wide character string view (wchar_t).
using WStringView = BasicStringView<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
