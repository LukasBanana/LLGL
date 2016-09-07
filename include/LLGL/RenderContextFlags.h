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

//! Render context draw mode enumeration.
enum class DrawMode
{
    Points,                 //!< Point list (by default only one pixel).
    Lines,                  //!< Line list (by default with one pixel width).
    LineStrip,              //!< Line strip where each primitive after the first one begins with the previous vertex.
    LineLoop,               //!< Line loop which is similiar to line strip but the last primtive ends with the first vertex (Only supported for OpenGL).
    LinesAdjacency,         //!< Adjacency line list.
    LineStripAdjacency,     //!< Adjacency line strips.
    Triangles,              //!< Triangle list.
    TriangleStrip,          //!< Triangle strip where each primitive after the first one begins with the previous vertex.
    TriangleFan,            //!< Triangle fan where each primitive use the first vertex, the previous vertex and a new vertex (Only supported for OpenGL).
    TrianglesAdjacency,     //!< Adjacency triangle list.
    TriangleStripAdjacency, //!< Adjacency triangle strips.
    Patches,                //!< Patches (main type for tessellation).
};

/**
\brief Enumeration of all renderer info entries.
\see RenderContext::QueryRendererInfo
*/
enum class RendererInfo
{
    Version,
    Vendor,
    Hardware,
    ShadingLanguageVersion,
};



/* ----- Structures ----- */

//! Render context clear buffer flags.
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
\brief Low-level graphics API dependent state descriptor structure.
\see RenderContext::SetGraphicsAPIDependentState
*/
struct GraphicsAPIDependentStateDescriptor
{
    /**
    \briefs Specifies whether to invert the front face setting in the graphics pipeline state.
    \remarks If this is true, the front facing will be inverted everytime "BindGraphicsPipeline" is called.
    \note Only supported with: OpenGL.
    \see RasterizerDescriptor::frontCCW
    \see RenderContext::BindGraphicsPipeline
    */
    bool invertFrontFace = false;
};


} // /namespace LLGL


#endif



// ================================================================================
