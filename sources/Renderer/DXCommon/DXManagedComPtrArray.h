/*
 * DXManagedComPtrArray.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DX_MANAGED_COMPTR_ARRAY_H
#define LLGL_DX_MANAGED_COMPTR_ARRAY_H


#include "ComPtr.h"
#include "../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <vector>
#include <algorithm>
#include <string>


namespace LLGL
{


// Container class to manage removal of empty entries when new ones are added without changing the indices of all other entries.
template <typename T>
class DXManagedComPtrArray
{

    public:

        using iterator          = typename std::vector<ComPtr<T>>::iterator;
        using const_iterator    = typename std::vector<ComPtr<T>>::const_iterator;

    public:

        // Emplaces the specified object into the first available entry.
        T* Emplace(const ComPtr<T>& object, std::size_t* outIndex = nullptr)
        {
            const std::size_t index = FindFreeIndex();
            if (index < container_.size())
                container_[index] = object;
            else
                container_.push_back(object);
            if (outIndex != nullptr)
                *outIndex = index;
            return container_[index].Get();
        }

        // Emplaces the specified object into the first available entry.
        T* Emplace(ComPtr<T>&& object, std::size_t* outIndex = nullptr)
        {
            const std::size_t index = FindFreeIndex();
            if (index < container_.size())
                container_[index] = std::forward<ComPtr<T>&&>(object);
            else
                container_.push_back(std::forward<ComPtr<T>&&>(object));
            if (outIndex != nullptr)
                *outIndex = index;
            return container_[index].Get();
        }

        // Replaces the entry at the specified position.
        void Exchange(std::size_t index, const ComPtr<T>& object)
        {
            LLGL_ASSERT(index < container_.size());
            container_[index] = object;
            if (object == nullptr)
                lowerFreeBound_ = std::min(lowerFreeBound_, index);
        }

        // Removes the entry at the specified location.
        void Remove(std::size_t index)
        {
            LLGL_ASSERT(index < container_.size());
            container_[index] = nullptr;
            lowerFreeBound_ = std::min(lowerFreeBound_, index);
        }

    public:

        iterator begin()
        {
            return container_.begin();
        }

        const_iterator begin() const
        {
            return container_.begin();
        }

        const_iterator cbegin() const
        {
            return container_.cbegin();
        }

        iterator end()
        {
            return container_.begin();
        }

        const_iterator end() const
        {
            return container_.begin();
        }

        const_iterator cend() const
        {
            return container_.cbegin();
        }

    public:

        const ComPtr<T>& operator [] (std::size_t index) const
        {
            return container_[index];
        }

    private:

        // Returns the lowest index with a free entry.
        std::size_t FindFreeIndex()
        {
            while (lowerFreeBound_ < container_.size() && container_[lowerFreeBound_] != nullptr)
                ++lowerFreeBound_;
            return lowerFreeBound_;
        }

    private:

        std::vector<ComPtr<T>>  container_;
        std::size_t             lowerFreeBound_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
