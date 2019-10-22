/*
 * CsPipelineStateFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsPipelineStateFlags.h"


namespace SharpLLGL
{


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

Viewport::Viewport(Extent2D^ extent)
{
    X           = 0.0f;
    Y           = 0.0f;
    Width       = static_cast<float>(extent->Width);
    Height      = static_cast<float>(extent->Height);
    MinDepth    = 0.0f;
    MaxDepth    = 1.0f;
}

Viewport::Viewport(Extent2D^ extent, float minDepth, float maxDepth)
{
    X           = 0.0f;
    Y           = 0.0f;
    Width       = static_cast<float>(extent->Width);
    Height      = static_cast<float>(extent->Height);
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


/*
 * DepthDescriptor class
 */

DepthDescriptor::DepthDescriptor()
{
    TestEnabled     = false;
    WriteEnabled    = false;
    CompareOp       = SharpLLGL::CompareOp::Less;
}


/*
 * StencilFaceDescriptor class
 */

StencilFaceDescriptor::StencilFaceDescriptor()
{
    StencilFailOp   = StencilOp::Keep;
    DepthFailOp     = StencilOp::Keep;
    DepthPassOp     = StencilOp::Keep;
    CompareOp       = SharpLLGL::CompareOp::Less;
    ReadMask        = ~0;
    WriteMask       = ~0;
    Reference       = 0;
}


/*
 * StencilDescriptor class
 */

StencilDescriptor::StencilDescriptor()
{
    TestEnabled         = false;
    ReferenceDynamic    = false;
    Front               = gcnew StencilFaceDescriptor();
    Back                = gcnew StencilFaceDescriptor();
}


/*
 * DepthBiasDescriptor class
 */

DepthBiasDescriptor::DepthBiasDescriptor()
{
    ConstantFactor  = 0.0f;
    SlopeFactor     = 0.0f;
    Clamp           = 0.0f;
}


/*
 * RasterizerDescriptor class
 */

RasterizerDescriptor::RasterizerDescriptor()
{
    PolygonMode                 = SharpLLGL::PolygonMode::Fill;
    CullMode                    = SharpLLGL::CullMode::Disabled;
    DepthBias                   = gcnew DepthBiasDescriptor();
    DiscardEnabled              = false;
    FrontCCW                    = false;
    DepthClampEnabled           = false;
    ScissorTestEnabled          = false;
    MultiSampleEnabled          = false;
    AntiAliasedLineEnabled      = false;
    ConservativeRasterization   = false;
    LineWidth                   = 1.0f;
}


/*
 * BlendTargetDescriptor class
 */

BlendTargetDescriptor::BlendTargetDescriptor()
{
    BlendEnabled    = false;
    SrcColor        = BlendOp::SrcAlpha;
    DstColor        = BlendOp::InvSrcAlpha;
    ColorArithmetic = BlendArithmetic::Add;
    SrcAlpha        = BlendOp::SrcAlpha;
    DstAlpha        = BlendOp::InvSrcAlpha;
    AlphaArithmetic = BlendArithmetic::Add;
    ColorMask       = gcnew ColorRGBA<bool>(true, true, true, true);
}


/*
 * BlendDescriptor class
 */

BlendDescriptor::BlendDescriptor()
{
    BlendFactor             = gcnew ColorRGBA<float>(0.0f, 0.0f, 0.0f, 0.0f);
    BlendFactorDynamic      = false;
    AlphaToCoverageEnabled  = false;
    IndependentBlendEnabled = false;
    LogicOp                 = SharpLLGL::LogicOp::Disabled;
    Targets                 = gcnew array<BlendTargetDescriptor^>(8);
    for (int i = 0; i < Targets->Length; ++i)
        Targets[i] = gcnew BlendTargetDescriptor();
}


/*
 * GraphicsPipelineDescriptor class
 */

GraphicsPipelineDescriptor::GraphicsPipelineDescriptor()
{
    ShaderProgram       = nullptr;
    RenderPass          = nullptr;
    PipelineLayout      = nullptr;
    PrimitiveTopology   = SharpLLGL::PrimitiveTopology::TriangleList;
    Viewports           = gcnew List<Viewport^>();
    Scissors            = gcnew List<Scissor^>();
    Depth               = gcnew DepthDescriptor();
    Stencil             = gcnew StencilDescriptor();
    Rasterizer          = gcnew RasterizerDescriptor();
    Blend               = gcnew BlendDescriptor();
}


} // /namespace SharpLLGL



// ================================================================================
