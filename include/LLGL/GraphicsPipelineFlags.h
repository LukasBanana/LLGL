/*
 * GraphicsPipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GRAPHICS_PIPELINE_FLAGS_H
#define LLGL_GRAPHICS_PIPELINE_FLAGS_H


#include "Export.h"
#include "ColorRGBA.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class ShaderProgram;


/* ----- Enumerations ----- */

/**
\brief Primitive type enumeration.
\remarks These entries are generic terms of a primitive topology.
\see CommandBuffer::BeginStreamOutput
*/
enum class PrimitiveType
{
    /**
    \brief Generic term for all point primitives.
    \remarks This term refers to the following primitive topologies:
    PrimitiveTopology::PointList.
    */
    Points,

    /**
    \brief Generic term for all line primitives.
    \remarks This term refers to the following primitive topologies:
    PrimitiveTopology::LineList, PrimitiveTopology::LineStrip, PrimitiveTopology::LineLoop,
    PrimitiveTopology::LineListAdjacency, and PrimitiveTopology::LineStripAdjacency.
    */
    Lines,

    /**
    \brief Generic term for all triangle primitives.
    \remarks This term refers to the following primitive topologies:
    PrimitiveTopology::TriangleList, PrimitiveTopology::TriangleStrip, PrimitiveTopology::TriangleFan,
    PrimitiveTopology::TriangleListAdjacency, and PrimitiveTopology::TriangleStripAdjacency.
    */
    Triangles,
};

/**
\brief Primitive topology enumeration.
\see GraphicsPipelineDescriptor::primitiveTopology
*/
enum class PrimitiveTopology
{
    PointList,              //!< Point list.

    LineList,               //!< Line list where each line has its own two vertices.
    LineStrip,              //!< Line strip where each line after the first one begins with the previous vertex.

    /**
    \brief Line loop which is similiar to line strip but the last line ends with the first vertex.
    \note Only supported with: OpenGL.
    */
    LineLoop,

    LineListAdjacency,      //!< Adjacency line list.
    LineStripAdjacency,     //!< Adjacency line strips.

    TriangleList,           //!< Triangle list where each triangle has its own three vertices.
    TriangleStrip,          //!< Triangle strip where each triangle after the first one begins with the previous vertex.

    /**
    \brief Triangle fan where each triangle uses the first vertex, the previous vertex, and a new vertex.
    \note Only supported with: OpenGL.
    */
    TriangleFan,

    TriangleListAdjacency,  //!< Adjacency triangle list.
    TriangleStripAdjacency, //!< Adjacency triangle strips.

    Patches1,               //!< Patches with 1 control point.
    Patches2,               //!< Patches with 2 control points.
    Patches3,               //!< Patches with 3 control points.
    Patches4,               //!< Patches with 4 control points.
    Patches5,               //!< Patches with 5 control points.
    Patches6,               //!< Patches with 6 control points.
    Patches7,               //!< Patches with 7 control points.
    Patches8,               //!< Patches with 8 control points.
    Patches9,               //!< Patches with 9 control points.
    Patches10,              //!< Patches with 10 control points.
    Patches11,              //!< Patches with 11 control points.
    Patches12,              //!< Patches with 12 control points.
    Patches13,              //!< Patches with 13 control points.
    Patches14,              //!< Patches with 14 control points.
    Patches15,              //!< Patches with 15 control points.
    Patches16,              //!< Patches with 16 control points.
    Patches17,              //!< Patches with 17 control points.
    Patches18,              //!< Patches with 18 control points.
    Patches19,              //!< Patches with 19 control points.
    Patches20,              //!< Patches with 20 control points.
    Patches21,              //!< Patches with 21 control points.
    Patches22,              //!< Patches with 22 control points.
    Patches23,              //!< Patches with 23 control points.
    Patches24,              //!< Patches with 24 control points.
    Patches25,              //!< Patches with 25 control points.
    Patches26,              //!< Patches with 26 control points.
    Patches27,              //!< Patches with 27 control points.
    Patches28,              //!< Patches with 28 control points.
    Patches29,              //!< Patches with 29 control points.
    Patches30,              //!< Patches with 30 control points.
    Patches31,              //!< Patches with 31 control points.
    Patches32,              //!< Patches with 32 control points.
};

/**
\brief Compare operations enumeration.
\remarks This operation is used for depth-test and stencil-test.
*/
enum class CompareOp
{
    Never,          //!< Comparison never passes.
    Less,           //!< Comparison passes if the source data is less than the destination data.
    Equal,          //!< Comparison passes if the source data is euqal to the right-hand-side.
    LessEqual,      //!< Comparison passes if the source data is less than or equal to the right-hand-side.
    Greater,        //!< Comparison passes if the source data is greater than the right-hand-side.
    NotEqual,       //!< Comparison passes if the source data is not equal to the right-hand-side.
    GreaterEqual,   //!< Comparison passes if the source data is greater than or equal to the right-hand-side.
    Ever,           //!< Comparison always passes. (Can not be called "Always" due to conflict with X11 lib on Linux).
};

//! Stencil operations enumeration.
enum class StencilOp
{
    Keep,       //!< Keep the existing stencil data.
    Zero,       //!< Set stencil data to 0.
    Replace,    //!< Set the stencil data to the reference value. \see StencilFaceDescriptor::reference
    IncClamp,   //!< Increment the stencil value by 1, and clamp the result.
    DecClamp,   //!< Decrement the stencil value by 1, and clamp the result.
    Invert,     //!< Invert the stencil data.
    IncWrap,    //!< Increment the stencil value by 1, and wrap the result if necessary.
    DecWrap,    //!< Decrement the stencil value by 1, and wrap the result if necessary.
};

//! Blending operations enumeration.
enum class BlendOp
{
    Zero,               //!< Data source is the color black (0, 0, 0, 0).
    One,                //!< Data source is the color white (1, 1, 1, 1).
    SrcColor,           //!< Data source is color data (RGB) from a fragment shader.
    InvSrcColor,        //!< Data source is inverted color data (1 - RGB) from a fragment shader.
    SrcAlpha,           //!< Data source is alpha data (A) from a fragment shader.
    InvSrcAlpha,        //!< Data source is inverted alpha data (1 - A) from a fragment shader.
    DestColor,          //!< Data source is color data (RGB) from a framebuffer.
    InvDestColor,       //!< Data source is inverted color data (1 - RGB) from a framebuffer.
    DestAlpha,          //!< Data source is alpha data (A) from a framebuffer.
    InvDestAlpha,       //!< Data source is inverted alpha data (1 - A) from a framebuffer.
    SrcAlphaSaturate,   //!< Data source is alpha data (A) from a fragment shader which is clamped to 1 or less.
    BlendFactor,        //!< Data source is the blend factor (RGBA) from the blend state. \see BlendDescriptor::blendFactor
    InvBlendFactor,     //!< Data source is the inverted blend factor (1 - RGBA) from the blend state. \see BlendDescriptor::blendFactor
    Src1Color,          //!< Data sources are both color data (RGB) from a fragment shader with dual-source color blending.
    InvSrc1Color,       //!< Data sources are both inverted color data (1 - RGB) from a fragment shader with dual-source color blending.
    Src1Alpha,          //!< Data sources are both alpha data (A) from a fragment shader with dual-source color blending.
    InvSrc1Alpha        //!< Data sources are both inverted alpha data (1 - A) from a fragment shader with dual-source color blending.
};

//! Blending arithmetic operations enumeration.
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
    /**
    \brief Draw vertex points only.
    \note Only supported with: OpenGL.
    */
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

//! Multi-sampling descriptor structure.
struct MultiSamplingDescriptor
{
    MultiSamplingDescriptor() = default;

    inline MultiSamplingDescriptor(unsigned int samples) :
        enabled { samples > 1 },
        samples { samples     }
    {
    }

    /**
    \brief Returns the sample count for the state of this multi-sampling descriptor.
    \return max{ 1, samples } if multi-sampling is enabled, otherwise 1.
    */
    inline unsigned int SampleCount() const
    {
        return (enabled && samples > 1 ? samples : 1);
    }

    //! Specifies whether multi-sampling is enabled or disabled. By default disabled.
    bool            enabled = false;

    //! Number of samples used for multi-sampling. By default 1.
    unsigned int    samples = 1;
};

//! Depth state descriptor structure.
struct DepthDescriptor
{
    /**
    \brief Specifies whether the depth test is enabled or disabled. By default disabled.
    \remarks If no pixel shader is used in the graphics pipeline, the depth test must be disabled.
    */
    bool        testEnabled     = false;

    //! Specifies whether writing to the depth buffer is enabled or disabled. By default disabled.
    bool        writeEnabled    = false;

    //! Specifies the depth test comparison function. By default CompareOp::Less.
    CompareOp   compareOp       = CompareOp::Less;
};

//! Stencil face descriptor structure.
struct StencilFaceDescriptor
{
    //! Specifies the operation to take when the stencil test fails.
    StencilOp       stencilFailOp   = StencilOp::Keep;

    //! Specifies the operation to take when the stencil test passes but the depth test fails.
    StencilOp       depthFailOp     = StencilOp::Keep;

    //! Specifies the operation to take when both the stencil test and the depth test pass.
    StencilOp       depthPassOp     = StencilOp::Keep;

    //! Specifies the stencil compare operation.
    CompareOp       compareOp       = CompareOp::Less;

    /**
    \brief Specifies the portion of the depth-stencil buffer for reading stencil data. By default 0xffffffff.
    \note For Direct3D 11 and Direct3D 12, only the first 8 least significant bits (readMask & 0xff) of the read mask value of the front face will be used.
    \see StencilDescriptor::front
    */
    std::uint32_t   readMask        = ~0;

    /**
    \brief Specifies the portion of the depth-stencil buffer for writing stencil data. By default 0xffffffff.
    \note For Direct3D 11 and Direct3D 12, only the first 8 least significant bits (writeMask & 0xff) of the write mask value of the front face will be used.
    \see StencilDescriptor::front
    */
    std::uint32_t   writeMask       = ~0;

    /**
    \brief Specifies the stencil reference value.
    \remarks This value will be used when the stencil operation is StencilOp::Replace.
    \note For Direct3D 11 and Direct3D 12, only the stencil reference value of the front face will be used.
    \see StencilDescriptor::front
    */
    std::uint32_t   reference       = 0;
};

//! Stencil state descriptor structure.
struct StencilDescriptor
{
    /**
    \brief Specifies whether the stencil test is enabled or disabled.
    \remarks If no pixel shader is used in the graphics pipeline, the stencil test must be disabled.
    */
    bool                    testEnabled  = false;

    //! Specifies the front face settings for the stencil test.
    StencilFaceDescriptor   front;

    //! Specifies the back face settings for the stencil test.
    StencilFaceDescriptor   back;
};

//! Rasterizer state descriptor structure.
struct RasterizerDescriptor
{
    //! Polygon render mode. By default PolygonMode::Fill.
    PolygonMode             polygonMode                 = PolygonMode::Fill;

    //! Polygon face culling mode. By default CullMode::Disabled.
    CullMode                cullMode                    = CullMode::Disabled;
    int                     depthBias                   = 0;
    float                   depthBiasClamp              = 0.0f;
    float                   slopeScaledDepthBias        = 0.0f;

    //! (Multi-)sampling descriptor.
    MultiSamplingDescriptor multiSampling;

    //! If enabled, front facing polygons are in counter-clock-wise winding, otherwise in clock-wise winding.
    bool                    frontCCW                    = false;

    //! If enabled, there is effectively no near and far clipping plane. By default disabled.
    bool                    depthClampEnabled           = false;

    //! Specifies whether scissor test is enabled or disabled. By default disabled.
    bool                    scissorTestEnabled          = false;

    //! Specifies whether lines are rendered with or without anti-aliasing. By default disabled.
    bool                    antiAliasedLineEnabled      = false;

    /**
    \brief If ture, conservative rasterization is enabled.
    \note Only supported with: Direct3D 12 (or OpenGL if the extension "GL_NV_conservative_raster" or "GL_INTEL_conservative_rasterization" is supported).
    \see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    \see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    */
    bool                    conservativeRasterization   = false;
};

//! Blend target state descriptor structure.
struct BlendTargetDescriptor
{
    //! Source color blending operation. By default BlendOp::SrcAlpha.
    BlendOp         srcColor        = BlendOp::SrcAlpha;

    //! Destination color blending operation. By default BlendOp::InvSrcAlpha.
    BlendOp         destColor       = BlendOp::InvSrcAlpha;
    
    //! Color blending arithmetic. By default BlendArithmetic::Add.
    BlendArithmetic colorArithmetic = BlendArithmetic::Add;
    
    //! Source alpha blending operation. By default BlendOp::SrcAlpha.
    BlendOp         srcAlpha        = BlendOp::SrcAlpha;

    //! Destination alpha blending operation. By default BlendOp::InvSrcAlpha.
    BlendOp         destAlpha       = BlendOp::InvSrcAlpha;

    //! Alpha blending arithmetic. By default BlendArithmetic::Add.
    BlendArithmetic alphaArithmetic = BlendArithmetic::Add;

    //! Specifies which color components are enabled for writing. By default (true, true, true, true).
    ColorRGBAb      colorMask;
};

//! Blending state descriptor structure.
struct BlendDescriptor
{
    //! Specifies whether blending is enabled or disabled. This applies to all blending targets.
    bool                                blendEnabled    = false;

    /**
    \brief Specifies the blending color factor. By default (0, 0, 0, 0).
    \remarks This is only used if any blending operations of any blending target is either BlendOp::BlendFactor or BlendOp::InvBlendFactor.
    \see BlendOp::BlendFactor
    \see BlendOp::InvBlendFactor
    */
    ColorRGBAf                          blendFactor { 0.0f, 0.0f, 0.0f, 0.0f };

    //! Render-target blend states. A maximum of 8 targets is supported. Further targets will be ignored.
    std::vector<BlendTargetDescriptor>  targets;
};

/**
\brief Graphics pipeline descriptor structure.
\remarks This structure describes the entire graphics pipeline:
viewports, depth-/ stencil-/ rasterizer-/ blend states, shader stages etc.
*/
struct GraphicsPipelineDescriptor
{
    /**
    \brief Pointer to the shader program for the graphics pipeline.
    \remarks This must never be null when "RenderSystem::CreateGraphicsPipeline" is called with this structure.
    \see RenderSystem::CreateGraphicsPipeline
    \see RenderSystem::CreateShaderProgram
    */
    ShaderProgram*          shaderProgram       = nullptr;

    /**
    \brief Specifies the primitive topology and ordering of the primitive data. By default PrimitiveTopology::TriangleList.
    \see PrimitiveTopology
    */
    PrimitiveTopology       primitiveTopology   = PrimitiveTopology::TriangleList;

    //! Specifies the depth state descriptor.
    DepthDescriptor         depth;

    //! Specifies the stencil state descriptor.
    StencilDescriptor       stencil;

    //! Specifies the rasterizer state descriptor.
    RasterizerDescriptor    rasterizer;

    //! Specifies the blending state descriptor.
    BlendDescriptor         blend;
};


} // /namespace LLGL


#endif



// ================================================================================
