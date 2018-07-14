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


Extent2D::Extent2D()
{
    Width   = 0;
    Height  = 0;
}

Extent3D::Extent3D()
{
    Width   = 0;
    Height  = 0;
    Depth   = 0;
}

Offset2D::Offset2D()
{
    X = 0;
    Y = 0;
}

Offset3D::Offset3D()
{
    X = 0;
    Y = 0;
    Z = 0;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
