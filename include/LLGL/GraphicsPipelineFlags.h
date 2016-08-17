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


/**
\brief Viewport dimensions.
\remarks A viewport is in screen coordinates where the origin is in the left-top corner.
*/
struct Viewport
{
    Viewport() = default;
    Viewport(const Viewport&) = default;
    
    Viewport(float x, float y, float width, float height) :
        x       ( x      ),
        y       ( y      ),
        width   ( width  ),
        height  ( height )
    {
    }
    
    Viewport(float x, float y, float width, float height, float minDepth, float maxDepth) :
        x       ( x        ),
        y       ( y        ),
        width   ( width    ),
        height  ( height   ),
        minDepth( minDepth ),
        maxDepth( maxDepth )
    {
    }

    float x         = 0.0f; //!< Left-top X coordinate.
    float y         = 0.0f; //!< Left-top Y coordinate.
    float width     = 0.0f; //!< Right-bottom width.
    float height    = 0.0f; //!< Right-bottom height.
    float minDepth  = 0.0f; //!< Minimal depth range.
    float maxDepth  = 1.0f; //!< Maximal depth range.
};

/**
\brief Scissor dimensions.
\remarks A scissor is in screen coordinates where the origin is in the left-top corner.
*/
struct Scissor
{
    int x       = 0;
    int y       = 0;
    int width   = 0;
    int height  = 0;
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
