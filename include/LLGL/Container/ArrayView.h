/*
 * ArrayView.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ARRAY_VIEW_H
#define LLGL_ARRAY_VIEW_H


#include <LLGL/Export.h>
#include <cstddef>
#include <iterator>
#include <initializer_list>


namespace LLGL
{


/**
\brief Constant array view container template. Keeps pointer to array and length.
\tparam T Specifies the data type of the container elements.
*/
template <typename T>
class LLGL_EXPORT ArrayView
{

    public:

        using value_type                = const T;
        using size_type                 = std::size_t;
        using difference_type           = std::ptrdiff_t;
        using const_reference           = const T&;
        using reference                 = const_reference;
        using const_pointer             = const T*;
        using pointer                   = const_pointer;
        using const_iterator            = const T*;
        using iterator                  = const_iterator;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
        using reverse_iterator          = const_reverse_iterator;

    public:

        ArrayView() = default;
        ArrayView(const ArrayView&) = default;
        ArrayView(ArrayView&&) = default;
        ArrayView& operator = (const ArrayView&) = default;
        ArrayView& operator = (ArrayView&&) = default;

        //! Initializes the array view with an std::initializer_list.
        ArrayView(const std::initializer_list<T>& list) :
            data_ { list.begin() },
            size_ { list.size()  }
        {
        }

        //! Initializes the array view with a pointer to the data and size.
        ArrayView(const_pointer data, size_type size) :
            data_ { data },
            size_ { size }
        {
        }

        //! Initializes the array view with the specified fixed size array. The size of this container will be equal to the template parameter \c <N>.
        template <std::size_t N>
        ArrayView(const T (&data)[N]) :
            data_ { data },
            size_ { N    }
        {
        }

        template <template <typename, typename> class OtherContainer, typename OtherAllocator>
        ArrayView(const OtherContainer<T, OtherAllocator>& other) :
            ArrayView { other.data(), other.size() }
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

        const_reference operator [] (size_type pos) const
        {
            return data_[pos];
        }

    private:

        const_pointer   data_ = nullptr;
        size_type       size_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
