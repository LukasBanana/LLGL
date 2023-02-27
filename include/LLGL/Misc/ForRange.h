/*
 * ForRange.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_FOR_RANGE_H
#define LLGL_FOR_RANGE_H


#include <type_traits>


// Returns the count of iterations for a range-based for loop (see 'for_range' and 'for_range_reverse' macros).
#define for_range_end(INDEX) \
    ( INDEX ## _End )

// Range based for-loop over the half-open range [BEGIN, END).
#define for_subrange(INDEX, BEGIN, END)                                                                                             \
    for                                                                                                                             \
    (                                                                                                                               \
        typename std::remove_cv<typename std::remove_reference<decltype(END)>::type>::type INDEX ## _End = (END), INDEX = (BEGIN);  \
        INDEX < for_range_end(INDEX);                                                                                               \
        ++INDEX                                                                                                                     \
    )

// Range based for-loop over the half-open range [0, END).
#define for_range(INDEX, END) \
    for_subrange(INDEX, 0, END)

// Range based for-loop over the half-open range [START, END) in reverse order.
#define for_subrange_reverse(INDEX, START, END)                                                                             \
    for                                                                                                                     \
    (                                                                                                                       \
        typename std::remove_cv<typename std::remove_reference<decltype(END)>::type>::type INDEX ## _End = (END),           \
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


#endif



// ================================================================================
