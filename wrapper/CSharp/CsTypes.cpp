/*
 * CsTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsTypes.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LHermanns
{

namespace LLGL
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


/*
 * Viewport class
 */

Viewport::Viewport()
{
    X           = 0.0f;
    Y           = 0.0f;
    Width       = 0.0f;
    Height      = 0.0f;
    MinDepth    = 0.0f;
    MaxDepth    = 1.0f;
}

Viewport::Viewport(float x, float y, float width, float height)
{
    X           = x;
    Y           = y;
    Width       = width;
    Height      = height;
    MinDepth    = 0.0f;
    MaxDepth    = 1.0f;
}

Viewport::Viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
    X           = x;
    Y           = y;
    Width       = width;
    Height      = height;
    MinDepth    = minDepth;
    MaxDepth    = maxDepth;
}

Viewport::Viewport(Offset2D^ offset, Extent2D^ extent)
{
    X           = static_cast<float>(offset->X);
    Y           = static_cast<float>(offset->Y);
    Width       = static_cast<float>(extent->Width);
    Height      = static_cast<float>(extent->Height);
    MinDepth    = 0.0f;
    MaxDepth    = 1.0f;
}

Viewport::Viewport(Offset2D^ offset, Extent2D^ extent, float minDepth, float maxDepth)
{
    X           = static_cast<float>(offset->X);
    Y           = static_cast<float>(offset->Y);
    Width       = static_cast<float>(extent->Width);
    Height      = static_cast<float>(extent->Height);
    MinDepth    = minDepth;
    MaxDepth    = maxDepth;
}


/*
 * Scissor class
 */

Scissor::Scissor()
{
    X       = 0;
    Y       = 0;
    Width   = 0;
    Height  = 0;
}

Scissor::Scissor(int x, int y, int width, int height)
{
    X       = x;
    Y       = y;
    Width   = width;
    Height  = height;
}

Scissor::Scissor(Offset2D^ offset, Extent2D^ extent)
{
    X       = offset->X;
    Y       = offset->Y;
    Width   = static_cast<int>(extent->Width);
    Height  = static_cast<int>(extent->Height);
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
