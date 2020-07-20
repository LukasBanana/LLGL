/*
 * Surface.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Surface.h>


namespace LLGL
{


/* ----- Surface class ----- */

Extent2D Surface::GetPixelResolution() const
{
    return GetContentSize();
}

Extent2D Surface::GetContentSizeForPixelResolution(const Extent2D& contentSize) const
{
    return GetContentSize();
}

} // /namespace LLGL



// ================================================================================
