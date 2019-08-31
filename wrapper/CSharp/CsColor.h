/*
 * CsColor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <string>


namespace SharpLLGL
{


generic <typename T>
public ref struct ColorRGB
{

    public:

        ColorRGB(T r, T g, T b);
        ColorRGB(ColorRGB<T>^ rhs);

        property T R;
        property T G;
        property T B;

};

generic <typename T>
public ref struct ColorRGBA
{

    public:

        ColorRGBA(T r, T g, T b, T a);
        ColorRGBA(ColorRGBA<T>^ rhs);

        property T R;
        property T G;
        property T B;
        property T A;

};


} // /namespace SharpLLGL



// ================================================================================
