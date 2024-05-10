/*
 * SparseForwardIterator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPARSE_FORWARD_ITERATOR_H
#define LLGL_SPARSE_FORWARD_ITERATOR_H


namespace LLGL
{


// Forward iterator over a range of objects that skips over null pointers. See D3D11BindingTable.
template <typename T>
class SparseForwardIterator
{

    public:

        using value_type    = T;
        using pointer       = T*;
        using reference     = T&;

    public:

        SparseForwardIterator() noexcept = default;
        SparseForwardIterator(const SparseForwardIterator&) noexcept = default;
        SparseForwardIterator& operator = (const SparseForwardIterator&) noexcept = default;

        SparseForwardIterator(pointer end) noexcept :
            cur_ { end },
            end_ { end }
        {
        }

        SparseForwardIterator(pointer cur, pointer end) noexcept :
            cur_ { cur },
            end_ { end }
        {
            SkipNullEntries();
        }

        SparseForwardIterator& operator ++ ()
        {
            ++cur_;
            SkipNullEntries();
            return *this;
        }

        SparseForwardIterator operator ++ (int)
        {
            SparseForwardIterator result = *this;
            this->operator++();
            return result;
        }

        bool operator == (const SparseForwardIterator& rhs) const noexcept
        {
            return (cur_ == rhs.cur_);
        }

        bool operator != (const SparseForwardIterator& rhs) const noexcept
        {
            return (cur_ != rhs.cur_);
        }

        reference operator * () const noexcept
        {
            return *cur_;
        }

        pointer operator -> () const noexcept
        {
            return cur_;
        }

    private:

        void SkipNullEntries() noexcept
        {
            while (*cur_ == nullptr && cur_ != end_)
                ++cur_;
        }

    private:

        pointer cur_ = nullptr;
        pointer end_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
