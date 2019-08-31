/*
 * CsColor.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsColor.h"
#include <string>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


/*
 * ColorRGB struct
 */

generic <typename T>
ColorRGB<T>::ColorRGB(T r, T g, T b)
{
    R = r;
    G = g;
    B = b;
}

generic <typename T>
ColorRGB<T>::ColorRGB(ColorRGB<T>^ rhs)
{
    R = rhs->R;
    G = rhs->G;
    B = rhs->B;
}


/*
 * ColorRGBA struct
 */

generic <typename T>
ColorRGBA<T>::ColorRGBA(T r, T g, T b, T a)
{
    R = r;
    G = g;
    B = b;
    A = a;
}

generic <typename T>
ColorRGBA<T>::ColorRGBA(ColorRGBA<T>^ rhs)
{
    R = rhs->R;
    G = rhs->G;
    B = rhs->B;
    A = rhs->A;
}


} // /namespace SharpLLGL



// ================================================================================
