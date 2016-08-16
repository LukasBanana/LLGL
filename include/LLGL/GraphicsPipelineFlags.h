/*
 * GraphicsPipelineFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GRAPHICS_PIPELINE_FLAGS_H__
#define __LLGL_GRAPHICS_PIPELINE_FLAGS_H__


#include "Export.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


class ShaderProgram;
class VertexBuffer;
class IndexBuffer;


/**
\brief Compare operations enumeration.
\remarks This operation is used for depth-test and stencil-test.
*/
enum class CompareOp
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
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


/**
\brief Viewport dimensions.
\remarks A viewport is in screen coordinates where the origin is in the left-top corner.
*/
struct Viewport
{
    int             x           = 0;    //!< Left-top X coordinate.
    int             y           = 0;    //!< Left-top Y coordinate.
    unsigned int    width       = 0;    //!< Right-bottom width.
    unsigned int    height      = 0;    //!< Right-bottom height.
    float           minDepth    = 0.0f; //!< Minimal depth range.
    float           maxDepth    = 1.0f; //!< Maximal depth range.
};

/**
\brief Scissor dimensions.
\remarks A scissor is in screen coordinates where the origin is in the left-top corner.
*/
struct Scissor
{
    int             x       = 0;
    int             y       = 0;
    unsigned int    width   = 0;
    unsigned int    height  = 0;
};

struct DepthDescriptor
{
    bool        testEnabled     = false;
    bool        writeEnabled    = false;
    bool        rangeEnabled    = false;
    CompareOp   compareOp       = CompareOp::Less;
};

struct StencilStateDescriptor
{
    StencilOp       failOp      = StencilOp::Keep;
    StencilOp       passOp      = StencilOp::Keep;
    StencilOp       depthFailOp = StencilOp::Keep;
    CompareOp       compareOp   = CompareOp::Less;
    std::uint32_t   compareMask = 0;
    std::uint32_t   writeMask   = 0;
    std::uint32_t   reference   = 0;
};

struct StencilDescriptor
{
    bool                    testEnabled  = false;
    StencilStateDescriptor  front;
    StencilStateDescriptor  back;
};

struct RasterizerDescriptor
{
    //todo...
};

struct BlendDescriptor
{
    //todo...
};

/**
\brief Graphics pipeline descriptor structure.
\remarks This structure describes the entire graphics pipeline:
viewports, depth-/ stencil-/ rasterizer-/ blend states, shader stages etc.
*/
struct GraphicsPipelineDescriptor
{
    std::vector<Viewport>   viewports;
    std::vector<Scissor>    scissors;

    DepthDescriptor         depth;
    StencilDescriptor       stencil;
    RasterizerDescriptor    rasterizer;
    BlendDescriptor         blend;

    ShaderProgram*          shaderProgram   = nullptr;
};


} // /namespace LLGL


#endif



// ================================================================================
