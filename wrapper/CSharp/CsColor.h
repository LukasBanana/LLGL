/*
 * CsColor.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <string>


namespace LHermanns
{

namespace LLGL
{


public ref struct ColorRGB
{

    public:

        ColorRGB();
        ColorRGB(float r, float g, float b);
        ColorRGB(ColorRGB^ rhs);

        property float R;
        property float G;
        property float B;

};

public ref struct ColorRGBA
{

    public:

        ColorRGBA();
        ColorRGBA(float r, float g, float b, float a);
        ColorRGBA(ColorRGBA^ rhs);

        property float R;
        property float G;
        property float B;
        property float A;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
