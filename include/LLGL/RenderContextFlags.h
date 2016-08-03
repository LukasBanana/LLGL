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


} // /namespace LLGL


#endif



// ================================================================================
