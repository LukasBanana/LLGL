/*
 * StringView.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
\tparam Traits Specifies the character traits. By default std::char_traits<TChar>.
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

        /**
        \brief Constant value for an invalid position. This is the largest possible value for \c size_type.
        \see substr
        */
        static constexpr size_type npos = size_type(-1);

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

        /**
        \brief Returns true if this string view is empty, i.e. \c size() and \c length() return 0.
        \see size
        \see length
        */
        bool empty() const noexcept
        {
            return (size_ == 0);
        }

        /**
        \brief Returns the length of this string. This is equivalent to \c length().
        \see length
        */
        size_type size() const noexcept
        {
            return size_;
        }

        /**
        \brief Returns the length of this string. This is equivalent to \c size().
        */
        size_type length() const noexcept
        {
            return size_;
        }

        /**
        \brief Returns the raw pointer of this string view.
        \remarks This does not necessarily point to a NUL-terminated string like \c std::string does.
        A string view only covers a range from the first to the last character of a string whose memory is managed elsewhere.
        */
        const_pointer data() const noexcept
        {
            return data_;
        }

    public:

        /**
        \brief Returns a constant reference to the character at the specified zero-based position.
        \remarks If the string is empty, the behavior of this function is undefined.
        */
        const_reference at(size_type pos) const
        {
            return data_[pos];
        }

        /**
        \brief Returns a constant reference to the first character of this string.
        \remarks If the string is empty, the behavior of this function is undefined.
        */
        const_reference front() const
        {
            return data_[0];
        }

        /**
        \brief Returns a constant reference to the last character of this string.
        \remarks If the string is empty, the behavior of this function is undefined.
        */
        const_reference back() const
        {
            return data_[size_ - 1];
        }

    public:

        //! \see cbegin
        const_iterator begin() const noexcept
        {
            return data_;
        }

        /**
        \brief Returns constant iterator to the beginning of this string.
        \remarks This can also be used on an empty string as long as it's not dereferenced on such a string.
        */
        const_iterator cbegin() const noexcept
        {
            return data_;
        }

        //! \see crbegin
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator{ end() };
        }

        /**
        \brief Returns constant reverse iterator to the end of this string.
        \remarks This can also be used on an empty string as long as it's not dereferenced on such a string.
        */
        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{ cend() };
        }

        //! \see cend
        const_iterator end() const noexcept
        {
            return data_ + size_;
        }

        /**
        \brief Returns constant iterator to the end of this string.
        \remarks This can also be used on an empty string as long as it's not dereferenced on such a string.
        */
        const_iterator cend() const noexcept
        {
            return data_ + size_;
        }

        //! \see crend
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator{ begin() };
        }

        /**
        \brief Returns constant reverse iterator to the beginning of this string.
        \remarks This can also be used on an empty string as long as it's not dereferenced on such a string.
        */
        const_reverse_iterator crend() const
        {
            return const_reverse_iterator{ cbegin() };
        }

    public:

        /**
        \brief Returns a sub-view of this string view.
        \param[in] pos Optional zero-based position to the beginning of this string. By default 0.
        \param[in] count Optional length of the sub-view. This will be clamped to the size of this string minus the input position \c pos. By default \c npos.
        \return Empty string view if \c pos is out of bounds.
        */
        BasicStringView substr(size_type pos = 0, size_type count = npos) const
        {
            if (pos > size())
                return BasicStringView{};
            else
                return BasicStringView{ data() + pos, (std::min)(count, size() - pos) };
        }

        /**
        \brief Compares this string with the specified string in a strict-weak-order (SWO).
        \returns -1 if this string is considered to be ordered \e before the other string,
        +1 if this string is considered to be ordered \e after the other string,
        and 0 otherwise.
        */
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

        /**
        \brief Compares a sub-view of this string with the specified string.
        \see compare(const BasicStringView&)
        */
        int compare(size_type pos1, size_type count1, const BasicStringView& str) const
        {
            return substr(pos1, count1).compare(str);
        }

        /**
        \brief Compares a sub-view of this string with a sub-view of the specified string.
        \see compare(const BasicStringView&)
        */
        int compare(size_type pos1, size_type count1, const BasicStringView& str, size_type pos2, size_type count2 = npos) const
        {
            return substr(pos1, count1).compare(str.substr(pos2, count2));
        }

        //! \return Position of the first character that equals the input character starting from position \c pos. Otherwise, StringView::npos is returned.
        size_type find(TChar chr, size_type pos = 0) const noexcept
        {
            while (pos < size())
            {
                if (Traits::eq(data_[pos], chr))
                    return pos;
                ++pos;
            }
            return BasicStringView::npos;
        }

        size_type find_first_of(const TChar* sequence, size_type pos = 0) const noexcept
        {
            return FindFirstPositionOf<true>(sequence, pos, size());
        }

        size_type find_first_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return FindFirstPositionOf<true>(sequence, pos, (std::min)(count, size()));
        }

        size_type find_first_not_of(const TChar* sequence, size_type pos = 0) const noexcept
        {
            return FindFirstPositionOf<false>(sequence, pos, size());
        }

        size_type find_first_not_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return FindFirstPositionOf<false>(sequence, pos, (std::min)(count, size()));
        }

        size_type find_last_of(const TChar* sequence, size_type pos = BasicStringView::npos) const noexcept
        {
            return FindLastPositionOf<true>(sequence, pos, size());
        }

        size_type find_last_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return FindLastPositionOf<true>(sequence, pos, (std::min)(count, size()));
        }

        size_type find_last_not_of(const TChar* sequence, size_type pos = BasicStringView::npos) const noexcept
        {
            return FindLastPositionOf<false>(sequence, pos, size());
        }

        size_type find_last_not_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return FindLastPositionOf<false>(sequence, pos, (std::min)(count, size()));
        }

    public:

        //! \see at
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

        template <bool Predicate>
        size_type FindFirstPositionOf(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            pos = (std::min)(pos, count);
            while (pos < size())
            {
                for (const TChar* s = sequence; *s != TChar('\0'); ++s)
                {
                    if (Traits::eq(data_[pos], *s) == Predicate)
                        return pos;
                }
                ++pos;
            }
            return BasicStringView::npos;
        }

        template <bool Predicate>
        size_type FindLastPositionOf(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            pos = (std::min)(pos, count);
            while (pos > 0)
            {
                --pos;
                for (const TChar* s = sequence; *s != TChar('\0'); ++s)
                {
                    if (Traits::eq(data_[pos], *s) == Predicate)
                        return pos;
                }
            }
            return BasicStringView::npos;
        }

};


//! ANSI character string view (char).
using StringView = BasicStringView<char>;

//! Wide character string view (wchar_t).
using WStringView = BasicStringView<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
