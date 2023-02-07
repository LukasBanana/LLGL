/*
 * SmallVector.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SMALL_VECTOR_H
#define LLGL_SMALL_VECTOR_H


#include <LLGL/Export.h>
#include <LLGL/Container/ArrayView.h>
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
    static inline std::size_t IncreaseToFit(std::size_t size)
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
    static inline std::size_t IncreaseToFit(std::size_t size)
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
    static inline std::size_t IncreaseToFit(std::size_t size)
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

    public:

        static_assert(std::is_copy_assignable<T>::value, "SmallVector<T>: T must be copy assignable");
        static_assert(std::is_copy_constructible<T>::value, "SmallVector<T>: T must be copy constructible");

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

        SmallVector() :
            data_ { local_data() }
        {
        }

        SmallVector(const SmallVector& other) :
            SmallVector {}
        {
            operator = (other);
        }

        SmallVector(SmallVector&& other) :
            SmallVector {}
        {
            operator = (std::forward<SmallVector&&>(other));
        }

        template <typename InputIter>
        SmallVector(InputIter from, InputIter to) :
            SmallVector {}
        {
            insert(end(), from, to);
        }

        SmallVector(const std::initializer_list<T>& list) :
            SmallVector {}
        {
            insert(end(), list);
        }

        template <template <typename, typename> class OtherContainer, typename OtherAllocator>
        SmallVector(const OtherContainer<T, OtherAllocator>& other) :
            SmallVector { other.begin(), other.end() }
        {
        }

        ~SmallVector()
        {
            release();
        }

    public:

        bool empty() const noexcept
        {
            return (size_ == 0);
        }

        size_type size() const noexcept
        {
            return size_;
        }

        size_type capacity() const noexcept
        {
            return cap_;
        }

        pointer data() noexcept
        {
            return data_;
        }

        const_pointer data() const noexcept
        {
            return data_;
        }

        reference at(size_type pos) noexcept
        {
            return data_[pos];
        }

        const_reference at(size_type pos) const noexcept
        {
            return data_[pos];
        }

        reference front() noexcept
        {
            return data_[0];
        }

        const_reference front() const noexcept
        {
            return data_[0];
        }

        reference back() noexcept
        {
            return data_[size_ - 1];
        }

        const_reference back() const noexcept
        {
            return data_[size_ - 1];
        }

    public:

        void clear()
        {
            destroy_range(begin(), end());
            size_ = 0;
        }

        void resize(size_type size)
        {
            resize(size, value_type{});
        }

        void resize(size_type size, const value_type& value)
        {
            if (size_ < size)
            {
                reserve_to_fit_min(size);
                construct_single(begin() + size_, begin() + size, value);
                size_ = size;
            }
            else if (size_ > size)
            {
                destroy_range(begin() + size, end());
                size_ = size;
            }
        }

        void reserve(size_type size)
        {
            reserve_to_fit(size);
        }

        void shrink_to_fit()
        {
            if (size_ < cap_)
                realloc(size_);
        }

        void push_back(const value_type& value)
        {
            if (size_ == cap_)
                realloc(GrowStrategy::IncreaseToFit(size_ + 1));
            Allocator{}.construct(end(), value);
            ++size_;
        }

        void push_back(value_type&& value)
        {
            reserve_to_fit_min(size() + 1);
            Allocator{}.construct(end(), std::forward<value_type&&>(value));
            ++size_;
        }

        void pop_back()
        {
            if (size_ > 0)
            {
                Allocator{}.destroy(end());
                --size_;
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
                    reserve_to_fit_min(size() + count);
                    return insert_inline(end(), from, count);
                }
                else if (pos >= begin() && pos < end())
                {
                    const auto offset = static_cast<size_type>(std::distance(cbegin(), pos));
                    if (size() + count <= capacity())
                    {
                        /* Move trail to new end and insert new elements in-place */
                        move_trail(begin() + offset + count, begin() + offset, end());
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
                    /* Destroy range, move trail backwards, and reduce container size */
                    destroy_range(const_cast<iterator>(from), const_cast<iterator>(to));
                    move_trail(const_cast<iterator>(from), to, end());
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
                move_range(local_data(), other.begin(), other.end());
                std::swap(cap_, other.cap_);
                std::swap(size_, other.size_);
                other.data_ = data_;
                data_ = local_data();
            }
            else if (other.is_dynamic())
            {
                /* Copy local buffer of this container into local buffer of other container */
                move_range(other.local_data(), begin(), end());
                std::swap(cap_, other.cap_);
                std::swap(size_, other.size_);
                data_ = other.data_;
                other.data_ = other.local_data();
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

        iterator begin()
        {
            return data_;
        }

        const_iterator begin() const
        {
            return data_;
        }

        const_iterator cbegin() const
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

        iterator end()
        {
            return data_ + size_;
        }

        const_iterator end() const
        {
            return data_ + size_;
        }

        const_iterator cend() const
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
                    data_ = local_data();
                    construct_range(begin(), rhs.begin(), rhs.end());
                    rhs.destroy_range(rhs.begin(), rhs.end());
                }

                cap_    = rhs.cap_;
                size_   = rhs.size_;

                /* Drop old container */
                rhs.data_   = rhs.local_data();
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
                alloc.destroy(from);
        }

        template <typename... TArgs>
        void construct_single(iterator from, iterator to, TArgs&&... args)
        {
            Allocator alloc;
            for (; from != to; ++from)
                alloc.construct(from, std::forward<TArgs>(args)...);
        }

        template <typename InputIter>
        void construct_range(iterator pos, InputIter from, InputIter to)
        {
            Allocator alloc;
            for (pointer p = pos; from != to; ++from, ++p)
                alloc.construct(p, *from);
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
                data_   = local_data();
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
                Allocator alloc;
                pointer data = alloc.allocate(cap);

                /* Move all elements into new container */
                move_all(data);

                /* Take new container */
                data_   = data;
                cap_    = cap;
            }
            else
            {
                /* Move all elements into local buffer */
                move_all(local_data());

                /* Use local buffer */
                data_   = local_data();
                cap_    = cap;
            }
        }

        void reserve_to_fit(size_type size)
        {
            if (cap_ < size)
                realloc(size);
        }

        void reserve_to_fit_min(size_type size)
        {
            if (cap_ < size)
                realloc(GrowStrategy::IncreaseToFit(size));
        }

        void move_all(pointer dst)
        {
            /* Copy elements into new container, destroy old elements, and deallocate old container */
            construct_range(dst, begin(), end());
            destroy_range(begin(), end());
            release_heap();
        }

        void move_trail_left(iterator dst, iterator from, iterator to)
        {
            Allocator alloc;
            for (; from != to; ++from, ++dst)
            {
                /* Copy element from current position 'from' to destination 'dst' and destroy the old one */
                alloc.construct(dst, *from);
                alloc.destroy(from);
            }
        }

        void move_trail_right(iterator dst, iterator from, iterator to)
        {
            Allocator alloc;
            const auto count = static_cast<size_type>(std::distance(from, to));
            auto rdst = reverse_iterator{ dst + count };
            for (auto rfrom = reverse_iterator{ to }, rto = reverse_iterator{ from }; rfrom != rto; ++rfrom, ++rdst)
            {
                /* Copy element from current position 'from' to destination 'dst' and destroy the old one */
                alloc.construct(&(*rdst), *rfrom);
                alloc.destroy(&(*rfrom));
            }
        }

        void move_trail(iterator dst, iterator from, iterator to)
        {
            if (dst < from)
                move_trail_left(dst, from, to);
            else if (dst > from)
                move_trail_right(dst, from, to);
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
            const size_type cap = GrowStrategy::IncreaseToFit(size() + count);
            Allocator alloc;
            pointer data = alloc.allocate(cap);

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

        pointer local_data()
        {
            return reinterpret_cast<pointer>(local_);
        }

        const_pointer local_data() const
        {
            return reinterpret_cast<const_pointer>(local_);
        }

        bool is_dynamic() const
        {
            return (data_ != local_data());
        }

    private:

        char        local_[LocalCapacity * sizeof(T)];
        pointer     data_   = nullptr;
        size_type   cap_    = LocalCapacity;
        size_type   size_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
