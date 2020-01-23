/*
 * ContainerUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_CONTAINER_UTILS_H
#define LLGL_CONTAINER_UTILS_H


#include <cstddef>
#include <functional>


namespace LLGL
{

namespace Utils
{


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


} // /namespace Utils

} // /namespace LLGL


#endif



// ================================================================================
