/*
 * DynamicVector.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DYNAMIC_VECTOR_H
#define LLGL_DYNAMIC_VECTOR_H


#include <LLGL/Export.h>
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


/**
\brief Template alias to a SmallVector with zero local capacity. This is intended as alternative to \c std::vector in the public interface.
\remarks The SmallVector template also allows a local capacity of 0, which effectively disables the local storage and only performs dynamic allocations.
\see SmallVector
*/
template
<
    typename T,
    typename Allocator       = std::allocator<T>,
    typename GrowStrategy    = GrowStrategyAddHalf
>
using DynamicVector = SmallVector<T, 0, Allocator, GrowStrategy>;


} // /namespace LLGL


#endif



// ================================================================================
