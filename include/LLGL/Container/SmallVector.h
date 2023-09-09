/*
 * SmallVector.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SMALL_VECTOR_H
#define LLGL_SMALL_VECTOR_H


#include <LLGL/Export.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/AlignedArray.h>
#include <memory>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <algorithm>
#include <initializer_list>


namespace LLGL
{


/**
\brief Container capacity grow strategy to increase the capacity to 150%.
\remarks This is the default grow strategy for the SmallVector container.
\see SmallVector
*/
struct GrowStrategyAddHalf
{
    //! Returns the increased size by adding half of the input value, effectively returning 150% the input size. This is always greater than or equal to \c size.
    static inline std::size_t Grow(std::size_t size)
    {
        return (size + size / 2);
    }
};

/**
\brief Container capacity grow strategy to increase the capacity to 200%.
\see SmallVector
*/
struct GrowStrategyDouble
{
    //! Returns the increased size by multiplying by two, effectively doubling the input size. This is always greater than or equal to \c size.
    static inline std::size_t Grow(std::size_t size)
    {
        return (size * 2);
    }
};

/**
\brief Container capacity grow strategy to increase the capacity to the next higher power of two.
\see SmallVector
*/
struct GrowStrategyRoundUpPow2
{
    static inline unsigned int Round(unsigned int v)
    {
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        ++v;
        return v;
    }

    static inline unsigned long Round(unsigned long v)
    {
        // see https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        ++v;
        return v;
    }

    static inline unsigned long long Round(unsigned long long v)
    {
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        ++v;
        return v;
    }

    //! Returns the increased size by rounding up to the next power of two. This is always greater than or equal to \c size.
    static inline std::size_t Grow(std::size_t size)
    {
        return GrowStrategyRoundUpPow2::Round(size);
    }
};

/**
\brief Generic container class for consecutive arrays optimized for small sizes.
\tparam T Specifies the array element type.
\tparam LocalCapacity Specifies the capacity of the local buffer. Up to this number of elemnts no dynamic memory allocation is necessary. By default 16.
\tparam Allocator Specifies the memory allocator. This has to be compatible with std::allocator. By default std::allocator<T>.
\tparam GrowStrategy Specifies the strategy to grow the internal storage when the container switched to heap allocations. By default GrowStrategyAddHalf.
*/
template
<
    typename    T,
    std::size_t LocalCapacity   = 16,
    typename    Allocator       = std::allocator<T>,
    typename    GrowStrategy    = GrowStrategyAddHalf
>
class LLGL_EXPORT SmallVector
{

        static_assert(std::is_copy_assignable<T>::value, "SmallVector<T>: T must be copy assignable");
        static_assert(std::is_copy_constructible<T>::value, "SmallVector<T>: T must be copy constructible");

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

        //! Default initializes an empty vector.
        SmallVector() :
            data_ { local_.data() }
        {
        }

        //! Initializes the vector with a copy of all elements from the \c other vector.
        SmallVector(const SmallVector& other) :
            SmallVector {}
        {
            operator = (other);
        }

        //! Initializes the vector with a copy of all elements from the \c other vector of different type.
        template
        <
            typename    OtherT,
            std::size_t OtherLocalCapacity,
            typename    OtherAllocator,
            typename    OtherGrowStrategy
        >
        SmallVector(const SmallVector<OtherT, OtherLocalCapacity, OtherAllocator, OtherGrowStrategy>& other) :
            SmallVector {}
        {
            operator = (other);
        }

        //! Takes the ownership of dynamically allocated elements from the \c other vector or copies all elements if the dynamic allocation is not used yet.
        SmallVector(SmallVector&& other) :
            SmallVector {}
        {
            operator = (std::forward<SmallVector&&>(other));
        }

        //! Initializes the vector with the specified elements in the half-open range <code>[from, to)</code>.
        template <typename InputIter>
        SmallVector(InputIter from, InputIter to) :
            SmallVector {}
        {
            insert(end(), from, to);
        }

        //! Initializes the vector with a copy of all elements from the specified initializer list.
        SmallVector(const std::initializer_list<T>& list) :
            SmallVector {}
        {
            insert(end(), list);
        }

        //! Initializes the vector with a copy of all elements from the \c other generic container.
        template <template <typename, typename> class OtherContainer, typename OtherAllocator>
        SmallVector(const OtherContainer<T, OtherAllocator>& other) :
            SmallVector { other.begin(), other.end() }
        {
        }

        //! Initializes the vector with the specified number of elements and initial default value.
        explicit SmallVector(size_type count) :
            SmallVector {}
        {
            resize(count);
        }

        //! Initializes the vector with the specified number of elements and initial value.
        explicit SmallVector(size_type count, const value_type& value) :
            SmallVector {}
        {
            resize(count, value);
        }

        //! Destroys all elements in this vector.
        ~SmallVector()
        {
            release();
        }

    public:

        //! Returns true if this vector is empty.
        bool empty() const noexcept
        {
            return (size_ == 0);
        }

        //! Returns the size (in number of elements) of this vector.
        size_type size() const noexcept
        {
            return size_;
        }

        //! Returns the internal capacity (in number of elements) of this vector. This refers to the memory allocated for this vector.
        size_type capacity() const noexcept
        {
            return cap_;
        }

        //! Returns a pointer to the beginning of this vector.
        pointer data() noexcept
        {
            return data_;
        }

        //! Returns a constant pointer to the beginning of this vector.
        const_pointer data() const noexcept
        {
            return data_;
        }

    public:

        /**
        \brief Returns a reference to the element at the specified position in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        reference at(size_type pos)
        {
            return data_[pos];
        }

        /**
        \brief Returns a constant reference to the element at the specified position in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        const_reference at(size_type pos) const
        {
            return data_[pos];
        }

        /**
        \brief Returns a reference to first element in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        reference front()
        {
            return data_[0];
        }

        /**
        \brief Returns a constant reference to first element in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        const_reference front() const
        {
            return data_[0];
        }

        /**
        \brief Returns a reference to last element in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        reference back()
        {
            return data_[size_ - 1];
        }

        /**
        \brief Returns a constant reference to last element in this vector.
        \remarks This must \e not be called on an empty vector!
        */
        const_reference back() const
        {
            return data_[size_ - 1];
        }

    public:

        /**
        \brief Destroys all elements in this vector.
        \remarks After this call, \c size() returns 0 but \c capacity() is unchanged.
        */
        void clear()
        {
            destroy_range(begin(), end());
            size_ = 0;
        }

        /**
        \brief Resizes this vector to the new size and default initializes all newly allocated elements.
        \param[in] size Specifies the new vector size (in number of elements).
        \remarks After this call, \c size() returns the same value as the input parameter \c size.
        \see resize(size_type, const value_type&)
        */
        void resize(size_type size)
        {
            resize(size, value_type{});
        }

        /**
        \brief Resizes this vector to the new size and explicitly initializes all newly allocated elements.
        \param[in] size Specifies the new vector size (in number of elements).
        \param[in] value Specifies the value all newly allocated elements will be initialized with.
        \remarks After this call, \c size() returns the same value as the input parameter \c size.
        \see resize(size_type)
        */
        void resize(size_type size, const value_type& value)
        {
            if (size_ < size)
            {
                reserve(size);
                construct_single(begin() + size_, begin() + size, value);
                size_ = size;
            }
            else if (size_ > size)
            {
                destroy_range(begin() + size, end());
                size_ = size;
            }
        }

        /**
        \brief Reserves memory for this vector to hold at least \c size elements.
        \param[in] size Specifies the minimum reserved capacity. The initial capacity is equal to the template argument \c LocalCapacity.
        If the new size is smaller than or equal to the current capacity, this function has no effect.
        \remarks After this call, \c capacity() returns a value that is greater than or equal to the input parameter \c size.
        */
        void reserve(size_type size)
        {
            if (cap_ < size)
                realloc(GrowStrategy::Grow(size));
        }

        /**
        \brief Re-allocates the internal memory for this vector to fit precisely the current number of elements.
        \remarks After this call, \c capacity() returns the same value as \c size().
        */
        void shrink_to_fit()
        {
            if (size_ < cap_)
                realloc();
        }

        void push_back(const value_type& value)
        {
            if (size_ == cap_)
                realloc(GrowStrategy::Grow(size_ + 1));
            Allocator alloc;
            std::allocator_traits<Allocator>::construct(alloc, end(), value);
            ++size_;
        }

        void push_back(value_type&& value)
        {
            reserve(size() + 1);
            Allocator alloc;
            std::allocator_traits<Allocator>::construct(alloc, end(), std::forward<value_type&&>(value));
            ++size_;
        }

        void pop_back()
        {
            if (size_ > 0)
            {
                --size_;
                Allocator{}.destroy(end());
            }
        }

        iterator insert(const_iterator pos, const value_type& value)
        {
            const_pointer p = &value;
            return insert(pos, p, p + 1);
        }

        template <typename InputIter>
        iterator insert(const_iterator pos, InputIter from, InputIter to)
        {
            if (from < to)
            {
                const auto count = static_cast<size_type>(std::distance(from, to));
                if (pos == end())
                {
                    /* Append elements at the end of the list */
                    reserve(size() + count);
                    return insert_inline(end(), from, count);
                }
                else if (pos >= begin() && pos < end())
                {
                    const auto offset = static_cast<size_type>(std::distance(cbegin(), pos));
                    if (size() + count <= capacity())
                    {
                        /* Move tail to new end and insert new elements in-place */
                        move_tail(begin() + offset + count, begin() + offset, end());
                        return insert_inline(const_cast<iterator>(pos), from, count);
                    }
                    else
                    {
                        /* Construct all new elements in new container */
                        return insert_realloc(offset, from, count);
                    }
                }
            }
            return const_cast<iterator>(pos);
        }

        iterator insert(const_iterator pos, const std::initializer_list<T>& list)
        {
            return insert(pos, list.begin(), list.end());
        }

        iterator erase(const_iterator pos)
        {
            return erase(pos, pos + 1);
        }

        iterator erase(const_iterator from, const_iterator to)
        {
            if (from < to && from < end() && to >= begin())
            {
                const auto count = static_cast<size_type>(std::distance(from, to));
                if (to == end())
                {
                    /* Destroy range and reduce container size */
                    destroy_range(const_cast<iterator>(from), const_cast<iterator>(to));
                    size_ -= count;
                    return end();
                }
                else
                {
                    /* Destroy range, move tail backwards, and reduce container size */
                    destroy_range(const_cast<iterator>(from), const_cast<iterator>(to));
                    move_tail(const_cast<iterator>(from), to, end());
                    size_ -= count;
                    return const_cast<iterator>(to);
                }
            }
            return end();
        }

        void swap(SmallVector& other)
        {
            if (is_dynamic() && other.is_dynamic())
            {
                /* Just swap members between both containers */
                std::swap(data_, other.data_);
                std::swap(cap_, other.cap_);
                std::swap(size_, other.size_);
            }
            else if (is_dynamic())
            {
                /* Copy local buffer of other container into local buffer of this container */
                move_range(local_.data(), other.begin(), other.end());
                std::swap(cap_, other.cap_);
                std::swap(size_, other.size_);
                other.data_ = data_;
                data_ = local_.data();
            }
            else if (other.is_dynamic())
            {
                /* Copy local buffer of this container into local buffer of other container */
                move_range(other.local_.data(), begin(), end());
                std::swap(cap_, other.cap_);
                std::swap(size_, other.size_);
                data_ = other.data_;
                other.data_ = other.local_.data();
            }
            else
            {
                /* Copy local buffer into intermediate buffer */
                char intermediate[LocalCapacity * sizeof(T)];
                auto intermediateBegin = reinterpret_cast<pointer>(intermediate);
                move_range(intermediateBegin, begin(), end());

                /* Copy local buffer of other container into local buffer of this container */
                move_range(begin(), other.begin(), other.end());

                /* Copy intermediate buffer into local buffer of other container */
                move_range(other.begin(), intermediateBegin, intermediateBegin + size());

                /* Swap configuration */
                std::swap(size_, other.size_);
            }
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

        SmallVector& operator = (const SmallVector& rhs)
        {
            clear();
            insert(end(), rhs.begin(), rhs.end());
            return *this;
        }

        template
        <
            typename    OtherT,
            std::size_t OtherLocalCapacity,
            typename    OtherAllocator,
            typename    OtherGrowStrategy
        >
        SmallVector& operator = (const SmallVector<OtherT, OtherLocalCapacity, OtherAllocator, OtherGrowStrategy>& rhs)
        {
            clear();
            insert(end(), rhs.begin(), rhs.end());
            return *this;
        }

        SmallVector& operator = (SmallVector&& rhs)
        {
            if (&rhs != this)
            {
                /* Clear this container and adopt new configuration */
                release_heap();

                if (rhs.is_dynamic())
                {
                    /* Take ownership of dynamic buffer */
                    data_ = rhs.data_;
                }
                else
                {
                    /* Copy local buffer */
                    data_ = local_.data();
                    construct_range(begin(), rhs.begin(), rhs.end());
                    rhs.destroy_range(rhs.begin(), rhs.end());
                }

                cap_    = rhs.cap_;
                size_   = rhs.size_;

                /* Drop old container */
                rhs.data_   = rhs.local_.data();
                rhs.cap_    = LocalCapacity;
                rhs.size_   = 0;
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

    private:

        void destroy_range(iterator from, iterator to)
        {
            Allocator alloc;
            for (; from != to; ++from)
                std::allocator_traits<Allocator>::destroy(alloc, from);
        }

        template <typename... TArgs>
        void construct_single(iterator from, iterator to, TArgs&&... args)
        {
            Allocator alloc;
            for (; from != to; ++from)
                std::allocator_traits<Allocator>::construct(alloc, from, std::forward<TArgs>(args)...);
        }

        template <typename InputIter>
        void construct_range(iterator pos, InputIter from, InputIter to)
        {
            Allocator alloc;
            for (pointer p = pos; from != to; ++from, ++p)
                std::allocator_traits<Allocator>::construct(alloc, p, *from);
        }

        template <typename InputIter>
        void move_range(iterator pos, InputIter from, InputIter to)
        {
            construct_range(pos, from, to);
            destroy_range(from, to);
        }

        void release_heap()
        {
            if (is_dynamic())
            {
                Allocator{}.deallocate(data_, cap_);
                data_   = local_.data();
                cap_    = LocalCapacity;
            }
        }

        void release()
        {
            destroy_range(begin(), end());
            release_heap();
        }

        void realloc(size_type cap = 0)
        {
            cap = (std::max)(cap, size_);

            if (cap > LocalCapacity)
            {
                /* Allocate new container */
                pointer data = Allocator{}.allocate(cap);

                /* Move all elements into new container */
                move_all(data);

                /* Take new container */
                data_   = data;
                cap_    = cap;
            }
            else
            {
                /* Move all elements into local buffer */
                move_all(local_.data());

                /* Use local buffer */
                data_   = local_.data();
                cap_    = cap;
            }
        }

        void move_all(pointer dst)
        {
            /* Copy elements into new container, destroy old elements, and deallocate old container */
            construct_range(dst, begin(), end());
            destroy_range(begin(), end());
            release_heap();
        }

        void move_tail_left(iterator dst, iterator from, iterator to)
        {
            Allocator alloc;
            for (; from != to; ++from, ++dst)
            {
                /* Copy element from current position 'from' to destination 'dst' and destroy the old one */
                std::allocator_traits<Allocator>::construct(alloc, dst, *from);
                std::allocator_traits<Allocator>::destroy(alloc, from);
            }
        }

        void move_tail_right(iterator dst, iterator from, iterator to)
        {
            Allocator alloc;
            const auto count = static_cast<size_type>(std::distance(from, to));
            auto rdst = reverse_iterator{ dst + count };
            for (auto rfrom = reverse_iterator{ to }, rto = reverse_iterator{ from }; rfrom != rto; ++rfrom, ++rdst)
            {
                /* Copy element from current position 'from' to destination 'dst' and destroy the old one */
                std::allocator_traits<Allocator>::construct(alloc, &(*rdst), *rfrom);
                std::allocator_traits<Allocator>::destroy(alloc, &(*rfrom));
            }
        }

        void move_tail(iterator dst, iterator from, iterator to)
        {
            if (dst < from)
                move_tail_left(dst, from, to);
            else if (dst > from)
                move_tail_right(dst, from, to);
        }

        template <typename InputIter>
        iterator insert_inline(iterator dst, InputIter src, size_type count)
        {
            construct_range(dst, src, src + count);
            size_ += count;
            return dst;
        }

        template <typename InputIter>
        iterator insert_realloc(size_type offset, InputIter src, size_type count)
        {
            /* Allocate new container */
            const size_type cap = GrowStrategy::Grow(size() + count);
            pointer data = Allocator{}.allocate(cap);

            /* Copy elements into new container, destroy old elements, and deallocate old container */
            construct_range(data, begin(), begin() + offset);
            construct_range(data + offset, src, src + count);
            construct_range(data + offset + count, begin() + offset, end());
            destroy_range(begin(), end());
            release_heap();

            /* Take new container */
            data_   = data;
            cap_    = cap;
            size_   += count;

            return begin() + offset;
        }

        bool is_dynamic() const
        {
            return (data_ != local_.data() || LocalCapacity == 0);
        }

    private:

        AlignedArray<T, LocalCapacity>  local_;
        pointer                         data_   = nullptr;
        size_type                       cap_    = LocalCapacity;
        size_type                       size_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
