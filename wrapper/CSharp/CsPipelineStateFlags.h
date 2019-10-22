/*
 * CsPipelineStateFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsShaderProgram.h"
#include "CsRenderSystemChild.h"
#include "CsTypes.h"
#include "CsColor.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


/* ----- Enumerations ----- */

public enum class PrimitiveTopology
{
    PointList,
    LineList,
    LineStrip,
    LineLoop,
    LineListAdjacency,
    LineStripAdjacency,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    TriangleListAdjacency,
    TriangleStripAdjacency,
    Patches1,
    Patches2,
    Patches3,
    Patches4,
    Patches5,
    Patches6,
    Patches7,
    Patches8,
    Patches9,
    Patches10,
    Patches11,
    Patches12,
    Patches13,
    Patches14,
    Patches15,
    Patches16,
    Patches17,
    Patches18,
    Patches19,
    Patches20,
    Patches21,
    Patches22,
    Patches23,
    Patches24,
    Patches25,
    Patches26,
    Patches27,
    Patches28,
    Patches29,
    Patches30,
    Patches31,
    Patches32,
};

public enum class CompareOp
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Ever,
};

public enum class StencilOp
{
    Keep,
    Zero,
    Replace,
    IncClamp,
    DecClamp,
    Invert,
    IncWrap,
    DecWrap,
};

public enum class BlendOp
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DstColor,
    InvDstColor,
    DstAlpha,
    InvDstAlpha,
    SrcAlphaSaturate,
    BlendFactor,
    InvBlendFactor,
    Src1Color,
    InvSrc1Color,
    Src1Alpha,
    InvSrc1Alpha,
};

public enum class BlendArithmetic
{
    Add,
    Subtract,
    RevSubtract,
    Min,
    Max,
};

public enum class PolygonMode
{
    Fill,
    Wireframe,
    Points,
};

public enum class CullMode
{
    Disabled,
    Front,
    Back,
};

public enum class LogicOp
{
    Disabled,
    Clear,
    Set,
    Copy,
    CopyInverted,
    NoOp,
    Invert,
    AND,
    ANDReverse,
    ANDInverted,
    NAND,
    OR,
    ORReverse,
    ORInverted,
    NOR,
    XOR,
    Equiv,
};


/* ----- Structures ----- */

public ref class Viewport
{

    public:

        Viewport();
        Viewport(float x, float y, float width, float height);
        Viewport(float x, float y, float width, float height, float minDepth, float maxDepth);
        Viewport(Extent2D^ extent);
        Viewport(Extent2D^ extent, float minDepth, float maxDepth);
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

public ref class DepthDescriptor
{

    public:

        DepthDescriptor();

        property bool        TestEnabled;
        property bool        WriteEnabled;
        property CompareOp   CompareOp;

};

public ref class StencilFaceDescriptor
{

    public:

        StencilFaceDescriptor();

        property StencilOp      StencilFailOp;
        property StencilOp      DepthFailOp;
        property StencilOp      DepthPassOp;
        property CompareOp      CompareOp;
        property unsigned int   ReadMask;
        property unsigned int   WriteMask;
        property unsigned int   Reference;

};

public ref class StencilDescriptor
{

    public:

        StencilDescriptor();

        property bool                   TestEnabled;
        property bool                   ReferenceDynamic;
        property StencilFaceDescriptor^ Front;
        property StencilFaceDescriptor^ Back;

};

public ref class DepthBiasDescriptor
{

    public:

        DepthBiasDescriptor();

        property float ConstantFactor;
        property float SlopeFactor;
        property float Clamp;

};

public ref class RasterizerDescriptor
{

    public:

        RasterizerDescriptor();

        property PolygonMode                PolygonMode;
        property CullMode                   CullMode;
        property DepthBiasDescriptor^       DepthBias;
        property bool                       FrontCCW;
        property bool                       DiscardEnabled;
        property bool                       DepthClampEnabled;
        property bool                       ScissorTestEnabled;
        property bool                       MultiSampleEnabled;
        property bool                       AntiAliasedLineEnabled;
        property bool                       ConservativeRasterization;
        property float                      LineWidth;

};

public ref class BlendTargetDescriptor
{

    public:

        BlendTargetDescriptor();

        property bool               BlendEnabled;
        property BlendOp            SrcColor;
        property BlendOp            DstColor;
        property BlendArithmetic    ColorArithmetic;
        property BlendOp            SrcAlpha;
        property BlendOp            DstAlpha;
        property BlendArithmetic    AlphaArithmetic;
        property ColorRGBA<bool>^   ColorMask;

};

public ref class BlendDescriptor
{

    public:

        BlendDescriptor();

        property ColorRGBA<float>^              BlendFactor;
        property bool                           BlendFactorDynamic;
        property bool                           AlphaToCoverageEnabled;
        property bool                           IndependentBlendEnabled;
        property LogicOp                        LogicOp;
        property array<BlendTargetDescriptor^>^ Targets;

};

public ref class GraphicsPipelineDescriptor
{

    public:

        GraphicsPipelineDescriptor();

        property ShaderProgram^         ShaderProgram;
        property RenderPass^            RenderPass;
        property PipelineLayout^        PipelineLayout;
        property PrimitiveTopology      PrimitiveTopology;
        property List<Viewport^>^       Viewports;
        property List<Scissor^>^        Scissors;
        property DepthDescriptor^       Depth;
        property StencilDescriptor^     Stencil;
        property RasterizerDescriptor^  Rasterizer;
        property BlendDescriptor^       Blend;

};


} // /namespace SharpLLGL



// ================================================================================
