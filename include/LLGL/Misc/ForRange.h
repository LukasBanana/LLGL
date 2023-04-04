/*
 * ForRange.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FOR_RANGE_H
#define LLGL_FOR_RANGE_H


#include <type_traits>


// Returns the count of iterations for a range-based for loop (see 'for_range' and 'for_range_reverse' macros).
#define for_range_end(INDEX) \
    ( INDEX ## _End )

// Range based for-loop over the half-open range [BEGIN, END).
#define for_subrange(INDEX, BEGIN, END)                                                                         \
    for                                                                                                         \
    (                                                                                                           \
        typename LLGL::Utils::ForRangeTypeTraits<decltype(END)>::type INDEX ## _End = (END), INDEX = (BEGIN);   \
        INDEX < for_range_end(INDEX);                                                                           \
        ++INDEX                                                                                                 \
    )

// Range based for-loop over the half-open range [0, END).
#define for_range(INDEX, END) \
    for_subrange(INDEX, 0, END)

// Range based for-loop over the half-open range [START, END) in reverse order.
#define for_subrange_reverse(INDEX, START, END)                                                                             \
    for                                                                                                                     \
    (                                                                                                                       \
        typename LLGL::Utils::ForRangeTypeTraits<decltype(END)>::type INDEX ## _End = (END),                                \
        INDEX ## _Start = (START),                                                                                          \
        INDEX ## _Iter = (INDEX ## _Start),                                                                                 \
        INDEX;                                                                                                              \
        (INDEX = (INDEX ## _Start) + for_range_end(INDEX) - (INDEX ## _Iter) - 1), (INDEX ## _Iter) < for_range_end(INDEX); \
        ++(INDEX ## _Iter)                                                                                                  \
    )

// Range based for-loop over the half-open range [0, END) in reverse order.
#define for_range_reverse(INDEX, END) \
    for_subrange_reverse(INDEX, 0, END)

// Iterator of the reverse range based for-loop.
#define for_range_reverse_iter(INDEX) \
    (INDEX ## _Iter)


namespace LLGL
{

namespace Utils
{


//! Utility template to deduce range-based for loop iterator type. This is the base structure and should not be used directly.
template <typename T, bool IsEnum>
struct ForRangeTypeTraitsBase {};

//! Utility template to deduce range-based for loop iterator type for non-enumeration types.
template <typename T>
struct ForRangeTypeTraitsBase<T, false>
{
    using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

//! Utility template to deduce range-based for loop iterator type for enumeration types.
template <typename T>
struct ForRangeTypeTraitsBase<T, true>
{
    using type = typename std::underlying_type<T>::type;
};

//! Utility template to deduce range-based for loop iterator type.
template <typename T>
struct ForRangeTypeTraits
{
    using type = typename ForRangeTypeTraitsBase<T, std::is_enum<T>::value>::type;
};


} // /namespace Utils

} // /namespace LLGL


#endif



// ================================================================================
