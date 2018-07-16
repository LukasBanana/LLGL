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

public ref class Viewport
{

    public:

        Viewport();
        Viewport(float x, float y, float width, float height);
        Viewport(float x, float y, float width, float height, float minDepth, float maxDepth);
        Viewport(Offset2D^ offset, Extent2D^ extent);
        Viewport(Offset2D^ offset, Extent2D^ extent, float minDepth, float maxDepth);

        property float X;
        property float Y;
        property float Width;
        property float Height;
        property float MinDepth;
        property float MaxDepth;

};

public ref class Scissor
{

    public:

        Scissor();
        Scissor(int x, int y, int width, int height);
        Scissor(Offset2D^ offset, Extent2D^ extent);

        property int X;
        property int Y;
        property int Width;
        property int Height;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
