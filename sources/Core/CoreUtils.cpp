/*
 * CoreUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "CoreUtils.h"


namespace LLGL
{


// Ensure power-of-two alignment works
static_assert(GetAlignedSize(3u, 4u) == 4u, "GetAlignedSize<T>(3, 4) == 4 failed");
static_assert(GetAlignedSize(4u, 4u) == 4u, "GetAlignedSize<T>(4, 4) == 4 failed");
static_assert(GetAlignedSize(5u, 4u) == 8u, "GetAlignedSize<T>(5, 4) == 8 failed");

// Ensure non-power-of-two alignment works
static_assert(GetAlignedSize(6u, 6u) == 6u, "GetAlignedSize<T>(6, 6) == 6 assertion failed; This must work with any integral number, not just power-of-twos");
static_assert(GetAlignedSize(7u, 6u) == 12u, "GetAlignedSize<T>(7, 6) == 12 assertion failed; This must work with any integral number, not just power-of-twos");
static_assert(GetAlignedSize(12u, 6u) == 12u, "GetAlignedSize<T>(12, 6) == 12 assertion failed; This must work with any integral number, not just power-of-twos");


} // /namespace LLGL



// ================================================================================
