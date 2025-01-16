/*
 * StringLiteral.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_STRING_LITERAL_H
#define LLGL_STRING_LITERAL_H


#include <LLGL/Export.h>
#include <LLGL/Container/StringView.h>
#include <cstring>


namespace LLGL
{


/**
\brief Constant string container template to either reference a compile-time string literal (lightweight) or own a dynamic string (non-mutable).
\remarks This provides a null-terminated string and serves as a lightweight representation for a string that is not meant to change once assigned.
\tparam TChar Specifies the string character type.
\tparam Traits Specifies the character traits. By default std::char_traits<TChar>.
*/
template
<
    typename TChar,
    typename Traits     = std::char_traits<TChar>,
    typename Allocator  = std::allocator<TChar>
>
class LLGL_EXPORT BasicStringLiteral
{

    public:

        using value_type                = const TChar;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using const_reference           = const TChar&;
        using reference                 = const_reference;
        using const_pointer             = const TChar*;
        using pointer                   = TChar*;
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

        //! Initializes an empty string literal.
        BasicStringLiteral() :
            data_ { BasicStringLiteral::GetEmptyString() }
        {
        }

        /**
        \brief Initializes the string with a copy of the input string.
        \remarks This either makes a lightweight copy of the reference or allocates a new managed string.
        */
        BasicStringLiteral(const BasicStringLiteral& rhs)
        {
            if (rhs.IsManaged())
                CopyFrom(rhs.AsView());
            else
                data_ = rhs.data_;
        }

        //! Moves ownership from the input string literal to the newly constructed string literal.
        BasicStringLiteral(BasicStringLiteral&& rhs) noexcept :
            data_ { rhs.data_ },
            size_ { rhs.size_ }
        {
            rhs.data_ = BasicStringLiteral::GetEmptyString();
            rhs.size_ = size_type(-1);
        }

        BasicStringLiteral& operator = (const BasicStringLiteral& rhs)
        {
            if (rhs.IsManaged())
                CopyFrom(rhs.AsView());
            else
            {
                data_ = rhs.data_;
                size_ = size_type(-1);
            }
            return *this;
        }

        BasicStringLiteral& operator = (BasicStringLiteral&& rhs)
        {
            clear();
            data_ = rhs.data_;
            size_ = rhs.size_;
            rhs.data_ = BasicStringLiteral::GetEmptyString();
            rhs.size_ = size_type(-1);
            return *this;
        }

        //! Initializes the string literal as a reference (non-managed).
        BasicStringLiteral(const_pointer str) :
            data_ { str }
        {
        }

        //! Initializes the string literal as either a reference to (non-managed) or copy of (managed) the input string.
        BasicStringLiteral(const_pointer str, bool isManaged)
        {
            if (isManaged)
                CopyFrom(BasicStringView<TChar, Traits>{ str });
            else
                data_ = str;
        }

        //! Initializes the string literal with the specified null-terminated string.
        BasicStringLiteral(const BasicStringView<TChar, Traits>& str)
        {
            CopyFrom(str);
        }

        //! Initializes the string literal with another templated string class. This would usually be std::basic_string.
        template <template <class, class, class> class TString>
        BasicStringLiteral(const TString<TChar, Traits, Allocator>& str)
        {
            CopyFrom(str);
        }

        //! Destrcuts the string literal. If this is a managed string, it will delete its internal memory.
        ~BasicStringLiteral()
        {
            clear();
        }

    public:

        /**
        \brief Returns true if this string literal is empty, i.e. \c size() and \c length() return 0.
        \see size
        \see length
        */
        bool empty() const noexcept
        {
            return (size() == 0);
        }

        /**
        \brief Returns the length of this string. This is equivalent to \c length().
        \see length
        */
        size_type size() const noexcept
        {
            return (IsManaged() ? size_ : BasicStringView<TChar, Traits>{ data() }.size());
        }

        /**
        \brief Returns the length of this string. This is equivalent to \c size().
        */
        size_type length() const noexcept
        {
            return size();
        }

        /**
        \brief Returns the raw pointer of this string literal.
        \remarks This is equivalent to c_str().
        */
        const_pointer data() const noexcept
        {
            return c_str();
        }

        /**
        \brief Returns the raw pointer of this string literal.
        \remarks This points to a NUL-terminated string like \c std::string does.
        */
        const_pointer c_str() const noexcept
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
            return data_[size() - 1];
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
            return data_ + size();
        }

        /**
        \brief Returns constant iterator to the end of this string.
        \remarks This can also be used on an empty string as long as it's not dereferenced on such a string.
        */
        const_iterator cend() const noexcept
        {
            return data_ + size();
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
        \brief Compares this string with the specified string in a strict-weak-order (SWO).
        \returns -1 if this string is considered to be ordered \e before the other string,
        +1 if this string is considered to be ordered \e after the other string,
        and 0 otherwise.
        */
        int compare(const BasicStringLiteral& str) const
        {
            return AsView().compare(BasicStringView<TChar, Traits>{ str.data(), str.size() });
        }

        /**
        \brief Compares a sub-view of this string with the specified string.
        \see compare(const BasicStringLiteral&)
        */
        int compare(size_type pos1, size_type count1, const BasicStringLiteral& str) const
        {
            return AsView().compare(pos1, count1, str.AsView());
        }

        /**
        \brief Compares a sub-view of this string with a sub-view of the specified string.
        \see compare(const BasicStringLiteral&)
        */
        int compare(size_type pos1, size_type count1, const BasicStringLiteral& str, size_type pos2, size_type count2 = npos) const
        {
            return AsView().compare(pos1, count1, str.AsView(), pos2, count2);
        }

        //! \return Position of the first character that equals the input character starting from position \c pos. Otherwise, StringView::npos is returned.
        size_type find(TChar chr, size_type pos = 0) const noexcept
        {
            return AsView().find(chr, pos);
        }

        size_type find_first_of(const TChar* sequence, size_type pos = 0) const noexcept
        {
            return AsView().find_first_of(sequence, pos);
        }

        size_type find_first_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return AsView().find_first_of(sequence, pos, count);
        }

        size_type find_first_not_of(const TChar* sequence, size_type pos = 0) const noexcept
        {
            return AsView().find_first_not_of(sequence, pos);
        }

        size_type find_first_not_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return AsView().find_first_not_of(sequence, pos, count);
        }

        size_type find_last_of(const TChar* sequence, size_type pos = BasicStringLiteral::npos) const noexcept
        {
            return AsView().find_last_of(sequence, pos);
        }

        size_type find_last_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return AsView().find_last_of(sequence, pos, count);
        }

        size_type find_last_not_of(const TChar* sequence, size_type pos = BasicStringLiteral::npos) const noexcept
        {
            return AsView().find_last_not_of(sequence, pos);
        }

        size_type find_last_not_of(const TChar* sequence, size_type pos, size_type count) const noexcept
        {
            return AsView().find_last_not_of(sequence, pos, count);
        }

    public:

        /**
        \brief Clears this string literal by making it a reference to the empty string.
        \remarks If this was a managed string, its internal memory will be deleted.
        */
        void clear()
        {
            if (IsManaged())
            {
                /* NOTE:
                    const_cast is safe here because if this string literal is managed,
                    it was allocated and this instance owns this memory.
                    Otherwise, it is referencing a compile-time string literal.
                    To keep things simple and avoid unnecessary union declarations,
                    this class only holds a const_pointer to its string literal. */
                Allocator{}.deallocate(const_cast<pointer>(data_), size_ + 1);
            }
            data_ = BasicStringLiteral::GetEmptyString();
            size_ = size_type(-1);
        }

    public:

        //! \see at
        const_reference operator [] (size_type pos) const
        {
            return data_[pos];
        }

        //! Implicitly converts this string literal as string view.
        operator BasicStringView<TChar, Traits> () const
        {
            return AsView();
        }

    private:

        // Returns true if this is a managed string. Otherwise, it's only a reference to a compile-time string literal.
        bool IsManaged() const
        {
            return (size_ != size_type(-1));
        }

        // Returns this string literal as StringView.
        BasicStringView<TChar, Traits> AsView() const
        {
            return BasicStringView<TChar, Traits>{ data(), size() };
        }

        // Copies the specified string view and makes this a managed string literal.
        void CopyFrom(const BasicStringView<TChar, Traits>& str)
        {
            /* Allocate new string array and copy source into it */
            size_ = str.size();
            TChar* newData = Allocator{}.allocate(size_ + 1);
            std::memcpy(newData, str.data(), size_*sizeof(TChar));
            newData[size_] = TChar(0);
            data_ = newData;
        }

    private:

        static const TChar* GetEmptyString()
        {
            static const TChar emptyString{ TChar(0) };
            return &emptyString;
        }

    private:

        const_pointer   data_ = nullptr;
        size_type       size_ = size_type(-1); // -1 indicates this is a non-managed string and its size must be determined on demand.

};


//! ANSI character string literal (char).
using StringLiteral = BasicStringLiteral<char>;

//! Wide character string literal (wchar_t).
using WStringLiteral = BasicStringLiteral<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
