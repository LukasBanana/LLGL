/*
 * DynamicArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DYNAMIC_ARRAY_H
#define LLGL_DYNAMIC_ARRAY_H


#include <LLGL/Export.h>
#include <LLGL/Tags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/Memory.h>
#include <memory>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <string.h>


namespace LLGL
{


/**
\brief Generic container class for dynamic arrays that usually do not change in size.
\remarks Because this container does not support \c push_back, \c pop_back, or \c insert functionallity,
it only supports trivially constructible and trivially copyable types.
\tparam T Specifies the array element type.
\tparam Allocator Specifies the memory allocator. This has to be compatible with std::allocator. By default std::allocator<T>.
*/
template
<
    typename T,
    typename Allocator = std::allocator<T>
>
class LLGL_EXPORT DynamicArray
{

        static_assert(std::is_trivially_constructible<T>::value, "DynamicArray<T>: T must be trivially constructible");
        static_assert(std::is_trivially_copyable<T>::value, "DynamicArray<T>: T must be trivially copyable");

    public:

        using value_type                = T;
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

        //! Default initializes an empty array.
        DynamicArray() = default;

        //! Equivalent to default constructor: initializes an empty array.
        DynamicArray(std::nullptr_t) :
            DynamicArray {}
        {
        }

        //! Initializes the array with a copy of all elements from the \c other array.
        DynamicArray(const DynamicArray& other) :
            DynamicArray {}
        {
            operator = (other);
        }

        //! Takes the ownership of dynamically allocated elements from the \c other array.
        DynamicArray(DynamicArray&& other) :
            DynamicArray {}
        {
            operator = (std::forward<DynamicArray&&>(other));
        }

        //! Initializes the array with the specified elements in the half-open range <code>[from, to)</code>.
        template <typename InputIter>
        DynamicArray(InputIter from, InputIter to) :
            DynamicArray { static_cast<size_type>(std::distance(from, to)), UninitializeTag{} }
        {
            for (iterator it = begin(); it != end() && from != to; ++it, ++from)
                *it = *from;
        }

        //! Initializes the array with the specified number of elements and uninitial default value.
        explicit DynamicArray(size_type count, UninitializeTag) :
            data_ { Allocator{}.allocate(count) },
            size_ { count                       }
        {
        }

        //! Initializes the array with the specified number of elements and initial value.
        explicit DynamicArray(size_type count, const value_type& value = value_type{}) :
            DynamicArray { count, UninitializeTag{} }
        {
            Memory::Memset<T>(data(), value, size());
        }

        //! Destroys all elements in this array.
        ~DynamicArray()
        {
            if (data_ != nullptr)
                Allocator{}.deallocate(data_, size_);
        }

    public:

        //! Returns true if this array is empty.
        bool empty() const noexcept
        {
            return (size_ == 0);
        }

        //! Returns the size (in number of elements) of this array.
        size_type size() const noexcept
        {
            return size_;
        }

        //! Convenience function to be partially compatible with \c std::vector<T>. Equivalent to \c size().
        size_type capacity() const noexcept
        {
            return size();
        }

        //! Returns a pointer to the beginning of this array.
        pointer data() noexcept
        {
            return data_;
        }

        //! Returns a constant pointer to the beginning of this array.
        const_pointer data() const noexcept
        {
            return data_;
        }

        //! Convenience function to be partially compatible with \c std::unique_ptr<T[]>. Equivalent to \c data().
        pointer get() noexcept
        {
            return data();
        }

        //! Convenience function to be partially compatible with \c std::unique_ptr<T[]>. Equivalent to \c data().
        const_pointer get() const noexcept
        {
            return data();
        }

    public:

        /**
        \brief Returns a reference to the element at the specified position in this array.
        \remarks This must \e not be called on an empty array!
        */
        reference at(size_type pos)
        {
            return data_[pos];
        }

        /**
        \brief Returns a constant reference to the element at the specified position in this array.
        \remarks This must \e not be called on an empty array!
        */
        const_reference at(size_type pos) const
        {
            return data_[pos];
        }

        /**
        \brief Returns a reference to first element in this array.
        \remarks This must \e not be called on an empty array!
        */
        reference front()
        {
            return data_[0];
        }

        /**
        \brief Returns a constant reference to first element in this array.
        \remarks This must \e not be called on an empty array!
        */
        const_reference front() const
        {
            return data_[0];
        }

        /**
        \brief Returns a reference to last element in this array.
        \remarks This must \e not be called on an empty array!
        */
        reference back()
        {
            return data_[size_ - 1];
        }

        /**
        \brief Returns a constant reference to last element in this array.
        \remarks This must \e not be called on an empty array!
        */
        const_reference back() const
        {
            return data_[size_ - 1];
        }

    public:

        /**
        \brief Release the internal memory.
        \remarks After this call, \c size() and \c capacity() return 0.
        */
        void clear()
        {
            if (data_ != nullptr)
            {
                Allocator{}.deallocate(data_, size_);
                data_ = nullptr;
                size_ = 0;
            }
        }

        /**
        \brief Release the ownershuip of the internally allocated memory.
        \remarks This must later be deallocated with Allocator::deallocate().
        */
        pointer release()
        {
            pointer data = data_;
            data_ = nullptr;
            size_ = 0;
            return data;
        }

        /**
        \brief Resizes this array to the new size and default initializes all newly allocated elements.
        \param[in] newSize Specifies the new array size (in number of elements).
        \remarks After this call, \c size() returns the same value as the input parameter \c size.
        \see resize(size_type, const value_type&)
        */
        void resize(size_type newSize, UninitializeTag)
        {
            if (size_ < newSize)
            {
                /* Allocate new buffer, copy old data into new buffer, replace buffer and size property */
                pointer newData = Allocator{}.allocate(newSize);
                if (data_ != nullptr)
                {
                    ::memcpy(newData, data_, size_ * sizeof(T));
                    Allocator{}.deallocate(data_, size_);
                }
                data_ = newData;
                size_ = newSize;
            }
        }

        /**
        \brief Resizes this array to the new size and explicitly initializes all newly allocated elements.
        \param[in] newSize Specifies the new array size (in number of elements).
        \param[in] value Specifies the value all newly allocated elements will be initialized with.
        \remarks After this call, \c size() returns the same value as the input parameter \c size.
        \see resize(size_type)
        */
        void resize(size_type newSize, const value_type& value)
        {
            /* Resize uninitialized, then copy new value into tail */
            if (size_ < newSize)
            {
                const size_type oldSize = size_;
                resize(newSize, UninitializeTag{});
                Memory::Memset<T>(data() + oldSize, value, newSize - oldSize);
            }
        }

        void swap(DynamicArray& other)
        {
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
        }

    public:

        iterator begin() noexcept
        {
            return data_;
        }

        const_iterator begin() const noexcept
        {
            return data_;
        }

        const_iterator cbegin() const noexcept
        {
            return data_;
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator{ end() };
        }

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator{ end() };
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{ cend() };
        }

        iterator end() noexcept
        {
            return data_ + size_;
        }

        const_iterator end() const noexcept
        {
            return data_ + size_;
        }

        const_iterator cend() const noexcept
        {
            return data_ + size_;
        }

        reverse_iterator rend()
        {
            return reverse_iterator{ begin() };
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

        DynamicArray& operator = (const DynamicArray& rhs)
        {
            if (size() != rhs.size())
            {
                clear();
                resize(rhs.size(), UninitializeTag{});
            }
            ::memcpy(data(), rhs.data(), size() * sizeof(T));
            return *this;
        }

        DynamicArray& operator = (DynamicArray&& rhs)
        {
            if (&rhs != this)
            {
                clear();
                swap(rhs);
            }
            return *this;
        }

        const_reference operator [] (size_type pos) const
        {
            return data_[pos];
        }

        reference operator [] (size_type pos)
        {
            return data_[pos];
        }

        operator ArrayView<T> () const
        {
            return ArrayView<T>{ data(), size() };
        }

        //! Returns true if this array is non-empty.
        operator bool () const
        {
            return !empty();
        }

    private:

        pointer     data_   = nullptr;
        size_type   size_   = 0;

};


/**
\brief Common type for dynamic byte arrays.
\remarks This is primarily used for image data conversion. It only manages a pointer of dynamically allocated memory and its size (in number of elements).
\see ConvertImageBuffer
\see DecompressImageBufferToRGBA8UNorm
\see GenerateImageBuffer
*/
using DynamicByteArray = DynamicArray<char>;


} // /namespace LLGL


#endif



// ================================================================================
