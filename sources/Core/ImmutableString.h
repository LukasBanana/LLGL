/*
 * ImmutableString.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMMUTABLE_STRING_H
#define LLGL_IMMUTABLE_STRING_H


#include "Helper.h"


namespace LLGL
{


// Helper class to manage memory of a null-terminated string with only a single pointer.
template <typename T>
class ImmutableStringBase
{

    public:

        ImmutableStringBase() = default;

        // Copy constructor.
        ImmutableStringBase(const ImmutableStringBase& rhs)
        {
            CopyString(rhs.Get());
        }

        // Initializes this string with the specified null-terminated string.
        ImmutableStringBase(const T* rhs)
        {
            CopyString(rhs);
        }

        // Initializes this string with the specified STL string.
        ImmutableStringBase(const std::basic_string<T>& rhs)
        {
            CopyStringPrimary(rhs.c_str(), rhs.size());
        }

        // Copy operator.
        ImmutableStringBase& operator = (const ImmutableStringBase& rhs)
        {
            CopyString(rhs.Get());
            return *this;
        }

        // Copies the specified null-terminated string.
        ImmutableStringBase& operator = (const T* rhs)
        {
            CopyString(rhs);
            return *this;
        }

        // Copies the specified STL string.
        ImmutableStringBase& operator = (const std::basic_string<T>& rhs)
        {
            CopyStringPrimary(rhs.c_str(), rhs.size());
            return *this;
        }

        // Clears the internal buffer.
        void Clear()
        {
            data_.reset();
        }

        // Returns the pointer to the first character in the null-terminated string.
        const T* Get() const
        {
            return data_.get();
        }

        // Returns the length of this string.
        std::size_t Size() const
        {
            return (Get() != nullptr ? StrLength(Get()) : 0);
        }

        // Returns true if this string has a valid buffer.
        operator bool () const
        {
            return (data_.get() != nullptr);
        }

        // Conversion operator to a null-terminated string.
        operator const T* () const
        {
            return Get();
        }

    private:

        // Copies the specified into into the internal container.
        void CopyStringPrimary(const T* str, std::size_t len)
        {
            len++;
            data_ = MakeUniqueArray<T>(len);
            ::memcpy(&data_[0], str, len);
        }

        // Copies the specified into into the internal container.
        void CopyString(const T* str)
        {
            if (str != nullptr)
                CopyStringPrimary(str, StrLength(str));
            else
                Clear();
        }

    private:

        std::unique_ptr<T[]> data_;

};

// Immutable string type for ANSI strings.
using ImmutableString = ImmutableStringBase<char>;

// Immutable string type for Unicode strings.
using ImmutableWString = ImmutableStringBase<wchar_t>;


} // /namespace LLGL


#endif



// ================================================================================
