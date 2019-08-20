/*
 * GraphicsPipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GRAPHICS_PIPELINE_FLAGS_H
#define LLGL_GRAPHICS_PIPELINE_FLAGS_H


#include "Export.h"
#include "ColorRGBA.h"
#include "Types.h"
#include "ForwardDecls.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


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
    //! Point list, where each vertex represents a single point primitive.
    PointList,

    //! Line list, where each pair of two vertices represetns a single line primitive.
    LineList,

    //! Line strip, where each vertex generates a new line primitive while the previous vertex is used as line start.
    LineStrip,

    /**
    \brief Line loop, which is similar to LineStrip but the first and last vertices generate yet another line primitive.
    \note Only supported with: OpenGL.
    */
    LineLoop,

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
    \brief Triangle fan, where each vertex generates a new triangle primitive while all share the same first vertex.
    \note Only supported with: OpenGL, Vulkan.
    */
    TriangleFan,

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
    \remarks Reverse mappings such as minDepth=1 and maxDepth=0 are also valid.
    */
    float minDepth  = 0.0f;

    /**
    \brief Maximum of the depth range. Must be in the range [0, 1]. By default 1.0.
    \remarks Reverse mappings such as minDepth=1 and maxDepth=0 are also valid.
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
\brief Multi-sampling descriptor structure.
\see RasterizerDescriptor::multiSampling
*/
struct MultiSamplingDescriptor
{
    MultiSamplingDescriptor() = default;

    /**
    \brief Constructor to initialize the sample.
    \param[in] samples Specifies the number of samples used for multi-sampling. If this is greater than 1, multi-sampling is enabled.
    \param[in] sampleMask Specifies the bitmask for sample coverage.
    */
    inline MultiSamplingDescriptor(std::uint32_t samples, std::uint32_t sampleMask = ~0) :
        enabled    { (samples > 1) },
        samples    { samples       },
        sampleMask { sampleMask    }
    {
    }

    /**
    \brief Returns the sample count for the state of this multi-sampling descriptor.
    \return <code>max{ 1, samples }</code> if multi-sampling is enabled, otherwise 1.
    */
    inline std::uint32_t SampleCount() const
    {
        return (enabled && samples > 1 ? samples : 1);
    }

    //! Specifies whether multi-sampling is enabled or disabled. By default disabled.
    bool            enabled     = false;

    /**
    \brief Number of samples used for multi-sampling. By default 1.
    \remarks The equivalent member for multi-sampled textures is TextureDescriptor::samples.
    \see TextureDescriptor::samples
    */
    std::uint32_t   samples     = 1;

    //! Specifies the bitmask for sample coverage. By default \c 0xFFFFFFFF.
    std::uint32_t   sampleMask  = ~0u;
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
    \note For Direct3D 11 and Direct3D 12, only the stencil reference value of the front face will be used.
    \see StencilDescriptor::front
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
    bool                    testEnabled  = false;

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
    (see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_polygon_offset_clamp.txt).
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
    PolygonMode             polygonMode                 = PolygonMode::Fill;

    //! Polygon face culling mode. By default CullMode::Disabled.
    CullMode                cullMode                    = CullMode::Disabled;

    //! Specifies the parameters to bias fragment depth values.
    DepthBiasDescriptor     depthBias;

    //! (Multi-)sampling descriptor.
    MultiSamplingDescriptor multiSampling;

    //! If enabled, front facing polygons are in counter-clock-wise winding, otherwise in clock-wise winding. By default disabled.
    bool                    frontCCW                    = false;

    /**
    \brief If enabled, primitives are discarded after optional stream-outputs but before the rasterization stage. By default disabled.
    \note Only supported with: OpenGL, Vulkan, Metal.
    */
    bool                    discardEnabled              = false;

    //! If enabled, there is effectively no near and far clipping plane. By default disabled.
    bool                    depthClampEnabled           = false;

    /**
    \brief Specifies whether scissor test is enabled or disabled. By default disabled.
    \see CommandBuffer::SetScissor
    \see CommandBuffer::SetScissors
    */
    bool                    scissorTestEnabled          = false;

    //! Specifies whether lines are rendered with or without anti-aliasing. By default disabled.
    bool                    antiAliasedLineEnabled      = false;

    /**
    \brief If true, conservative rasterization is enabled. By default disabled.
    \note Only supported with: Direct3D 12, Direct3D 11.3, OpenGL (if the extension \c GL_NV_conservative_raster or \c GL_INTEL_conservative_rasterization is supported).
    \see https://www.opengl.org/registry/specs/NV/conservative_raster.txt
    \see https://www.opengl.org/registry/specs/INTEL/conservative_rasterization.txt
    \see RenderingFeatures::hasConservativeRasterization
    */
    bool                    conservativeRasterization   = false;

    /**
    \brief Specifies the width of all generated line primitives. By default 1.0.
    \remarks The minimum and maximum supported line width can be determined by the \c lineWidthRange member in the RenderingCapabilities structure.
    If this line width is out of range, it will be clamped silently during graphics pipeline creation.
    \note Only supported with: OpenGL, Vulkan.
    \see RenderingLimits::lineWidthRange
    */
    float                   lineWidth                   = 1.0f;
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
    \brief Specifies which color components are enabled for writing. By default <code>(true, true, true, true)</code>.
    \remarks If no pixel shader is used in the graphics pipeline,
    the color mask \b must be set to \c false for all components. Otherwise, the behavior is undefined.
    */
    ColorRGBAb      colorMask       = { true, true, true, true };
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
    */
    bool                                alphaToCoverageEnabled  = false;

    /**
    \brief Specifies whether to enable independent blending in simultaneous color attachments. By default false.
    \remarks If this is true, each color attachment has its own blending configuration described in the \c targets array.
    Otherwise, each color attachment uses the blending configuration described only by the first entry of the \c targets array,
    i.e. <code>targets[0]</code> and all remaining entries <code>targets[1..7]</code> are ignored.
    \see targets
    */
    bool                                independentBlendEnabled = false;

    /**
    \brief Specifies the logic fragment operation. By default LogicOp::Disabled.
    \remarks Logic pixel operations can not be used in combination with color and alpha blending.
    Therefore, if this is not LogicOp::Disabled, \c independentBlendEnabled must be false and \c blendEnabled of the first target must be false as well.
    If logic fragment operations are not supported by the renderer, this must be LogicOp::Disabled.
    \note For Direct3D 11, feature level 11.1 is required.
    \see blendEnabled
    \see RenderingFeatures::hasLogicOp
    */
    LogicOp                             logicOp                 = LogicOp::Disabled;

    #if 1//TODO: replace this by CommandBuffer::SetBlendFactor
    /**
    \brief Specifies the blending color factor. By default (0, 0, 0, 0).
    \remarks This is only used if any blending operations of any blending target is either BlendOp::BlendFactor or BlendOp::InvBlendFactor.
    \see BlendOp::BlendFactor
    \see BlendOp::InvBlendFactor
    \todo Move this into a dynamic function "CommandBuffer::SetBlendFactor".
    */
    ColorRGBAf                          blendFactor             = { 0.0f, 0.0f, 0.0f, 0.0f };
    #endif

    /**
    \brief Render-target blend states for the respective color attachments. A maximum of 8 targets is supported.
    \remarks If \c independentBlendEnabled is set to \c false, only the first entry is used,
    i.e. <code>targets[0]</code> and all remaining entries <code>targets[1..7]</code> are ignored.
    \see independentBlendEnabled
    */
    BlendTargetDescriptor               targets[8];
};

/**
\brief Graphics pipeline descriptor structure.
\remarks This structure describes the entire graphics pipeline:
shader stages, depth-/ stencil-/ rasterizer-/ blend states etc.
\see RenderSystem::CreateGraphicsPipeline
*/
struct GraphicsPipelineDescriptor
{
    /**
    \brief Pointer to the shader program for the graphics pipeline. By default null.
    \remarks This must never be null when RenderSystem::CreateGraphicsPipeline is called with this structure.
    \see RenderSystem::CreateShaderProgram
    */
    const ShaderProgram*    shaderProgram       = nullptr;

    /**
    \brief Pointer to a RenderPass object. By default null.
    \remarks If this is null, the render pass of the RenderContext that was first created is used.
    This render pass must be compatible with the one passed to the CommandBuffer::BeginRenderPass function in which the graphics pipeline will be used.
    \see CommandBuffer::BeginRenderPass
    */
    const RenderPass*       renderPass          = nullptr;

    /**
    \brief Pointer to an optional pipeline layout for the graphics pipeline. By default null.
    \remarks This layout determines at which slots buffer resources can be bound.
    This is ignored by render systems which do not support pipeline layouts.
    */
    const PipelineLayout*   pipelineLayout      = nullptr;

    /**
    \brief Specifies the primitive topology and ordering of the primitive data. By default PrimitiveTopology::TriangleList.
    \see PrimitiveTopology
    */
    PrimitiveTopology       primitiveTopology   = PrimitiveTopology::TriangleList;

    /**
    \brief Specifies an optional list of viewports. If empty, the viewports must be set dynamically with the command buffer.
    \remarks This list must have the same number of entries as \c scissors, unless one of the lists is empty.
    \see CommandBuffer::SetViewport
    \see CommandBuffer::SetViewports
    */
    std::vector<Viewport>   viewports;

    /**
    \brief Specifies an optional list of scissor rectangles. If empty, the scissors must be set dynamically with the command buffer.
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
};


/* ----- Functions ----- */

//! Returns true if the specified primitive topology is a patch list.
LLGL_EXPORT bool IsPrimitiveTopologyPatches(const PrimitiveTopology primitiveTopology);

//! Returns the number of patch control points of the specified primitive topology (in range [1, 32]), or 0 if the topology is not a patch list.
LLGL_EXPORT std::uint32_t GetPrimitiveTopologyPatchSize(const PrimitiveTopology primitiveTopology);


} // /namespace LLGL


#endif



// ================================================================================
