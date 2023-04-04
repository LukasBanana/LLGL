/*
 * FieldIterator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FIELD_ITERATOR_H
#define LLGL_FIELD_ITERATOR_H


#include <cstddef>
#include <type_traits>


namespace LLGL
{


// Iterator class to iterate over an array but access only a single element of a structure type.
template <typename T, typename TByte>
class BasicFieldRangeIterator
{

    public:

        using value_type    = T;
        using reference     = T&;
        using pointer       = T*;
        using size_type     = std::size_t;

    public:

        BasicFieldRangeIterator() = default;
        BasicFieldRangeIterator(const BasicFieldRangeIterator&) = default;
        BasicFieldRangeIterator& operator = (const BasicFieldRangeIterator&) = default;

        BasicFieldRangeIterator(pointer first, size_type count) :
            ptr_    { first         },
            ptrEnd_ { first + count },
            stride_ { sizeof(T)     }
        {
        }

        BasicFieldRangeIterator(pointer first, size_type count, size_type stride) :
            ptr_    { first                                                                      },
            ptrEnd_ { reinterpret_cast<pointer>(reinterpret_cast<TByte>(first) + count * stride) },
            stride_ { stride                                                                     }
        {
        }

    public:

        // Returns the current element or null if the iterator reached the end.
        pointer Get() const
        {
            return (ptr_ != ptrEnd_ ? ptr_ : nullptr);
        }

        // Returns the next element or null if the iterator reached the end.
        pointer Next()
        {
            if (ptr_ != ptrEnd_)
            {
                pointer result = ptr_;
                ptr_ = reinterpret_cast<pointer>(reinterpret_cast<TByte>(ptr_) + stride_);
                return result;
            }
            return nullptr;
        }

    public:

        reference operator * () const
        {
            return *ptr_;
        }

        pointer operator -> () const
        {
            return ptr_;
        }

    public:

        pointer     ptr_    = nullptr;
        pointer     ptrEnd_ = nullptr;
        size_type   stride_ = 0;

};

template <typename T>
using FieldRangeIterator = BasicFieldRangeIterator<T, char*>;

template <typename T>
using ConstFieldRangeIterator = BasicFieldRangeIterator<const T, const char*>;


#define LLGL_FIELD_RANGE_ITERATOR(CONT, FIELD) \
    FieldRangeIterator<BindingSlot>{ &((CONT).front().FIELD), (CONT).size(), sizeof(typename decltype(CONT)::value_type) }

#define LLGL_CONST_FIELD_RANGE_ITERATOR(CONT, FIELD) \
    ConstFieldRangeIterator<BindingSlot>{ &((CONT).front().FIELD), (CONT).size(), sizeof(typename decltype(CONT)::value_type) }


} // /namespace LLGL


#endif



// ================================================================================
