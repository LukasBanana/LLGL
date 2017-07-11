/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_CONTEXT_FLAGS_H
#define LLGL_RENDER_CONTEXT_FLAGS_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Render condition mode enumeration.
\remarks The condition is determined by the type of the Query object.
\see RenderContext::BeginRenderCondition
*/
enum class RenderConditionMode
{
    Wait,                   //!< Wait until the occlusion query result is available, before conditional rendering begins.
    NoWait,                 //!< Do not wait until the occlusion query result is available, before conditional rendering begins.
    ByRegionWait,           //!< Similar to Wait, but the renderer may discard the results of commands for any framebuffer region that did not contribute to the occlusion query.
    ByRegionNoWait,         //!< Similar to NoWait, but the renderer may discard the results of commands for any framebuffer region that did not contribute to the occlusion query.
    WaitInverted,           //!< Same as Wait, but the condition is inverted.
    NoWaitInverted,         //!< Same as NoWait, but the condition is inverted.
    ByRegionWaitInverted,   //!< Same as ByRegionWait, but the condition is inverted.
    ByRegionNoWaitInverted, //!< Same as ByRegionNoWait, but the condition is inverted.
};

/**
\brief Logical pixel operation enumeration.
\remarks These logical pixel operations are bitwise operations.
\note Only supported with: OpenGL.
\see GraphicsAPIDependentStateDescriptor::StateOpenGLDescriptor::logicOp
\see https://www.opengl.org/sdk/docs/man/html/glLogicOp.xhtml
*/
enum class LogicOp
{
    /* Configuration entries */
    Keep,           //!< Keep previous logical pixel operation.
    Disabled,       //!< Logical pixel operation is disabled.

    /* Logical operation entries */
    Clear,          //!< Resulting operation: 0.
    Set,            //!< Resulting operation: 1.
    Copy,           //!< Resulting operation: src.
    InvertedCopy,   //!< Resulting operation: ~src.
    Noop,           //!< Resulting operation: dest.
    Invert,         //!< Resulting operation: ~dest.
    AND,            //!< Resulting operation: src & dest.
    NAND,           //!< Resulting operation: ~(src & dest).
    OR,             //!< Resulting operation: src | dest.
    NOR,            //!< Resulting operation: ~(src | dest).
    XOR,            //!< Resulting operation: src ^ dest.
    Equiv,          //!< Resulting operation: ~(src ^ dest).
    ReverseAND,     //!< Resulting operation: src & ~dest.
    InvertedAND,    //!< Resulting operation: ~src & dest.
    ReverseOR,      //!< Resulting operation: src | ~dest.
    InvertedOR,     //!< Resulting operation: ~src | dest.
};


/* ----- Structures ----- */

/**
\brief Command buffer clear flags.
\see CommandBuffer::Clear
*/
struct ClearFlags
{
    enum
    {
        Color           = (1 << 0),                     //!< Clears the color buffer.
        Depth           = (1 << 1),                     //!< Clears the depth buffer.
        Stencil         = (1 << 2),                     //!< Clears the stencil buffer.

        ColorDepth      = (Color | Depth),              //!< Clears the color and depth buffers.
        DepthStencil    = (Depth | Stencil),            //!< Clears the depth and stencil buffers.
        All             = (Color | Depth | Stencil),    //!< Clears the all buffers (color, depth, and stencil).
    };
};

/**
\brief Viewport dimensions.
\remarks A viewport is in screen coordinates where the origin is in the left-top corner.
*/
struct Viewport
{
    Viewport() = default;
    Viewport(const Viewport&) = default;
    
    inline Viewport(float x, float y, float width, float height) :
        x      { x      },
        y      { y      },
        width  { width  },
        height { height }
    {
    }
    
    inline Viewport(float x, float y, float width, float height, float minDepth, float maxDepth) :
        x        { x        },
        y        { y        },
        width    { width    },
        height   { height   },
        minDepth { minDepth },
        maxDepth { maxDepth }
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
    Scissor() = default;
    Scissor(const Scissor&) = default;

    inline Scissor(int x, int y, int width, int height) :
        x      { x      },
        y      { y      },
        width  { width  },
        height { height }
    {
    }

    int x       = 0;
    int y       = 0;
    int width   = 0;
    int height  = 0;
};

/**
\brief Low-level graphics API dependent state descriptor union.
\remarks This descriptor is used to compensate a few differences between OpenGL and Direct3D.
\see RenderContext::SetGraphicsAPIDependentState
*/
union GraphicsAPIDependentStateDescriptor
{
    GraphicsAPIDependentStateDescriptor()
    {
        stateOpenGL.screenSpaceOriginLowerLeft      = false;
        stateOpenGL.invertFrontFace                 = false;
        stateOpenGL.logicOp                         = LogicOp::Keep;
        stateOpenGL.lineWidth                       = 0.0f;

        stateDirect3D12.disableAutoStateSubmission  = false;
    }

    struct StateOpenGLDescriptor
    {
        /**
        \brief Specifies whether the screen-space origin is on the lower-left. By default false.
        \remarks If this is true, the viewports and scissor rectangles of OpenGL are NOT emulated to the upper-left,
        which is the default to be uniform with other rendering APIs such as Direct3D and Vulkan.
        */
        bool        screenSpaceOriginLowerLeft;

        /**
        \brief Specifies whether to invert front-facing. By default false.
        \remarks If this is true, the front facing (either GL_CW or GL_CCW) will be inverted,
        i.e. CCW becomes CW, and CW becomes CCW.
        */
        bool        invertFrontFace;

        /**
        \brief Specifies the logical pixel operation for drawing operations. By default LogicOp::Keep.
        \see https://www.opengl.org/sdk/docs/man/html/glLogicOp.xhtml
        */
        LogicOp     logicOp;

        /**
        \brief Specifies the width to rasterize lines. By default 0.
        \remarks If this is 0, the attribute is ignored and the current line width will not be changed.
        \see https://www.opengl.org/sdk/docs/man/html/glLineWidth.xhtml
        */
        float       lineWidth;
    }
    stateOpenGL;

    struct StateDirect3D12Descriptor
    {
        /**
        \brief Specifies whether persistent states are automatically submitted to the command buffer or not. By default false.
        \remarks If this is true, "SetViewport" or "SetViewportArray", and "SetScissor" or "SetScissorArray" of the CommandBuffer interface
        must be called every time after the command buffer has been submitted to the command queue (e.g. after the "RenderContext::Present" function has been called).
        \see CommandBuffer::SetViewport
        \see CommandBuffer::SetViewportArray
        \see CommandBuffer::SetScissor
        \see CommandBuffer::SetScissorArray
        */
        bool        disableAutoStateSubmission;
    }
    stateDirect3D12;
};


} // /namespace LLGL


#endif



// ================================================================================
