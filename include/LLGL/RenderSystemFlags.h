/*
 * RenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_SYSTEM_FLAGS_H__
#define __LLGL_RENDER_SYSTEM_FLAGS_H__


#include <Gauss/Vector3.h>
#include "ColorRGBA.h"
#include <cstddef>


namespace LLGL
{


/* ----- Constants ----- */

/**
\brief Specifies the maximal number of threads the system supports.
\see ConvertImageBuffer
*/
static const std::size_t maxThreadCount = ~0;


/* ----- Enumerations ----- */

/**
\brief Hardware buffer usage enumeration.
\remarks For OpenGL, the buffer usage is just a hint to the GL server.
For Direct3D, the buffer usage is crucial during buffer creation.
\see RenderSystem::SetupVertexBuffer
\see RenderSystem::SetupIndexBuffer
\see RenderSystem::SetupConstantBuffer
\see RenderSystem::SetupStorageBuffer
*/
enum class BufferUsage
{
    /**
    \brief The hardware buffer will be rarely changed by the client but often used by the hardware.
    \remarks For Direct3D 11, a buffer can use the static buffer usage, if always the entire buffer will be updated.
    Otherwise, the dynamic buffer usage must be used.
    */
    Static,

    /**
    \brief The hardware buffer will be often changed by the client (e.g. almost every frame).
    \remarks For Direct3D 11, a buffer must use the dynamic buffer usage, if it will only partially be updated at any time.
    */
    Dynamic,
};

/**
\brief Hardware buffer CPU acccess enumeration.
\see RenderSystem::MapBuffer
*/
enum class BufferCPUAccess
{
    ReadOnly,   //!< CPU read access only.
    WriteOnly,  //!< CPU write access only.
    ReadWrite,  //!< CPU read and write access.
};

//! Shading language version enumation.
enum class ShadingLanguage
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

//! Screen coordinate system origin enumeration.
enum class ScreenOrigin
{
    LowerLeft, //!< Screen origin is in the lower-left (default in OpenGL).
    UpperLeft, //!< Screen origin is in the upper-left (default in Direct3D).
};

//! Clipping depth range enumeration.
enum class ClippingRange
{
    MinusOneToOne,  //!< Clipping depth is in the range [-1, 1] (default in OpenGL).
    ZeroToOne,      //!< Clipping depth is in the range [0, 1] (default in Direct3D).
};


/* ----- Structures ----- */

//! Render system configuration structure.
struct RenderSystemConfiguration
{
    /**
    \brief Specifies the default color for an uninitialized textures. The default value is white (255, 255, 255, 255).
    \remarks This will be used for each "SetupTexture..." function (not the "WriteTexture..." functions), when no initial image data is specified.
    */
    ColorRGBAub defaultImageColor;

    /**
    \brief Specifies the number of threads that will be used internally by the render system. By default maxThreadCount.
    \remarks This is mainly used by the Direct3D render systems, e.g. inside the "SetupTexture..." and "WriteTexture..." functions
    to convert the image data into the respective hardware texture format. OpenGL does this automatically.
    \see maxThreadCount
    */
    std::size_t threadCount = maxThreadCount;
};

//! Rendering capabilities structure.
struct RenderingCaps
{
    /**
    \brief Screen coordinate system origin.
    \remarks This determines the coordinate space of viewports, scissors, and framebuffers.
    */
    ScreenOrigin    screenOrigin                    = ScreenOrigin::UpperLeft;

    //! Clipping depth range.
    ClippingRange   clippingRange                   = ClippingRange::ZeroToOne;

    /**
    \brief Specifies whether GLSL shaders are supported or not.
    \note Only supported with: OpenGL, Vulkan.
    */
    bool            hasGLSL                         = false;

    /**
    \brief Specifies whether HLSL shaders are supported or not.
    \note Only supported with: Direct3D 11, Direct3D 12.
    */
    bool            hasHLSL                         = false;

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
    
    //! Specifies whether samplers are supported.
    bool            hasSamplers                     = false;

    //! Specifies whether constant buffers (also "uniform buffer objects") are supported.
    bool            hasConstantBuffers              = false;

    //! Specifies whether storage buffers (also "read/write buffers") are supported.
    bool            hasStorageBuffers               = false;

    //! Specifies whether individual shader uniforms are supported (typically only for OpenGL 2.0+).
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

    //! Specifies whether conservative rasterization is supported.
    bool            hasConservativeRasterization    = false;

    //! Specifies maximum number of texture array layers (for 1D-, 2D-, and cube textures).
    unsigned int    maxNumTextureArrayLayers        = 0;

    //! Specifies maximum number of attachment points for each render target.
    unsigned int    maxNumRenderTargetAttachments   = 0;
    
    //! Specifies maximum size (in bytes) of each constant buffer.
    unsigned int    maxConstantBufferSize           = 0;

    //! Specifies maximum number of patch control points.
    int             maxPatchVertices                = 0;

    //! Specifies maximum size of each 1D texture.
    int             max1DTextureSize                = 0;

    //! Specifies maximum size of each 2D texture (for width and height).
    int             max2DTextureSize                = 0;

    //! Specifies maximum size of each 3D texture (for width, height, and depth).
    int             max3DTextureSize                = 0;

    //! Specifies maximum size of each cube texture (for width and height).
    int             maxCubeTextureSize              = 0;

    //! Specifies maximum anisotropy texture filter.
    int             maxAnisotropy                   = 0;

    //! Specifies maximum number of work groups in a compute shader.
    Gs::Vector3ui   maxNumComputeShaderWorkGroups;
    
    //! Specifies maximum work group size in a compute shader.
    Gs::Vector3ui   maxComputeShaderWorkGroupSize;
};


} // /namespace LLGL


#endif



// ================================================================================
