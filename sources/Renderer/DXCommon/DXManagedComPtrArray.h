/*
 * DXManagedComPtrArray.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DX_MANAGED_COMPTR_ARRAY_H
#define LLGL_DX_MANAGED_COMPTR_ARRAY_H


#include "ComPtr.h"
#include <LLGL/Misc/ForRange.h>
#include <vector>
#include <algorithm>
#include <stdexcept>
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
            auto index = FindFreeIndex();
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
            auto index = FindFreeIndex();
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
            if (index < container_.size())
            {
                container_[index] = object;
                if (object == nullptr)
                    lowerFreeBound_ = std::min(lowerFreeBound_, index);
            }
            else
                throw std::out_of_range("DXManagedComPtrArray<T>::Exchange(" + std::to_string(index) + "): index out of range");
        }

        // Removes the entry at the specified location.
        void Remove(std::size_t index)
        {
            if (index < container_.size())
            {
                container_[index] = nullptr;
                lowerFreeBound_ = std::min(lowerFreeBound_, index);
            }
            else
                throw std::out_of_range("DXManagedComPtrArray<T>::Remove(" + std::to_string(index) + "): index out of range");
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
