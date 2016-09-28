/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_FLAGS_H__
#define __LLGL_RENDER_CONTEXT_FLAGS_H__


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Renderer info enumeration.
\see RenderContext::QueryRendererInfo
*/
enum class RendererInfo
{
    Version,
    Vendor,
    Hardware,
    ShadingLanguageVersion,
};

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


/* ----- Structures ----- */

/**
\brief Render context clear buffer flags.
\see RenderContext::ClearBuffers
*/
struct ClearBuffersFlags
{
    enum
    {
        Color   = (1 << 0),
        Depth   = (1 << 1),
        Stencil = (1 << 2),
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
    Scissor() = default;
    Scissor(const Scissor&) = default;

    Scissor(int x, int y, int width, int height) :
        x       ( x      ),
        y       ( y      ),
        width   ( width  ),
        height  ( height )
    {
    }

    int x       = 0;
    int y       = 0;
    int width   = 0;
    int height  = 0;
};

/**
\brief Low-level graphics API dependent state descriptor union.
\see RenderContext::SetGraphicsAPIDependentState
*/
union GraphicsAPIDependentStateDescriptor
{
    GraphicsAPIDependentStateDescriptor()
    {
        stateOpenGL.screenSpaceOriginLowerLeft  = false;
        stateOpenGL.invertFrontFace             = false;
    }

    struct StateOpenGLDescriptor
    {
        /**
        \brief Specifies whether the screen-space origin is on the lower-left. By default false.
        \remarks If this is true, the viewports and scissor rectangles of OpenGL are NOT emulated to the upper-left,
        which is the default to be uniform with other rendering APIs such as Direct3D and Vulkan.
        */
        bool screenSpaceOriginLowerLeft;

        /**
        \brief Specifies whether to invert front-facing. By default false.
        \remarks If this is true, the front facing (either GL_CW or GL_CCW) will be inverted,
        i.e. CCW becomes CW, and CW becomes CCW.
        */
        bool invertFrontFace;
    }
    stateOpenGL;
};


} // /namespace LLGL


#endif



// ================================================================================
