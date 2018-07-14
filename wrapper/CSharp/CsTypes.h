/*
 * CsTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/Types.h>


namespace LHermanns
{

namespace LLGL
{


public ref class Extent2D
{

    public:

        Extent2D();

        property unsigned int Width;
        property unsigned int Height;

};

public ref class Extent3D
{

    public:

        Extent3D();

        property unsigned int Width;
        property unsigned int Height;
        property unsigned int Depth;

};

public ref class Offset2D
{

    public:

        Offset2D();

        property int X;
        property int Y;

};

public ref class Offset3D
{

    public:

        Offset3D();

        property int X;
        property int Y;
        property int Z;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
