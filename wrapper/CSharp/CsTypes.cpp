/*
 * CsTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsTypes.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


/*
 * Extent2D class
 */

Extent2D::Extent2D()
{
    Width   = 0;
    Height  = 0;
}

Extent2D::Extent2D(unsigned int width, unsigned int height)
{
    Width   = width;
    Height  = height;
}


/*
 * Extent3D class
 */

Extent3D::Extent3D()
{
    Width   = 0;
    Height  = 0;
    Depth   = 0;
}

Extent3D::Extent3D(unsigned int width, unsigned int height, unsigned int depth)
{
    Width   = width;
    Height  = height;
    Depth   = depth;
}


/*
 * Offset2D class
 */

Offset2D::Offset2D()
{
    X = 0;
    Y = 0;
}

Offset2D::Offset2D(int x, int y)
{
    X = x;
    Y = y;
}


/*
 * Offset3D class
 */

Offset3D::Offset3D()
{
    X = 0;
    Y = 0;
    Z = 0;
}

Offset3D::Offset3D(int x, int y, int z)
{
    X = x;
    Y = y;
    Z = z;
}


} // /namespace SharpLLGL



// ================================================================================
