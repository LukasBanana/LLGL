/*
 * Input.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_HELPER_H__
#define __LLGL_HELPER_H__


#include <algorithm>
#include <type_traits>
#include <memory>
#include <vector>
#include <algorithm>
#include <set>
#include <cstring>


namespace LLGL
{


template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
    
template <class T>
void InitMemory(T& data)
{
    static_assert(!std::is_pointer<T>::value, "'InitMemory' does not allow pointer types");
    static_assert(std::is_pod<T>::value, "'InitMemory' does only allow plain-old-data (POD)");
    memset(&data, 0, sizeof(T));
}

template <typename Container, typename Value>
void Fill(Container& container, Value&& value)
{
    std::fill(container.begin(), container.end(), value);
}

template <class Container, class T>
void RemoveFromList(Container& cont, const T& entry)
{
    auto it = std::find(std::begin(cont), std::end(cont), entry);
    if (it != std::end(cont))
        cont.erase(it);
}

template <class Container, class UnaryPredicate>
void RemoveFromListIf(Container& cont, UnaryPredicate pred)
{
    auto it = std::find_if(std::begin(cont), std::end(cont), pred);
    if (it != std::end(cont))
        cont.erase(it);
}

template <class Container, class T>
void RemoveAllFromList(Container& cont, const T& entry)
{
    cont.erase(
        std::remove(std::begin(cont), std::end(cont), entry),
        cont.end()
    );
}

template <class Container, class UnaryPredicate>
void RemoveAllFromListIf(Container& cont, UnaryPredicate pred)
{
    cont.erase(
        std::remove_if(std::begin(cont), std::end(cont), pred),
        cont.end()
    );
}

template <typename T>
void AddOnceToSharedList(std::vector<std::shared_ptr<T>>& container, const std::shared_ptr<T>& entry)
{
    if (entry && std::find(container.begin(), container.end(), entry) == container.end())
        container.push_back(entry);
}

template <typename T>
void RemoveFromSharedList(std::vector<std::shared_ptr<T>>& container, const T* entry)
{
    if (entry)
    {
        RemoveFromListIf(
            container,
            [entry](const std::shared_ptr<T>& e)
            {
                return (e.get() == entry);
            }
        );
    }
}

template <typename T>
T* TakeOwnership(std::set<std::unique_ptr<T>>& objectSet, std::unique_ptr<T>&& object)
{
    auto ref = object.get();
    objectSet.emplace(std::move(object));
    return ref;
}

template <typename T>
T* TakeOwnership(std::vector<std::unique_ptr<T>>& objectSet, std::unique_ptr<T>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::move(object));
    return ref;
}


} // /namespace LLGL


#endif



// ================================================================================
