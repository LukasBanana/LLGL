/*
 * CoreUtils.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CORE_UTILS_H
#define LLGL_CORE_UTILS_H


#include "../Renderer/CheckedCast.h"
#include <LLGL/Export.h>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <cstdint>
#include <cstddef>
#include <functional>


namespace LLGL
{


/* ----- Templates ----- */

// Alternative to std::make_unique<T> for strict C++11 support.
template <typename T, typename... Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Alternative to std::make_unique<T[]> for strict C++11 support.
template <typename T>
std::unique_ptr<T[]> MakeUniqueArray(std::size_t size)
{
    return std::unique_ptr<T[]>(new T[size]);
}

// Initializes the specified data of basic type of POD structure type with zeros (using ::memset).
template <class T>
void MemsetZero(T& data)
{
    static_assert(!std::is_pointer<T>::value, "MemsetZero<T>: template parameter 'T' must not be a pointer type");
    static_assert(std::is_pod<T>::value, "MemsetZero<T>: template parameter 'T' must be a plain-old-data (POD) type");
    ::memset(&data, 0, sizeof(T));
}

// Returns true if the specified container contains the entry specified by 'value' (using std::find).
template <class Container, class T>
bool Contains(const Container& cont, const T& value)
{
    return (std::find(std::begin(cont), std::end(cont), value) != std::end(cont));
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

template <class Container, typename T>
void AddOnceToSharedList(Container& cont, const std::shared_ptr<T>& entry)
{
    if (entry && std::find(cont.begin(), cont.end(), entry) == cont.end())
        cont.push_back(entry);
}

template <class Container, typename T>
void RemoveFromSharedList(Container& cont, const T* entry)
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

template <typename BaseType, typename SubType>
SubType* TakeOwnership(std::list<std::unique_ptr<BaseType>>& objectSet, std::unique_ptr<SubType>&& object)
{
    auto ref = object.get();
    objectSet.emplace_back(std::forward<std::unique_ptr<SubType>>(object));
    return ref;
}

/*
\brief Returns the next resource from the specified resource array.
\param[in,out] numResources Specifies the remaining number of resources in the array.
\param[in,out] resourceArray Pointer to the remaining array of resource pointers.
\remarks If the last element in the array is reached,
'resourceArray' is points to the location after the last entry, and 'numResources' is 0.
*/
template <typename TSub, typename TBase>
TSub* NextArrayResource(std::uint32_t& numResources, TBase* const * & resourceArray)
{
    if (numResources > 0)
    {
        --numResources;
        return LLGL_CAST(TSub*, (*(resourceArray++)));
    }
    return nullptr;
}

// Searches an entry in an array that is always sorted; complexity is O(log n).
template <typename T>
T* FindInSortedArray(
    T*                                  data,
    std::size_t                         count,
    const std::function<int(const T&)>& comparator,
    std::size_t*                        position    = nullptr)
{
    std::size_t first = 0, last = count, index = 0;
    int order = 0;

    while (first < last)
    {
        /* Compare centered object in the container with 'compareObject' */
        index = (first + last)/2;

        /* Check if current state object is compatible */
        order = comparator(data[index]);

        if (order > 0)
        {
            /* Search in upper range */
            first = index + 1;
        }
        else if (order < 0)
        {
            /* Search in lower range */
            last = index;
        }
        else
        {
            /* Return index and object from container */
            if (position != nullptr)
                *position = index;
            return &(data[index]);
        }
    }

    /* Check if item must be inserted after last index */
    if (order > 0)
        ++index;

    /* Return last index as insertion position */
    if (position != nullptr)
        *position = index;

    return nullptr;
}

// Returns the adjusted size with the specified alignment, which is always greater or equal to 'size' (T can be UINT or VkDeviceSize for instance).
template <typename T>
T GetAlignedSize(T size, T alignment)
{
    if (alignment > 1)
    {
        const T alignmentBitmask = alignment - 1;
        return ((size + alignmentBitmask) & ~alignmentBitmask);
    }
    return size;
}

// Returns the division while always rounding up. This equivalent to 'ceil(numerator / denominator)' but for integral numbers.
template <typename T>
T DivideCeil(T numerator, T denominator)
{
    static_assert(std::is_integral<T>::value, "DivideRoundUp<T>: template parameter 'T' must be an integral type");
    return ((numerator + denominator - T(1)) / denominator);
}


} // /namespace LLGL


#endif



// ================================================================================
