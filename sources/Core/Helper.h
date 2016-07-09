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


namespace LLGL
{

    
template <class T>
void InitMemory(T& data)
{
    static_assert(!std::is_pointer<T>::value, "'InitMemory' does not allow pointer types");
    static_assert(std::is_pod<T>::value, "'InitMemory' does only allow plain-old-data (POD)");
    memset(&data, 0, sizeof(T));
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
void AddListenerGlob(std::vector<std::shared_ptr<T>>& container, const std::shared_ptr<T>& listener)
{
    if (listener && std::find(container.begin(), container.end(), listener) == container.end())
        container.push_back(listener);
}

template <typename T>
void RemoveListenerGlob(std::vector<std::shared_ptr<T>>& container, const T* listener)
{
    if (listener)
    {
        RemoveFromListIf(
            container,
            [listener](const std::shared_ptr<T>& lst)
            {
                return lst.get() == listener;
            }
        );
    }
}


} // /namespace LLGL


#endif



// ================================================================================
