/*
 * GraphicsPipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GRAPHICS_PIPELINE_FLAGS_H__
#define __LLGL_GRAPHICS_PIPELINE_FLAGS_H__


#include "Export.h"
#include "ColorRGBA.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class ShaderProgram;
class VertexBuffer;
class IndexBuffer;


/* ----- Enumerations ----- */

/**
\brief Compare operations enumeration.
\remarks This operation is used for depth-test and stencil-test.
*/
enum class CompareOp
{
    Never,          //!< Compare test never succeeds.
    Less,           //!< Compare test succeeds if the left-hand-side is less than the right-hand-side.
    Equal,          //!< Compare test succeeds if the left-hand-side is euqal to the right-hand-side.
    LessEqual,      //!< Compare test succeeds if the left-hand-side is less than or equal to the right-hand-side.
    Greater,        //!< Compare test succeeds if the left-hand-side is greater than the right-hand-side.
    NotEqual,       //!< Compare test succeeds if the left-hand-side is not equal to the right-hand-side.
    GreaterEqual,   //!< Compare test succeeds if the left-hand-side is greater than or equal to the right-hand-side.
    Ever,           //!< Compare test always succeeds. (Can not be called "Always" due to conflict with X11 lib on Linux).
};

//! Stencil operations enumeration.
enum class StencilOp
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

//! Blending operations enumeration.
enum class BlendOp
{
    Zero,
    One,

    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,

    DestColor,
    InvDestColor,
    DestAlpha,
    InvDestAlpha,
};

//! Polygon filling modes enumeration.
enum class PolygonMode
{
    Fill,
    Wireframe,
    Points,
};

//! Polygon culling modes enumeration.
enum class CullMode
{
    Disabled,   //!< No culling.
    Front,      //!< Front face culling.
    Back,       //!< Back face culling.
};


/* ----- Structures ----- */

struct DepthDescriptor
{
    bool        testEnabled     = false;
    bool        writeEnabled    = false;
    CompareOp   compareOp       = CompareOp::Less;
};

struct StencilStateDescriptor
{
    StencilOp       stencilFailOp   = StencilOp::Keep;  //!< Specifies the operation to take when the stencil test fails.
    StencilOp       depthFailOp     = StencilOp::Keep;  //!< Specifies the operation to take when the stencil test passes but the depth test fails.
    StencilOp       depthPassOp     = StencilOp::Keep;  //!< Specifies the operation to take when both the stencil test and the depth test pass.
    CompareOp       compareOp       = CompareOp::Less;  //!< Specifies the stencil compare operation.
    std::uint32_t   compareMask     = 0;
    std::uint32_t   writeMask       = 0;
    std::uint32_t   reference       = 0;
};

struct StencilDescriptor
{
    bool                    testEnabled  = false;
    StencilStateDescriptor  front;
    StencilStateDescriptor  back;
};

struct RasterizerDescriptor
{
    PolygonMode polygonMode             = PolygonMode::Fill;
    CullMode    cullMode                = CullMode::Disabled;
    int         depthBias               = 0;
    float       depthBiasClamp          = 0.0f;
    float       slopeScaledDepthBias    = 0.0f;
    bool        frontCCW                = false;
    bool        depthClampEnabled       = false;
    bool        scissorTestEnabled      = false;
    bool        multiSampleEnabled      = false;
    bool        antiAliasedLineEnabled  = false;
};

struct BlendTargetDescriptor
{
    BlendOp     srcColor    = BlendOp::SrcAlpha;
    BlendOp     destColor   = BlendOp::InvSrcAlpha;
    BlendOp     srcAlpha    = BlendOp::SrcAlpha;
    BlendOp     destAlpha   = BlendOp::InvSrcAlpha;
    ColorRGBAb  colorMask;
};

struct BlendDescriptor
{
    bool                                blendEnabled    = false;
    std::vector<BlendTargetDescriptor>  targets;
};

/**
\brief Graphics pipeline descriptor structure.
\remarks This structure describes the entire graphics pipeline:
viewports, depth-/ stencil-/ rasterizer-/ blend states, shader stages etc.
*/
struct GraphicsPipelineDescriptor
{
    DepthDescriptor         depth;
    StencilDescriptor       stencil;
    RasterizerDescriptor    rasterizer;
    BlendDescriptor         blend;

    ShaderProgram*          shaderProgram   = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
