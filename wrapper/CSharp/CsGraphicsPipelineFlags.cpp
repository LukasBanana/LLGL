/*
 * CsGraphicsPipelineFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsGraphicsPipelineFlags.h"


namespace LHermanns
{

namespace LLGL
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
 * MultiSamplingDescriptor class
 */

MultiSamplingDescriptor::MultiSamplingDescriptor()
{
    Enabled = false;
    Samples = 1;
}

MultiSamplingDescriptor::MultiSamplingDescriptor(unsigned int samples)
{
    Enabled = (samples > 1);
    Samples = samples;
}

unsigned int MultiSamplingDescriptor::SampleCount::get()
{
    return (Enabled && Samples > 1 ? Samples : 1);
}


/*
 * DepthDescriptor class
 */

DepthDescriptor::DepthDescriptor()
{
    TestEnabled     = false;
    WriteEnabled    = false;
    CompareOp       = LHermanns::LLGL::CompareOp::Less;
}


/*
 * StencilFaceDescriptor class
 */

StencilFaceDescriptor::StencilFaceDescriptor()
{
    StencilFailOp   = StencilOp::Keep;
    DepthFailOp     = StencilOp::Keep;
    DepthPassOp     = StencilOp::Keep;
    CompareOp       = LHermanns::LLGL::CompareOp::Less;
    ReadMask        = ~0;
    WriteMask       = ~0;
    Reference       = 0;
}


/*
 * StencilDescriptor class
 */

StencilDescriptor::StencilDescriptor()
{
    TestEnabled = false;
    Front       = gcnew StencilFaceDescriptor();
    Back        = gcnew StencilFaceDescriptor();
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
    PolygonMode                 = LHermanns::LLGL::PolygonMode::Fill;
    CullMode                    = LHermanns::LLGL::CullMode::Disabled;
    DepthBias                   = gcnew DepthBiasDescriptor();
    MultiSampling               = gcnew MultiSamplingDescriptor();
    FrontCCW                    = false;
    DepthClampEnabled           = false;
    ScissorTestEnabled          = false;
    AntiAliasedLineEnabled      = false;
    ConservativeRasterization   = false;
    LineWidth                   = 1.0f;
}


/*
 * BlendTargetDescriptor class
 */

BlendTargetDescriptor::BlendTargetDescriptor()
{
    SrcColor        = BlendOp::SrcAlpha;
    DstColor        = BlendOp::InvSrcAlpha;
    ColorArithmetic = BlendArithmetic::Add;
    SrcAlpha        = BlendOp::SrcAlpha;
    DstAlpha        = BlendOp::InvSrcAlpha;
    AlphaArithmetic = BlendArithmetic::Add;
    ColorMask       = gcnew array<bool>(4);
    for (int i = 0; i < 4; ++i)
        ColorMask[i] = true;
}


/*
 * BlendDescriptor class
 */

BlendDescriptor::BlendDescriptor()
{
    BlendEnabled            = false;
    BlendFactor             = gcnew array<float>(4);
    for (int i = 0; i < 4; ++i)
        BlendFactor[i] = 0.0f;
    AlphaToCoverageEnabled  = false;
    LogicOp                 = LHermanns::LLGL::LogicOp::Disabled;
    Targets                 = gcnew List<BlendTargetDescriptor^>();
}


/*
 * GraphicsPipelineDescriptor class
 */

GraphicsPipelineDescriptor::GraphicsPipelineDescriptor()
{
    ShaderProgram       = nullptr;
    RenderPass          = nullptr;
    PipelineLayout      = nullptr;
    PrimitiveTopology   = LHermanns::LLGL::PrimitiveTopology::TriangleList;
    Viewports           = gcnew List<Viewport^>();
    Scissors            = gcnew List<Scissor^>();
    Depth               = gcnew DepthDescriptor();
    Stencil             = gcnew StencilDescriptor();
    Rasterizer          = gcnew RasterizerDescriptor();
    Blend               = gcnew BlendDescriptor();
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
