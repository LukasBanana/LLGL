/*
 * RenderContextFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_CONTEXT_FLAGS_H__
#define __LLGL_RENDER_CONTEXT_FLAGS_H__


#include <Gauss/Vector3.h>


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

//! Shading language version enumation.
enum ShadingLanguage
{
    GLSL_110,   //!< GLSL 1.10 (since OpenGL 2.0).
    GLSL_120,   //!< GLSL 1.20 (since OpenGL 2.1).
    GLSL_130,   //!< GLSL 1.30 (since OpenGL 3.0).
    GLSL_140,   //!< GLSL 1.40 (since OpenGL 3.1).
    GLSL_150,   //!< GLSL 1.50 (since OpenGL 3.2).
    GLSL_330,   //!< GLSL 3.30 (since OpenGL 3.3).
    GLSL_400,   //!< GLSL 4.00 (since OpenGL 4.0).
    GLSL_410,   //!< GLSL 4.10 (since OpenGL 4.1).
    GLSL_420,   //!< GLSL 4.20 (since OpenGL 4.2).
    GLSL_430,   //!< GLSL 4.30 (since OpenGL 4.3).
    GLSL_440,   //!< GLSL 4.40 (since OpenGL 4.4).
    GLSL_450,   //!< GLSL 4.50 (since OpenGL 4.5).

    HLSL_2_0,   //!< HLSL 2.0 (since Direct3D 9).
    HLSL_2_0a,  //!< HLSL 2.0a (since Direct3D 9a).
    HLSL_2_0b,  //!< HLSL 2.0b (since Direct3D 9b).
    HLSL_3_0,   //!< HLSL 3.0 (since Direct3D 9c).
    HLSL_4_0,   //!< HLSL 4.0 (since Direct3D 10).
    HLSL_4_1,   //!< HLSL 4.1 (since Direct3D 10.1).
    HLSL_5_0,   //!< HLSL 5.0 (since Direct3D 11).
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

//! Rendering capabilities structure.
struct RenderingCaps
{
    //! Specifies whether render targets (also "frame buffer objects") are supported.
    bool            hasRenderTargets                = false;

    //! Specifies whether 3D textures are supported.
    bool            has3DTextures                   = false;

    //! Specifies whether cube textures are supported.
    bool            hasCubeTextures                 = false;

    //! Specifies whether 1D- and 2D array textures are supported.
    bool            hasTextureArrays                = false;

    //! Specifies whether cube array textures are supported.
    bool            hasCubeTextureArrays            = false;
    
    //! Specifies whether constant buffers (also "uniform buffer objects") are supported.
    bool            hasConstantBuffers              = false;

    //! Specifies whether storage buffers (also "read/write buffers") are supported.
    bool            hasStorageBuffers               = false;

    //! Specifies whether shader uniforms are supported.
    bool            hasUniforms                     = false;

    //! Specifies whether geometry shaders are supported.
    bool            hasGeometryShaders              = false;

    //! Specifies whether tessellation shaders are supported.
    bool            hasTessellationShaders          = false;

    //! Speciifes whether compute shaders are supported.
    bool            hasComputeShaders               = false;

    //! Specifies whether hardware instancing is supported.
    bool            hasInstancing                   = false;

    //! Specifies whether hardware instancing with instance offsets is supported.
    bool            hasOffsetInstancing             = false;

    //! Specifies whether multiple viewports, depth-ranges, and scissors are supported at once.
    bool            hasViewportArrays               = false;

    //! Specifies maximum number of texture array layers (for 1D-, 2D-, and cube textures).
    unsigned int    maxNumTextureArrayLayers        = 0;

    //! Specifies maximum number of attachment points for each render target.
    unsigned int    maxNumRenderTargetAttachments   = 0;
    
    //! Specifies maximum size (in bytes) of each constant buffer.
    unsigned int    maxConstantBufferSize           = 0;

    //! Specifies maximum size of each 1D texture.
    int             max1DTextureSize                = 0;

    //! Specifies maximum size of each 2D texture (for width and height).
    int             max2DTextureSize                = 0;

    //! Specifies maximum size of each 3D texture (for width, height, and depth).
    int             max3DTextureSize                = 0;

    //! Specifies maximum size of each cube texture (for width and height).
    int             maxCubeTextureSize              = 0;

    //! Specifies maxmimum anisotropy texture filter.
    int             maxAnisotropy                   = 0;

    //! Specifies maximum number of work groups in a compute shader.
    Gs::Vector3ui   maxNumComputeShaderWorkGroups;
    
    //! Specifies maximum work group size in a compute shader.
    Gs::Vector3ui   maxComputeShaderWorkGroupSize;
};


} // /namespace LLGL


#endif



// ================================================================================
