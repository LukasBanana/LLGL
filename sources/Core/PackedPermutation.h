/*
 * PackedPermutation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PACKED_PERMUTATION_H
#define LLGL_PACKED_PERMUTATION_H


#include <cstdint>


namespace LLGL
{


// Packed permutation structure that can hold up to 3 permutation indices, e.g. { 0, 1, 2 } or { 2, 0 } etc.
struct PackedPermutation3
{
    using value_type = std::uint8_t;

    value_type bits = 0;

    inline void Append(value_type index)
    {
        /* Keep previous indices, shift new index to the left by count elements, and increment count */
        const auto count = Count();
        bits = (bits & 0x3F) | ((index & 0x03) << count*2) | ((count + 1) << 6);
    }

    inline value_type Count() const
    {
        return ((bits >> 6) & 0x03);
    }

    inline value_type operator [] (unsigned permutation) const
    {
        return ((bits >> permutation*2) & 0x03);
    }
};



} // /namespace LLGL


#endif



// ================================================================================
