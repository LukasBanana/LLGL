/*
 * CoreUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_CORE_UTILS_H
#define LLGL_CORE_UTILS_H


#include "../Renderer/CheckedCast.h"
#include "Assertion.h"
#include <LLGL/Export.h>
#include <LLGL/Types.h>
#include <algorithm>
#include <type_traits>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <functional>


namespace LLGL
{


/* ----- Template structures ----- */

// Required for std::unordered_map in Android/Clang compiler environment
template <typename T>
struct EnumHasher
{
    std::size_t operator() (const T& key) const
    {
        return std::hash<typename std::underlying_type<T>::type>{}(static_cast<typename std::underlying_type<T>::type>(key));
    }
};


/* ----- Template functions ----- */

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

template <class Container, class UnaryPredicate>
void RemoveAllConsecutiveFromListIf(Container& cont, UnaryPredicate pred)
{
    auto first = std::find_if(std::begin(cont), std::end(cont), pred);
    auto last = std::find_if_not(first, std::end(cont), pred);
    cont.erase(first, last);
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

// Resizes the specified container and throws an exception if the container would have to be re-allocated.
// Returns the pointer to the first new element.
template <typename Container>
typename Container::pointer ResizeNoRealloc(Container& cont, typename Container::size_type count)
{
    LLGL_ASSERT(cont.size() + count <= cont.capacity(), "exceeded capacity to append element without re-allocating container");
    typename Container::size_type offset = cont.size();
    cont.resize(cont.size() + count);
    return &(cont[offset]);
}

// Appends one new element to the specified container and throws an exception if the container would have to be re-allocated.
template <typename Container>
typename Container::reference AppendElementNoRealloc(Container& cont)
{
    return *ResizeNoRealloc(cont, 1);
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

// Returns 'numerator' divided by 'denominator' while always rounding up.
template <typename T>
constexpr T DivideRoundUp(T numerator, T denominator)
{
    static_assert(std::is_integral<T>::value, "DivideRoundUp<T>: template parameter 'T' must be an integral type");
    return ((numerator + denominator - T(1)) / denominator);
}

// Returns the adjusted size with the specified alignment, which is always greater or equal to 'size' (T can be UINT or VkDeviceSize for instance).
template <typename T>
constexpr T GetAlignedSize(T size, T alignment)
{
    return (alignment > 1 ? DivideRoundUp<T>(size, alignment) * alignment : size);
}

/*
Returns the image buffer size (in bytes) with aligned row stride for a given 3D extent.
The last row of the last layer will have length 'rowSize', all other rows will have length 'alignedRowStride'.
*/
template <typename T>
constexpr T GetAlignedImageSize(const Extent3D& extent, T rowSize, T alignedRowStride)
{
    return (alignedRowStride * extent.height) * (extent.depth - 1) + (alignedRowStride * (extent.height - 1) + rowSize);
}

// Clamps value x into the range [minimum, maximum].
template <typename T>
const T& Clamp(const T& x, const T& minimum, const T& maximum)
{
    return std::max<T>(minimum, std::min<T>(x, maximum));
}

// Casts the input raw pointer to the typed pointer if the input size matches.
template <typename T>
T* GetTypedNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (nativeHandle != nullptr && nativeHandleSize == sizeof(T))
        return static_cast<T*>(nativeHandle);
    else
        return nullptr;
}


} // /namespace LLGL


#endif



// ================================================================================
