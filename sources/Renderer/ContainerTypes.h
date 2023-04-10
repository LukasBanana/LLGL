/*
 * ContainerTypes.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CONTAINER_TYPES_H
#define LLGL_CONTAINER_TYPES_H


#include "../Core/CoreUtils.h"
#include "../Core/Assertion.h"
#include "CheckedCast.h"
#include <memory>
#include <vector>
#include <utility>
#include <type_traits>
#include <unordered_set>


namespace LLGL
{


/*
 * Global function templates
 */

template <typename T, typename TBase>
void RemoveFromUniqueSet(std::unordered_set<std::unique_ptr<T>>& cont, const TBase* entry)
{
    if (entry)
    {
        RemoveFromListIf(
            cont,
            [entry](const std::unique_ptr<T>& e)
            {
                return (e.get() == entry);
            }
        );
    }
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::unordered_set<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::vector<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}


/*
 * Global container class templates
 */

// Alternative for std::unique_ptr<T> with an index into its owning container for fast removal.
template <typename T, typename Payload>
class PayloadUniquePtr final
{

        static_assert(std::is_standard_layout<Payload>::value, "PayloadUniquePtr<T, Payload>: Payload must be a standard layout");

    public:

        using pointer       = T*;
        using reference     = T&;
        using element_type  = T;

    public:

        PayloadUniquePtr() = default;

        PayloadUniquePtr(const PayloadUniquePtr&) = delete;
        PayloadUniquePtr& operator = (const PayloadUniquePtr&) = delete;

        template <typename TOther>
        PayloadUniquePtr(PayloadUniquePtr<TOther, Payload>&& rhs) :
            ptr_ { rhs.release() }
        {
        }

        template <typename TOther>
        PayloadUniquePtr& operator = (PayloadUniquePtr<TOther, Payload>&& rhs)
        {
            if (this != &rhs)
                reset(rhs.release());
            return *this;
        }

        ~PayloadUniquePtr()
        {
            reset();
        }

        pointer release()
        {
            pointer ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        void reset(pointer ptr = nullptr)
        {
            if (ptr_ != ptr)
            {
                if (ptr_ != nullptr)
                {
                    /* Call destructor and free memory including payload */
                    ptr_->~T();
                    char* mem = reinterpret_cast<char*>(ptr_) - sizeof(Payload);
                    ::delete [] mem;
                }
                ptr_ = ptr;
            }
        }

        pointer get() const noexcept
        {
            return ptr_;
        }

        void swap(PayloadUniquePtr& other) noexcept
        {
            std::swap(other.ptr_, ptr_);
        }

        Payload& payload() noexcept
        {
            return *reinterpret_cast<Payload*>(reinterpret_cast<char*>(ptr_) - sizeof(Payload));
        }

        const Payload& payload() const noexcept
        {
            return *reinterpret_cast<const Payload*>(reinterpret_cast<const char*>(ptr_) - sizeof(Payload));
        }

    public:

        pointer operator -> () const noexcept
        {
            return get();
        }

        reference operator * () const noexcept
        {
            return *get();
        }

        operator bool () const noexcept
        {
            return (get() != nullptr);
        }

    public:

        template <typename... Args>
        static PayloadUniquePtr<T, Payload> Alloc(const Payload& payload, Args&&... args)
        {
            /* Allocate memory for payload and object */
            char* mem = ::new char[sizeof(T) + sizeof(Payload)];

            /* Initialize payload */
            *reinterpret_cast<Payload*>(mem) = payload;

            /* Construct  */
            T* ptr = reinterpret_cast<T*>(mem + sizeof(Payload));
            ::new (ptr) T(std::forward<Args>(args)...);

            return PayloadUniquePtr<T, Payload>{ ptr };
        }

    private:

        PayloadUniquePtr(T* ptr) :
            ptr_ { ptr }
        {
        }

    private:

        T* ptr_ = nullptr;

};

// Payload structure for indexed unique pointers. See IndexedUniquePtr<T>.
struct IndexPayload
{
    std::size_t index;
};

template <typename T>
using IndexedUniquePtr = PayloadUniquePtr<T, IndexPayload>;

// Container class for an array of unordered unique pointers. Used by RenderSystem implementations for all child objects.
template <typename T>
class UnorderedUniquePtrVector
{

    public:

        using container_type    = std::vector<IndexedUniquePtr<T>>;
        using iterator          = typename container_type::iterator;
        using const_iterator    = typename container_type::const_iterator;

    public:

        // Allocates a new object for this container and returns a non-owning raw pointer to that object.
        template <typename TSub, typename... Args>
        TSub* emplace(Args&&... args)
        {
            /* Allocate object and assign index from container in payload */
            const IndexPayload payload{ container_.size() };
            IndexedUniquePtr<TSub> object = IndexedUniquePtr<TSub>::Alloc(payload, std::forward<Args>(args)...);
            TSub* ref = object.get();
            container_.push_back(std::move(object));
            return ref;
        }

        // Releases the memory for the specified object in that list.
        template <typename TBase>
        void erase(TBase* object)
        {
            if (object != nullptr)
            {
                /* Locate object in container with index from payload */
                T* subTypedObject = ObjectCast<T*>(object);
                auto* payload = reinterpret_cast<IndexPayload*>(reinterpret_cast<char*>(subTypedObject) - sizeof(IndexPayload));
                LLGL_ASSERT(payload->index < container_.size());

                if (payload->index + 1 < container_.size())
                {
                    /* Move last element to location of the input object in order to delete it */
                    std::swap(container_[payload->index], container_.back());

                    /* Update payload for moved object */
                    container_[payload->index].payload() = *payload;
                }

                /* Remove last element in container; it's either input object or the one moved that object's location */
                container_.pop_back();
            }
        }

        void clear()
        {
            container_.clear();
        }

        bool empty() const
        {
            return container_.empty();
        }

    public:

        const_iterator cbegin() const
        {
            return container_.cbegin();
        }

        const_iterator begin() const
        {
            return container_.begin();
        }

        iterator begin()
        {
            return container_.begin();
        }

        const_iterator cend() const
        {
            return container_.cend();
        }

        const_iterator end() const
        {
            return container_.end();
        }

        iterator end()
        {
            return container_.end();
        }

    private:

        container_type container_;

};

// Container class for a set of unordered unique pointers. Used by RenderSystem implementations for all child objects.
template <typename T>
class UnorderedUniquePtrSet
{

    public:

        using container_type    = std::unordered_set<std::unique_ptr<T>>;
        using iterator          = typename container_type::iterator;
        using const_iterator    = typename container_type::const_iterator;

    public:

        // Allocates a new object for this container and returns a non-owning raw pointer to that object.
        template <typename TSub, typename... Args>
        TSub* emplace(Args&&... args)
        {
            return TakeOwnership(container_, MakeUnique<TSub>(std::forward<Args>(args)...));
        }

        // Releases the memory for the specified object in that list.
        template <typename TBase>
        void erase(TBase* object)
        {
            RemoveFromUniqueSet(container_, object);
        }

        void clear()
        {
            container_.clear();
        }

        bool empty() const
        {
            return container_.empty();
        }

    public:

        const_iterator cbegin() const
        {
            return container_.cbegin();
        }

        const_iterator begin() const
        {
            return container_.begin();
        }

        iterator begin()
        {
            return container_.begin();
        }

        const_iterator cend() const
        {
            return container_.cend();
        }

        const_iterator end() const
        {
            return container_.end();
        }

        iterator end()
        {
            return container_.end();
        }

    private:

        container_type container_;

};


/*
 * Global typenames
 */

template <typename T>
using HWObjectInstance = std::unique_ptr<T>;

#ifdef LLGL_PREFER_STL_CONTAINERS

template <typename T>
using HWObjectContainer = UnorderedUniquePtrSet<T>;

#else

template <typename T>
using HWObjectContainer = UnorderedUniquePtrVector<T>;

#endif


} // /namespace LLGL


#endif



// ================================================================================
