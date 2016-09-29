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
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>


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
void AddOnceToSharedList(std::vector<std::shared_ptr<T>>& cont, const std::shared_ptr<T>& entry)
{
    if (entry && std::find(cont.begin(), cont.end(), entry) == cont.end())
        cont.push_back(entry);
}

template <typename T, typename TBase>
void RemoveFromSharedList(std::vector<std::shared_ptr<T>>& cont, const TBase* entry)
{
    if (entry)
    {
        RemoveFromListIf(
            cont,
            [entry](const std::shared_ptr<T>& e)
            {
                return (e.get() == entry);
            }
        );
    }
}

template <typename T, typename TBase>
void RemoveFromUniqueSet(std::set<std::unique_ptr<T>>& cont, const TBase* entry)
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
SubType* TakeOwnership(std::set<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace(std::move(object));
    return ref;
}

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::vector<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::move(object));
    return ref;
}

template <typename T>
std::string ToHex(T value)
{
    std::stringstream s;
    s << std::setfill('0') << std::setw(sizeof(T)*2) << std::hex << std::uppercase << value;
    return s.str();
}


} // /namespace LLGL


#endif



// ================================================================================
