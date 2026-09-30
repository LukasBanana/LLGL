/*
 * CommandBufferFlags.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/CommandBufferFlags.h>


namespace LLGL
{


LLGL_EXPORT Extent2D GetShadingRateSize(ShadingRate shadingRate)
{
    switch (shadingRate)
    {
        case ShadingRate::Size1x1: return { 1, 1 };
        case ShadingRate::Size1x2: return { 1, 2 };
        case ShadingRate::Size2x1: return { 2, 1 };
        case ShadingRate::Size2x2: return { 2, 2 };
        case ShadingRate::Size2x4: return { 2, 4 };
        case ShadingRate::Size4x2: return { 4, 2 };
        case ShadingRate::Size4x4: return { 4, 4 };
    }
    return {};
}


} // /namespace LLGL



// ================================================================================
