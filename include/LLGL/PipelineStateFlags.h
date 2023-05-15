/*
 * PipelineStateFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PIPELINE_STATE_FLAGS_H
#define LLGL_PIPELINE_STATE_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Types.h>
#include <LLGL/Format.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/StaticLimits.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Primitive topology enumeration.
\remarks All strip topologies (i.e. PrimitiveTopology::LineStrip, PrimitiveTopology::LineStripAdjacency,
PrimitiveTopology::TriangleStrip, and PrimitiveTopology::TriangleStripAdjacency) use a fixed index value to restart the primitives.
This fixed index value is the maximum possible value for the respective index buffer format, i.e. <code>2^16-1</code> (or \c 0xFFFF) for Format::R16UInt and <code>2^32-1</code> (or \c 0xFFFFFFFF) for Format::R32UInt.
\todo For Direct3D 12, the restart primitive index value (aka. strip cut value) can only be \c 0xFFFFFFFF at the moment as this needs to be specified at PSO creation time.
\see GraphicsPipelineDescriptor::primitiveTopology
*/
enum class PrimitiveTopology
{
    //! Point list, where each vertex represents a single point primitive.
    PointList,

    //! Line list, where each pair of two vertices represetns a single line primitive.
    LineList,

    //! Line strip, where each vertex generates a new line primitive while the previous vertex is used as line start.
    LineStrip,

    /**
    \brief Adjacency line list, which is similar to LineList but each end point has a corresponding adjacent vertex that is accessible in a geometry shader.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    */
    LineListAdjacency,

    /**
    \brief Adjacency line strip, which is similar to LineStrip but each end point has a corresponding adjacent vertex that is accessible in a geometry shader.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    */
    LineStripAdjacency,

    //! Triangle list, where each set of three vertices represent a single triangle primitive.
    TriangleList,

    //! Triangle strip, where each vertex generates a new triangle primitive with an alternative triangle winding.
    TriangleStrip,

    /**
    \brief Adjacency triangle list, which is similar to TriangleList but each triangle edge has a corresponding adjacent vertex that is accessible in a geometry shader.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    */
    TriangleListAdjacency,

    /**
    \brief Adjacency triangle strips which is similar to TriangleStrip but each triangle edge has a corresponding adjacent vertex that is accessible in a geometry shader.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    */
    TriangleStripAdjacency,

    Patches1,               //!< Patches with 1 control point that is accessible in a tessellation shader.
    Patches2,               //!< Patches with 2 control points that are accessible in a tessellation shader.
    Patches3,               //!< Patches with 3 control points that are accessible in a tessellation shader.
    Patches4,               //!< Patches with 4 control points that are accessible in a tessellation shader.
    Patches5,               //!< Patches with 5 control points that are accessible in a tessellation shader.
    Patches6,               //!< Patches with 6 control points that are accessible in a tessellation shader.
    Patches7,               //!< Patches with 7 control points that are accessible in a tessellation shader.
    Patches8,               //!< Patches with 8 control points that are accessible in a tessellation shader.
    Patches9,               //!< Patches with 9 control points that are accessible in a tessellation shader.
    Patches10,              //!< Patches with 10 control points that are accessible in a tessellation shader.
    Patches11,              //!< Patches with 11 control points that are accessible in a tessellation shader.
    Patches12,              //!< Patches with 12 control points that are accessible in a tessellation shader.
    Patches13,              //!< Patches with 13 control points that are accessible in a tessellation shader.
    Patches14,              //!< Patches with 14 control points that are accessible in a tessellation shader.
    Patches15,              //!< Patches with 15 control points that are accessible in a tessellation shader.
    Patches16,              //!< Patches with 16 control points that are accessible in a tessellation shader.
    Patches17,              //!< Patches with 17 control points that are accessible in a tessellation shader.
    Patches18,              //!< Patches with 18 control points that are accessible in a tessellation shader.
    Patches19,              //!< Patches with 19 control points that are accessible in a tessellation shader.
    Patches20,              //!< Patches with 20 control points that are accessible in a tessellation shader.
    Patches21,              //!< Patches with 21 control points that are accessible in a tessellation shader.
    Patches22,              //!< Patches with 22 control points that are accessible in a tessellation shader.
    Patches23,              //!< Patches with 23 control points that are accessible in a tessellation shader.
    Patches24,              //!< Patches with 24 control points that are accessible in a tessellation shader.
    Patches25,              //!< Patches with 25 control points that are accessible in a tessellation shader.
    Patches26,              //!< Patches with 26 control points that are accessible in a tessellation shader.
    Patches27,              //!< Patches with 27 control points that are accessible in a tessellation shader.
    Patches28,              //!< Patches with 28 control points that are accessible in a tessellation shader.
    Patches29,              //!< Patches with 29 control points that are accessible in a tessellation shader.
    Patches30,              //!< Patches with 30 control points that are accessible in a tessellation shader.
    Patches31,              //!< Patches with 31 control points that are accessible in a tessellation shader.
    Patches32,              //!< Patches with 32 control points that are accessible in a tessellation shader.
};

/**
\brief Compare operations enumeration.
\remarks This operation is used for depth tests, stencil tests, and texture sample comparisons.
\see DepthDescriptor::compareOp
\see StencilFaceDescriptor::compareOp
\see SamplerDescriptor::compareOp
*/
enum class CompareOp
{
    NeverPass,      //!< Comparison never passes.
    Less,           //!< Comparison passes if the source data is less than the destination data.
    Equal,          //!< Comparison passes if the source data is euqal to the right-hand-side.
    LessEqual,      //!< Comparison passes if the source data is less than or equal to the right-hand-side.
    Greater,        //!< Comparison passes if the source data is greater than the right-hand-side.
    NotEqual,       //!< Comparison passes if the source data is not equal to the right-hand-side.
    GreaterEqual,   //!< Comparison passes if the source data is greater than or equal to the right-hand-side.
    AlwaysPass,     //!< Comparison always passes.
};

/**
\brief Stencil operations enumeration.
\see StencilFaceDescriptor
*/
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

/**
\brief Blending operations enumeration.
\see BlendTargetDescriptor
*/
enum class BlendOp
{
    Zero,               //!< Data source is the color black (0, 0, 0, 0).
    One,                //!< Data source is the color white (1, 1, 1, 1).
    SrcColor,           //!< Data source is color data (RGB) from a fragment shader.
    InvSrcColor,        //!< Data source is inverted color data (1 - RGB) from a fragment shader.
    SrcAlpha,           //!< Data source is alpha data (A) from a fragment shader.
    InvSrcAlpha,        //!< Data source is inverted alpha data (1 - A) from a fragment shader.
    DstColor,           //!< Data source is color data (RGB) from a framebuffer.
    InvDstColor,        //!< Data source is inverted color data (1 - RGB) from a framebuffer.
    DstAlpha,           //!< Data source is alpha data (A) from a framebuffer.
    InvDstAlpha,        //!< Data source is inverted alpha data (1 - A) from a framebuffer.
    SrcAlphaSaturate,   //!< Data source is alpha data (A) from a fragment shader which is clamped to 1 or less.
    BlendFactor,        //!< Data source is the blend factor (RGBA) from the blend state. \see CommandBuffer::SetBlendFactor
    InvBlendFactor,     //!< Data source is the inverted blend factor (1 - RGBA) from the blend state. \see CommandBuffer::SetBlendFactor
    Src1Color,          //!< Data sources are both color data (RGB) from a fragment shader with dual-source color blending.
    InvSrc1Color,       //!< Data sources are both inverted color data (1 - RGB) from a fragment shader with dual-source color blending.
    Src1Alpha,          //!< Data sources are both alpha data (A) from a fragment shader with dual-source color blending.
    InvSrc1Alpha        //!< Data sources are both inverted alpha data (1 - A) from a fragment shader with dual-source color blending.
};

/**
\brief Blending arithmetic operations enumeration.
\see BlendTargetDescriptor::colorArithmetic
\see BlendTargetDescriptor::alphaArithmetic
*/
enum class BlendArithmetic
{
    Add,            //!< Add source 1 and source 2. This is the default for all renderers.
    Subtract,       //!< Subtract source 1 from source 2.
    RevSubtract,    //!< Subtract source 2 from source 1.
    Min,            //!< Find the minimum of source 1 and source 2.
    Max,            //!< Find the maximum of source 1 and source 2.
};

/**
\brief Polygon filling modes enumeration.
\see RasterizerDescriptor::polygonMode
*/
enum class PolygonMode
{
    Fill,       //!< Draw filled polygon.
    Wireframe,  //!< Draw triangle edges only.
    /**
    \brief Draw vertex points only.
    \note Only supported with: OpenGL, Vulkan.
    */
    Points,
};

/**
\brief Polygon culling modes enumeration.
\see RasterizerDescriptor::cullMode
*/
enum class CullMode
{
    Disabled,   //!< No culling.
    Front,      //!< Front face culling.
    Back,       //!< Back face culling.
};

/**
\brief Logical pixel operation enumeration.
\remarks These logical pixel operations are bitwise operations.
In the following documentation, \c src denotes the source color and \c dst denotes the destination color.
\note Only supported with: OpenGL, Vulkan, Direct3D 11.1+, Direct3D 12.0.
\see BlendDescriptor::logicOp
*/
enum class LogicOp
{
    Disabled,       //!< No logical pixel operation.
    Clear,          //!< Resulting operation: <code>0</code>.
    Set,            //!< Resulting operation: <code>1</code>.
    Copy,           //!< Resulting operation: <code>src</code>.
    CopyInverted,   //!< Resulting operation: <code>~src</code>.
    NoOp,           //!< Resulting operation: <code>dst</code>.
    Invert,         //!< Resulting operation: <code>~dst</code>.
    AND,            //!< Resulting operation: <code>src & dst</code>.
    ANDReverse,     //!< Resulting operation: <code>src & ~dst</code>.
    ANDInverted,    //!< Resulting operation: <code>~src & dst</code>.
    NAND,           //!< Resulting operation: <code>~(src & dst)</code>.
    OR,             //!< Resulting operation: <code>src | dst</code>.
    ORReverse,      //!< Resulting operation: <code>src | ~dst</code>.
    ORInverted,     //!< Resulting operation: <code>~src | dst</code>.
    NOR,            //!< Resulting operation: <code>~(src | dst)</code>.
    XOR,            //!< Resulting operation: <code>src ^ dst</code>.
    Equiv,          //!< Resulting operation: <code>~(src ^ dst)</code>.
};

/**
\brief Tessellation partition mode enumeration.
\see TessellationDescriptor::partition
*/
enum class TessellationPartition
{
    //! Undefined partition mode.
    Undefined,

    /**
    \brief Integer with integers only.
    \remarks Equivalent of <code>[partitioning("integer")]</code> in HLSL and <code>layout(equal_spacing)</code> in GLSL.
    */
    Integer,

    /**
    \brief Partition with power-of-two number only.
    \remarks Equivalent of <code>[partitioning("pow2")]</code> in HLSL.
    */
    Pow2,

    /**
    \brief Partition with an odd, fractional number.
    \remarks Equivalent of <code>[partitioning("fractional_odd")]</code> in HLSL and <code>layout(fractional_odd_spacing)</code> in GLSL.
    */
    FractionalOdd,

    /**
    \brief Partition with an even, fractional number.
    \remarks Equivalent of <code>[partitioning("fractional_even")]</code> in HLSL and <code>layout(fractional_even_spacing)</code> in GLSL.
    */
    FractionalEven,
};


/* ----- Flags ----- */

/**
\brief Blend target color mask flags.
\see BlendTargetDescriptor::colorMask
*/
struct ColorMaskFlags
{
    enum : std::uint8_t
    {
        Zero    = 0,                //!< No color mask. Use this to disable rasterizer output.
        R       = (1 << 0),         //!< Bitmask for the red channel. Value is \c 0x1.
        G       = (1 << 1),         //!< Bitmask for the green channel. Value is \c 0x2.
        B       = (1 << 2),         //!< Bitmask for the blue channel. Value is \c 0x4.
        A       = (1 << 3),         //!< Bitmask for the alpha channel. Value is \c 0x8.
        All     = (R | G | B | A),  //!< Bitwise OR combination of all color component bitmasks.
    };
};


/* ----- Structures ----- */

/**
\brief Viewport dimensions.
\remarks A viewport is in screen coordinates where the origin is in the left-top corner.
\see CommandBuffer::SetViewport
\see CommandBuffer::SetViewports
\see GraphicsPipelineDescriptor::viewports
*/
struct Viewport
{
    Viewport() = default;
    Viewport(const Viewport&) = default;

    //! Viewport constructor with default depth range of [0, 1].
    inline Viewport(float x, float y, float width, float height) :
        x      { x      },
        y      { y      },
        width  { width  },
        height { height }
    {
    }

    //! Viewport constructor with parameters for all attributes.
    inline Viewport(float x, float y, float width, float height, float minDepth, float maxDepth) :
        x        { x        },
        y        { y        },
        width    { width    },
        height   { height   },
        minDepth { minDepth },
        maxDepth { maxDepth }
    {
    }

    //! Viewport constructor with extent and default depth range of [0, 1].
    inline Viewport(const Extent2D& extent) :
        width  { static_cast<float>(extent.width)  },
        height { static_cast<float>(extent.height) }
    {
    }

    //! Viewport constructor with extent and explicit depth range.
    inline Viewport(const Extent2D& extent, float minDepth, float maxDepth) :
        width    { static_cast<float>(extent.width)  },
        height   { static_cast<float>(extent.height) },
        minDepth { minDepth                          },
        maxDepth { maxDepth                          }
    {
    }

    //! Viewport constructor with offset, extent, and default depth range of [0, 1].
    inline Viewport(const Offset2D& offset, const Extent2D& extent) :
        x      { static_cast<float>(offset.x)      },
        y      { static_cast<float>(offset.y)      },
        width  { static_cast<float>(extent.width)  },
        height { static_cast<float>(extent.height) }
    {
    }

    //! Viewport constructor with offset, extent, and explicit depth range.
    inline Viewport(const Offset2D& offset, const Extent2D& extent, float minDepth, float maxDepth) :
        x        { static_cast<float>(offset.x)      },
        y        { static_cast<float>(offset.y)      },
        width    { static_cast<float>(extent.width)  },
        height   { static_cast<float>(extent.height) },
        minDepth { minDepth                          },
        maxDepth { maxDepth                          }
    {
    }

    //!< X coordinate of the left-top origin. By default 0.0.
    float x         = 0.0f;

    //!< Y coordinate of the left-top origin. By default 0.0.
    float y         = 0.0f;

    /**
    \brief Width of the right-bottom size. By default 0.0.
    \remarks Setting a viewport of negative width results in undefined behavior.
    */
    float width     = 0.0f;

    /**
    \brief Height of the right-bottom size. By default 0.0.
    \remarks Setting a viewport of negative height results in undefined behavior.
    */
    float height    = 0.0f;

    /**
    \brief Minimum of the depth range. Must be in the range [0, 1]. By default 0.0.
    \remarks Reverse mappings such as <code>minDepth=1</code> and <code>maxDepth=0</code> are also valid.
    */
    float minDepth  = 0.0f;

    /**
    \brief Maximum of the depth range. Must be in the range [0, 1]. By default 1.0.
    \remarks Reverse mappings such as <code>minDepth=1</code> and <code>maxDepth=0</code> are also valid.
    */
    float maxDepth  = 1.0f;
};

/**
\brief Scissor dimensions.
\remarks A scissor is in screen coordinates where the origin is in the left-top corner.
\see CommandBuffer::SetScissor
\see CommandBuffer::SetScissors
\see GraphicsPipelineDescriptor::scissors
*/
struct Scissor
{
    Scissor() = default;
    Scissor(const Scissor&) = default;

    //! Scissor constructor with parameters for all attributes.
    inline Scissor(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height) :
        x      { x      },
        y      { y      },
        width  { width  },
        height { height }
    {
    }

    //! Scissor constructor with offset and extent parameters.
    inline Scissor(const Offset2D& offset, const Extent2D& extent) :
        x      { offset.x                                 },
        y      { offset.y                                 },
        width  { static_cast<std::int32_t>(extent.width)  },
        height { static_cast<std::int32_t>(extent.height) }
    {
    }

    std::int32_t x       = 0; //!< Left-top X coordinate.
    std::int32_t y       = 0; //!< Left-top Y coordinate.
    std::int32_t width   = 0; //!< Right-bottom width.
    std::int32_t height  = 0; //!< Right-bottom height.
};

/**
\brief Depth state descriptor structure.
\see GraphicsPipelineDescriptor::depth
*/
struct DepthDescriptor
{
    //! Specifies whether the depth test is enabled or disabled. By default disabled.
    bool        testEnabled     = false;

    //! Specifies whether writing to the depth buffer is enabled or disabled. By default disabled.
    bool        writeEnabled    = false;

    //! Specifies the depth test comparison function. By default CompareOp::Less.
    CompareOp   compareOp       = CompareOp::Less;
};

/**
\brief Stencil face descriptor structure.
\see StencilDescriptor::front
\see StencilDescriptor::back
*/
struct StencilFaceDescriptor
{
    //! Specifies the operation to take when the stencil test fails. By default StencilOp::Keep.
    StencilOp       stencilFailOp   = StencilOp::Keep;

    //! Specifies the operation to take when the stencil test passes but the depth test fails. By default StencilOp::Keep.
    StencilOp       depthFailOp     = StencilOp::Keep;

    //! Specifies the operation to take when both the stencil test and the depth test pass. By default StencilOp::Keep.
    StencilOp       depthPassOp     = StencilOp::Keep;

    //! Specifies the stencil compare operation. By default CompareOp::Less.
    CompareOp       compareOp       = CompareOp::Less;

    /**
    \brief Specifies the portion of the depth-stencil buffer for reading stencil data. By default \c 0xFFFFFFFF.
    \note For Direct3D 11 and Direct3D 12, only the first 8 least significant bits (i.e. <code>readMask & 0xFF</code>) of the read mask value of the front face will be used.
    \see StencilDescriptor::front
    */
    std::uint32_t   readMask        = ~0u;

    /**
    \brief Specifies the portion of the depth-stencil buffer for writing stencil data. By default \c 0xFFFFFFFF.
    \note For Direct3D 11 and Direct3D 12, only the first 8 least significant bits (i.e. <code>writeMask & 0xFF</code>) of the write mask value of the front face will be used.
    \see StencilDescriptor::front
    */
    std::uint32_t   writeMask       = ~0u;

    /**
    \brief Specifies the stencil reference value. By default 0.
    \remarks This value will be used when the stencil operation is StencilOp::Replace.
    \remarks If \c referenceDynamic is set to true, this member is ignored.
    \note For Direct3D 11 and Direct3D 12, only the stencil reference value of the front face will be used.
    \see StencilDescriptor::front
    \see StencilDescriptor::referenceDynamic
    \see CommandBuffer::SetStencilReference
    */
    std::uint32_t   reference       = 0u;
};

/**
\brief Stencil state descriptor structure.
\see GraphicsPipelineDescriptor::stencil
*/
struct StencilDescriptor
{
    //! Specifies whether the stencil test is enabled or disabled. By default disabled.
    bool                    testEnabled         = false;

    /**
    \brief Specifies whether the stencil reference values will be set dynamically with the command buffer. By default false.
    \remarks If this is true, StencilFaceDescriptor::reference in \c front and \c back is ignored
    and the stencil reference values must be set with the \c SetStencilReference function everytime the graphics pipeline is set.
    \see CommandBuffer::SetStencilReference
    */
    bool                    referenceDynamic    = false;

    /**
    \brief Specifies the front face settings for the stencil test.
    \note For Direct3D 11 and Direct3D 12, the members \c readMask, \c writeMask, and \c reference are only supported for the front face.
    \see StencilFaceDescriptor::readMask
    \see StencilFaceDescriptor::writeMask
    \see StencilFaceDescriptor::reference
    */
    StencilFaceDescriptor   front;

    //! Specifies the back face settings for the stencil test.
    StencilFaceDescriptor   back;
};

//! Depth bias descriptor structure to control fragment depth values.
struct DepthBiasDescriptor
{
    /**
    \brief Specifies a scalar factor controlling the constant depth value added to each fragment. By default 0.0.
    \note The actual constant factor being added to each fragment is implementation dependent of the respective rendering API.
    Direct3D 12 for instance only considers the integral part.
    */
    float constantFactor    = 0.0f;

    //! Specifies a scalar factor applied to a fragment's slope in depth bias calculations. By default 0.0.
    float slopeFactor       = 0.0f;

    /**
    \brief Specifies the maximum (or minimum) depth bias of a fragment. By default 0.0.
    \note For OpenGL, this is only supported if the extension \c GL_ARB_polygon_offset_clamp is available
    \see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_polygon_offset_clamp.txt
    */
    float clamp             = 0.0f;
};

/**
\brief Rasterizer state descriptor structure.
\see GraphicsPipelineDescriptor::rasterizer
*/
struct RasterizerDescriptor
{
    //! Polygon render mode. By default PolygonMode::Fill.
    PolygonMode         polygonMode                 = PolygonMode::Fill;

    //! Polygon face culling mode. By default CullMode::Disabled.
    CullMode            cullMode                    = CullMode::Disabled;

    //! Specifies the parameters to bias fragment depth values.
    DepthBiasDescriptor depthBias;

    //! If enabled, front facing polygons are in counter-clock-wise winding, otherwise in clock-wise winding. By default disabled.
    bool                frontCCW                    = false;

    /**
    \brief If enabled, primitives are discarded after optional stream-outputs but before the rasterization stage. By default disabled.
    \note Only supported with: OpenGL, Vulkan, Metal.
    */
    bool                discardEnabled              = false;

    //! If enabled, there is effectively no near and far clipping plane. By default disabled.
    bool                depthClampEnabled           = false;

    /**
    \brief Specifies whether scissor test is enabled or disabled. By default disabled.
    \see CommandBuffer::SetScissor
    \see CommandBuffer::SetScissors
    */
    bool                scissorTestEnabled          = false;

    //! Specifies whether multi-sampling is enabled or disabled. By default disabled.
    bool                multiSampleEnabled          = false;

    //! Specifies whether lines are rendered with or without anti-aliasing. By default disabled.
    bool                antiAliasedLineEnabled      = false;

    /**
    \brief If true, conservative rasterization is enabled. By default disabled.
    \note Only supported with:
    - Direct3D 12
    - Direct3D 11.3
    - OpenGL (if the extension \c GL_NV_conservative_raster or \c GL_INTEL_conservative_rasterization is supported)
    - Vulkan (if the extension \c VK_EXT_conservative_rasterization is supported).
    \see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    \see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    \see https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VK_EXT_conservative_rasterization.html
    \see RenderingFeatures::hasConservativeRasterization
    */
    bool                conservativeRasterization   = false;

    /**
    \brief Specifies the width of all generated line primitives. By default 1.0.
    \remarks The minimum and maximum supported line width can be determined by the \c lineWidthRange member in the RenderingCapabilities structure.
    If this line width is out of range, it will be clamped silently during graphics pipeline creation.
    \note Only supported with: OpenGL, Vulkan.
    \see RenderingLimits::lineWidthRange
    */
    float               lineWidth                   = 1.0f;
};

/**
\brief Blend target state descriptor structure.
\see BlendDescriptor::targets
*/
struct BlendTargetDescriptor
{
    //! Specifies whether blending is enabled or disabled for the respective color attachment.
    bool            blendEnabled    = false;

    //! Source color blending operation. By default BlendOp::SrcAlpha.
    BlendOp         srcColor        = BlendOp::SrcAlpha;

    //! Destination color blending operation. By default BlendOp::InvSrcAlpha.
    BlendOp         dstColor        = BlendOp::InvSrcAlpha;

    //! Color blending arithmetic. By default BlendArithmetic::Add.
    BlendArithmetic colorArithmetic = BlendArithmetic::Add;

    //! Source alpha blending operation. By default BlendOp::SrcAlpha.
    BlendOp         srcAlpha        = BlendOp::SrcAlpha;

    //! Destination alpha blending operation. By default BlendOp::InvSrcAlpha.
    BlendOp         dstAlpha        = BlendOp::InvSrcAlpha;

    //! Alpha blending arithmetic. By default BlendArithmetic::Add.
    BlendArithmetic alphaArithmetic = BlendArithmetic::Add;

    /**
    \brief Specifies which color components are enabled for writing. By default LLGL::ColorMaskFlags::All to enable all components.
    \remarks If no pixel shader is used in the graphics pipeline,
    the color mask \b must be set to LLGL::ColorMaskFlags::Zero (or 0) to disable rasterizer output. Otherwise, the behavior is undefined.
    */
    std::uint8_t    colorMask       = LLGL::ColorMaskFlags::All;
};


/**
\brief Blending state descriptor structure.
\see GraphicsPipelineDescriptor::blend
*/
struct BlendDescriptor
{
    /**
    \brief Specifies whether to use alpha-to-coverage as a multi-sampling technique when setting a pixel to a render target. By default disabled.
    \remarks This is useful when multi-sampling is enabled and alpha tests are implemented in a fragment shader (e.g. to render fences, plants, or other transparent geometries).
    \see sampleMask
    */
    bool                    alphaToCoverageEnabled  = false;

    /**
    \brief Specifies whether to enable independent blending in simultaneous color attachments. By default false.
    \remarks If this is true, each color attachment has its own blending configuration described in the \c targets array.
    Otherwise, each color attachment uses the blending configuration described only by the first entry of the \c targets array,
    i.e. <code>targets[0]</code> and all remaining entries <code>targets[1..7]</code> are ignored.
    \see targets
    */
    bool                    independentBlendEnabled = false;

    /**
    \brief Specifies the sample bitmask if alpha coverage is enabled. By default \c 0xFFFFFFFF.
    \remarks If \c alphaToCoverageEnabled is false, this field is ignored.
    \see alphaToCoverageEnabled
    */
    std::uint32_t           sampleMask              = ~0u;

    /**
    \brief Specifies the logic fragment operation. By default LogicOp::Disabled.
    \remarks Logic pixel operations can not be used in combination with color and alpha blending.
    Therefore, if this is not LogicOp::Disabled, \c independentBlendEnabled must be false and \c blendEnabled of the first target must be false as well.
    If logic fragment operations are not supported by the renderer, this must be LogicOp::Disabled.
    \note For Direct3D 11, feature level 11.1 is required.
    \see blendEnabled
    \see RenderingFeatures::hasLogicOp
    */
    LogicOp                 logicOp                 = LogicOp::Disabled;

    /**
    \brief Specifies the blending color factor. By default (0, 0, 0, 0).
    \remarks This is only used if any blending operations of any blending target is either BlendOp::BlendFactor or BlendOp::InvBlendFactor.
    \remarks If \c blendFactorDynamic is set to true, this member is ignored.
    \see BlendOp::BlendFactor
    \see BlendOp::InvBlendFactor
    \see CommandBuffer::SetBlendFactor
    */
    float                   blendFactor[4]          = { 0.0f, 0.0f, 0.0f, 0.0f };

    /**
    \brief Specifies whether the blend factor will be set dynamically with the command buffer. By default false.
    \remarks If this is true, \c blendFactor is ignored
    and the blending factors must be set with the \c SetBlendFactor function everytime the graphics pipeline is set.
    \see CommandBuffer::SetBlendFactor
    */
    bool                    blendFactorDynamic      = false;

    /**
    \brief Render-target blend states for the respective color attachments. A maximum of 8 targets is supported.
    \remarks If \c independentBlendEnabled is set to \c false, only the first entry is used,
    i.e. <code>targets[0]</code> and all remaining entries <code>targets[1..7]</code> are ignored.
    \see independentBlendEnabled
    */
    BlendTargetDescriptor   targets[LLGL_MAX_NUM_COLOR_ATTACHMENTS];
};

/**
\brief Tessellation descriptor structure for the graphics pipeline.
\remarks This is only used for the Metal backend or shader reflection.
\see GraphicsPipelineDescriptor::tessellation
\see RenderingFeatures::hasTessellatorStage
*/
struct TessellationDescriptor
{
    //! Specifies the partition mode of the tessellator stage. By default TessellationPartition::Undefined.
    TessellationPartition   partition           = TessellationPartition::Undefined;

    /**
    \brief Specifies the index buffer format. By default
    \remarks If patches are rendered with an index buffer (i.e. \c DrawIndexed or \c DrawIndexedInstanced) this must be either Format::R16UInt or Format::R32UInt.
    \see CommandBuffer::DrawIndexed
    \see CommandBuffer::DrawIndexedInstanced
    */
    Format                  indexFormat         = Format::Undefined;

    /**
    \brief Specifies the maximum tessellation factor. By default 64.
    \remarks Depending on the partition mode, this value must be:
    - TessellationPartition::Integer: An odd or even number.
    - TessellationPartition::Pow2: A power of two.
    - TessellationPartition::FractionalOdd: An even number (same as <code>FractionalEven</code>).
    - TessellationPartition::FractionalEven: An even number (same as <code>FractionalOdd</code>).
    \remarks This value is automatically clamped to the maximum value allowed by the rendering API, e.g. 64 for macOS and 16 for iOS.
    \remarks Equivalent of <code>[maxtessfactor(64.0)]</code> in HLSL.
    \see RenderingLimits::maxTessFactor
    */
    std::uint32_t           maxTessFactor       = 64;

    /**
    \brief If enabled, the output topology is in counter-clockwise winding order. By default disabled.
    \remarks Equivalent of <code>[outputtopology("triangle_ccw")]</code> in HLSL and <code>layout(ccw)</code> in GLSL.
    \see RasterizerDescriptor::frontCCW
    */
    bool                    outputWindingCCW    = false;
};

/**
\brief Graphics pipeline state descriptor structure.
\remarks This structure describes the entire graphics pipeline:
shader stages, depth-/ stencil-/ rasterizer-/ blend states etc.
\see RenderSystem::CreatePipelineState
*/
struct GraphicsPipelineDescriptor
{
    /**
    \brief Specifies an optional pipeline layout for the graphics pipeline. By default null.
    \remarks This layout determines at which slots buffer resources will be bound.
    If this is null, a default layout will be used that is only compatible with graphics pipelines that have no binding points, i.e. no input/output buffers or textures.
    \see RenderSystem::CreatePipelineLayout
    */
    const PipelineLayout*   pipelineLayout          = nullptr;

    /**
    \brief Specifies an optional render pass. By default null.
    \remarks If this is null, the render pass of the SwapChain that was first created is used.
    This render pass must be compatible with the one passed to the CommandBuffer::BeginRenderPass function in which the graphics pipeline will be used.
    \see CommandBuffer::BeginRenderPass
    \see RenderSystem::CreateRenderPass
    */
    const RenderPass*       renderPass              = nullptr;

    /**
    \brief Specifies the vertex shader.
    \remarks Each graphics pipeline must have at least a vertex shader. Therefore, this must never be null when a graphics PSO is created.
    With OpenGL, this shader may also have a stream output.
    */
    Shader*                 vertexShader            = nullptr;

    /**
    \brief Specifies the tessellation-control shader (also referred to as "Hull Shader").
    \remarks If this is used, the counter part must also be specified, i.e. \c tessEvaluationShader.
    \see tessEvaluationShader
    */
    Shader*                 tessControlShader       = nullptr;

    /**
    \brief Specifies the tessellation-evaluation shader (also referred to as "Domain Shader").
    \remarks If this is used, the counter part must also be specified, i.e. \c tessControlShader.
    \see tessControlShader
    */
    Shader*                 tessEvaluationShader    = nullptr;

    /**
    \brief Specifies an optional geometry shader.
    \remarks This shader may also have a stream output.
    */
    Shader*                 geometryShader          = nullptr;

    /**
    \brief Specifies an optional fragment shader (also referred to as "Pixel Shader").
    \remarks If no fragment shader is specified, generated fragments are discarded by the output merger
    and only the stream-output functionality as well as depth writes are used by either the vertex or geometry shader.
    If a depth buffer is attached to the current render target, omitting the fragment shader can be utilized to render a standard shadow map.
    */
    Shader*                 fragmentShader          = nullptr;

    /**
    \brief Specifies the primitive topology and ordering of the primitive data. By default PrimitiveTopology::TriangleList.
    \see PrimitiveTopology
    */
    PrimitiveTopology       primitiveTopology       = PrimitiveTopology::TriangleList;

    /**
    \brief Specifies an optional list of static viewports. If empty, the viewports must be set dynamically with the command buffer.
    \remarks This list must have the same number of entries as \c scissors, unless one of the lists is empty.
    \see CommandBuffer::SetViewport
    \see CommandBuffer::SetViewports
    */
    std::vector<Viewport>   viewports;

    /**
    \brief Specifies an optional list of static scissor rectangles. If empty, the scissors must be set dynamically with the command buffer.
    \remarks This list must have the same number of entries as \c viewports, unless one of the lists is empty.
    \see CommandBuffer::SetScissor
    \see CommandBuffer::SetScissors
    */
    std::vector<Scissor>    scissors;

    //! Specifies the depth state for the depth-stencil stage.
    DepthDescriptor         depth;

    //! Specifies the stencil state for the depth-stencil stage.
    StencilDescriptor       stencil;

    //! Specifies the state for the rasterizer stage.
    RasterizerDescriptor    rasterizer;

    //! Specifies the state descriptor for the blend stage.
    BlendDescriptor         blend;

    /**
    \brief Specifies the tessellation pipeline state.
    \remarks This is only used to configure a few tessellation states on the CPU side for the Metal backend.
    All other backends ignore this member silently.
    \note Only supported with: Metal.
    */
    TessellationDescriptor  tessellation;
};

/**
\brief Compute pipeline state descriptor structure.
\see RenderSystem::CreatePipelineState
*/
struct ComputePipelineDescriptor
{
    /**
    \brief Pointer to an optional pipeline layout for the graphics pipeline.
    \remarks This layout determines at which slots buffer resources can be bound.
    This is ignored by render systems which do not support pipeline layouts.
    */
    const PipelineLayout*   pipelineLayout  = nullptr;

    /**
    \brief Specifies the compute shader.
    \remarks This must never be null when a compute PSO is created.
    */
    Shader*                 computeShader   = nullptr;
};


/* ----- Functions ----- */

//! Returns true if the specified primitive topology is a patch list.
LLGL_EXPORT bool IsPrimitiveTopologyPatches(const PrimitiveTopology primitiveTopology);

/**
\brief Returns true if the specified primitive topology is a strip that generates a new primitive with each new vertex.
\return True if \c primitiveTopology is equal to one of the following primitive topologies:
- PrimitiveTopology::LineStrip
- PrimitiveTopology::LineStripAdjacency
- PrimitiveTopology::TriangleStrip
- PrimitiveTopology::TriangleStripAdjacency
*/
LLGL_EXPORT bool IsPrimitiveTopologyStrip(const PrimitiveTopology primitiveTopology);

//! Returns the number of patch control points of the specified primitive topology (in range [1, 32]), or 0 if the topology is not a patch list.
LLGL_EXPORT std::uint32_t GetPrimitiveTopologyPatchSize(const PrimitiveTopology primitiveTopology);


} // /namespace LLGL


#endif



// ================================================================================
