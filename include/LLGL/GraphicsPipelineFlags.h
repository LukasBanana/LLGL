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

/**
\brief Blending arithmetic operations enumeration.
\remarks This is only available with Direct3D.
*/
enum class BlendArithmetic
{
    Add,            //!< Add source 1 and source 2. This is the default for all renderers.
    Subtract,       //!< Subtract source 1 from source 2.
    RevSubtract,    //!< Subtract source 2 from source 1.
    Min,            //!< Find the minimum of source 1 and source 2.
    Max,            //!< Find the maximum of source 1 and source 2.
};

//! Polygon filling modes enumeration.
enum class PolygonMode
{
    Fill,       //!< Draw filled polygon.
    Wireframe,  //!< Draw triangle edges only.
    Points,     //!< Draw vertex points only. This can only be used with OpenGL.
};

//! Polygon culling modes enumeration.
enum class CullMode
{
    Disabled,   //!< No culling.
    Front,      //!< Front face culling.
    Back,       //!< Back face culling.
};


/* ----- Structures ----- */

//! Depth state descriptor structure.
struct DepthDescriptor
{
    bool        testEnabled     = false;            //!< Specifies whether the depth test is enabled or disabled. By default disabled.
    bool        writeEnabled    = false;            //!< Specifies whether writing to the depth buffer is enabled or disabled. By default disabled.
    CompareOp   compareOp       = CompareOp::Less;  //!< Specifies the depth test comparison function. By default CompareOp::Less.
};

//! Stencil face descriptor structure.
struct StencilFaceDescriptor
{
    StencilOp       stencilFailOp   = StencilOp::Keep;  //!< Specifies the operation to take when the stencil test fails.
    StencilOp       depthFailOp     = StencilOp::Keep;  //!< Specifies the operation to take when the stencil test passes but the depth test fails.
    StencilOp       depthPassOp     = StencilOp::Keep;  //!< Specifies the operation to take when both the stencil test and the depth test pass.
    CompareOp       compareOp       = CompareOp::Less;  //!< Specifies the stencil compare operation.
    std::uint32_t   compareMask     = 0;
    std::uint32_t   writeMask       = 0;
    std::uint32_t   reference       = 0;
};

//! Stencil state descriptor structure.
struct StencilDescriptor
{
    bool                    testEnabled  = false;
    StencilFaceDescriptor   front;
    StencilFaceDescriptor   back;
};

struct RasterizerDescriptor
{
    PolygonMode     polygonMode                 = PolygonMode::Fill;
    CullMode        cullMode                    = CullMode::Disabled;
    int             depthBias                   = 0;
    float           depthBiasClamp              = 0.0f;
    float           slopeScaledDepthBias        = 0.0f;

    /**
    \brief Number of sample. This is only used for Direct3D when multi-sampling is enabled.
    \see multiSampleEnabled
    */
    unsigned int    samples                     = 1;

    //! If true, front facing polygons are in counter-clock-wise winding, otherwise in clock-wise winding.
    bool            frontCCW                    = false;

    bool            depthClampEnabled           = false;
    bool            scissorTestEnabled          = false;
    bool            multiSampleEnabled          = false;
    bool            antiAliasedLineEnabled      = false;

    //! If ture, conservative rasterization is enabled. This is only available with Direct3D 12.
    bool            conservativeRasterization   = false;
};

struct BlendTargetDescriptor
{
    BlendOp         srcColor        = BlendOp::SrcAlpha;    //!< Source color blending operation.
    BlendOp         destColor       = BlendOp::InvSrcAlpha; //!< Destination color blending operation.
    BlendArithmetic colorArithmetic = BlendArithmetic::Add; //!< Color blending arithmetic. Only available with Direct3D.
    BlendOp         srcAlpha        = BlendOp::SrcAlpha;    //!< Source alpha blending operation.
    BlendOp         destAlpha       = BlendOp::InvSrcAlpha; //!< Destination alpha blending operation.
    BlendArithmetic alphaArithmetic = BlendArithmetic::Add; //!< Alpha blending arithmetic. Only available with Direct3D.
    ColorRGBAb      colorMask;
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
