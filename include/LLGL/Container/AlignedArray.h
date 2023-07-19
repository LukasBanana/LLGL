/*
 * AlignedArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ALIGNED_ARRAY_H
#define LLGL_ALIGNED_ARRAY_H


#include <LLGL/Export.h>
#include <cstddef>


namespace LLGL
{


/**
\brief Alternative to \c std::aligned_storage but with support for zero length array.
\tparam T specifies the element type and the aligned of this array.
\tparam Size Specifies the size (in elements) of this array.
*/
template <typename T, std::size_t Size>
class LLGL_EXPORT AlignedArray
{

    public:

        using pointer       = T*;
        using const_pointer = const T*;
        using size_type     = std::size_t;

    public:

        //! Returns the static size of this array provided by the template argument \c Size.
        size_type size() const noexcept
        {
            return Size;
        }

        //! Returns a pointer to the aligned array.
        pointer data() noexcept
        {
            return reinterpret_cast<pointer>(data_);
        }

        //! Returns a constant pointer to the aligned array.
        const_pointer data() const noexcept
        {
            return reinterpret_cast<const_pointer>(data_);
        }

    private:

        alignas(T) char data_[Size * sizeof(T)];

};

/**
\brief Template specialization for an empty array.
\tparam T specifies the element type and the aligned of this array.
*/
template <typename T>
class AlignedArray<T, 0u>
{

    public:

        using pointer       = T*;
        using const_pointer = const T*;
        using size_type     = std::size_t;

    public:

        //! Returns 0.
        size_type size() const noexcept
        {
            return 0;
        }

        //! Returns null.
        pointer data() noexcept
        {
            return nullptr;
        }

        //! Returns null.
        const_pointer data() const noexcept
        {
            return nullptr;
        }

};


} // /namespace LLGL


#endif



// ================================================================================
