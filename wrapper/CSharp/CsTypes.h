/*
 * CsTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/Types.h>


namespace SharpLLGL
{


public ref class Extent2D
{

    public:

        Extent2D();
        Extent2D(unsigned int width, unsigned int height);

        property unsigned int Width;
        property unsigned int Height;

};

public ref class Extent3D
{

    public:

        Extent3D();
        Extent3D(unsigned int width, unsigned int height, unsigned int depth);

        property unsigned int Width;
        property unsigned int Height;
        property unsigned int Depth;

};

public ref class Offset2D
{

    public:

        Offset2D();
        Offset2D(int x, int y);

        property int X;
        property int Y;

};

public ref class Offset3D
{

    public:

        Offset3D();
        Offset3D(int x, int y, int z);

        property int X;
        property int Y;
        property int Z;

};


} // /namespace SharpLLGL



// ================================================================================
