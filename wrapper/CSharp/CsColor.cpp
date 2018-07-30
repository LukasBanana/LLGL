/*
 * CsColor.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsColor.h"
#include <string>

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
 * ColorRGB struct
 */

ColorRGB::ColorRGB()
{
    R = 1.0f;
    G = 1.0f;
    B = 1.0f;
}

ColorRGB::ColorRGB(float r, float g, float b)
{
    R = r;
    G = g;
    B = b;
}

ColorRGB::ColorRGB(ColorRGB^ rhs)
{
    R = rhs->R;
    G = rhs->G;
    B = rhs->B;
}


/*
 * ColorRGBA struct
 */

ColorRGBA::ColorRGBA()
{
    R = 1.0f;
    G = 1.0f;
    B = 1.0f;
    A = 1.0f;
}

ColorRGBA::ColorRGBA(float r, float g, float b, float a)
{
    R = r;
    G = g;
    B = b;
    A = a;
}

ColorRGBA::ColorRGBA(ColorRGBA^ rhs)
{
    R = rhs->R;
    G = rhs->G;
    B = rhs->B;
    A = rhs->A;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
