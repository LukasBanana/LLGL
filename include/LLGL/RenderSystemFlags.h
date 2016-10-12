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
#include <string>


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
\brief Shading language version enumation.
\remarks These enumeration entries can be casted to an integer to get the respective version number.
GLSL versions range from 110 (v.1.10) to 450 (v.4.50),
and HLSL version range from 100200 (v.2.0) to 100500 (v.5.0).
*/
enum class ShadingLanguage
{
    Unsupported = 0,        //!< Enumeration entry if shaders are not supported.

    GLSL_110    = 110,      //!< GLSL 1.10 (since OpenGL 2.0).
    GLSL_120    = 120,      //!< GLSL 1.20 (since OpenGL 2.1).
    GLSL_130    = 130,      //!< GLSL 1.30 (since OpenGL 3.0).
    GLSL_140    = 140,      //!< GLSL 1.40 (since OpenGL 3.1).
    GLSL_150    = 150,      //!< GLSL 1.50 (since OpenGL 3.2).
    GLSL_330    = 330,      //!< GLSL 3.30 (since OpenGL 3.3).
    GLSL_400    = 400,      //!< GLSL 4.00 (since OpenGL 4.0).
    GLSL_410    = 410,      //!< GLSL 4.10 (since OpenGL 4.1).
    GLSL_420    = 420,      //!< GLSL 4.20 (since OpenGL 4.2).
    GLSL_430    = 430,      //!< GLSL 4.30 (since OpenGL 4.3).
    GLSL_440    = 440,      //!< GLSL 4.40 (since OpenGL 4.4).
    GLSL_450    = 450,      //!< GLSL 4.50 (since OpenGL 4.5).

    HLSL_2_0    = 100200,   //!< HLSL 2.0 (since Direct3D 9).
    HLSL_2_0a   = 100201,   //!< HLSL 2.0a (since Direct3D 9a).
    HLSL_2_0b   = 100202,   //!< HLSL 2.0b (since Direct3D 9b).
    HLSL_3_0    = 100300,   //!< HLSL 3.0 (since Direct3D 9c).
    HLSL_4_0    = 100400,   //!< HLSL 4.0 (since Direct3D 10).
    HLSL_4_1    = 100410,   //!< HLSL 4.1 (since Direct3D 10.1).
    HLSL_5_0    = 100500,   //!< HLSL 5.0 (since Direct3D 11).
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
    \brief Specifies the default color for an uninitialized textures. The default value is black (0, 0, 0, 0).
    \remarks This will be used when a texture is created and no initial image data is specified.
    */
    ColorRGBAub defaultImageColor { 0, 0, 0, 0 };

    /**
    \brief Specifies the number of threads that will be used internally by the render system. By default maxThreadCount.
    \remarks This is mainly used by the Direct3D render systems, e.g. inside the "CreateTexture" and "WriteTexture" functions
    to convert the image data into the respective hardware texture format. OpenGL does this automatically.
    \see maxThreadCount
    */
    std::size_t threadCount = maxThreadCount;
};

/**
\brief Renderer identification number enumeration.
\see RendererInfo::rendererID
*/
struct RendererID
{
    static const unsigned int OpenGL        = 0x00000001; //!< ID number for the OpenGL renderer.
    static const unsigned int Direct3D11    = 0x00000002; //!< ID number for the Direct3D 11 renderer.
    static const unsigned int Direct3D12    = 0x00000003; //!< ID number for the Direct3D 12 renderer.
    static const unsigned int Vulkan        = 0x00000004; //!< ID number for the Vulkan renderer.

    static const unsigned int Reserved5     = 0x00000005; //!< ID number for a reserved future renderer.
    static const unsigned int Reserved6     = 0x00000006; //!< ID number for a reserved future renderer.
    static const unsigned int Reserved7     = 0x00000007; //!< ID number for a reserved future renderer.
    static const unsigned int Reserved8     = 0x00000008; //!< ID number for a reserved future renderer.
    static const unsigned int Reserved9     = 0x00000009; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedA     = 0x0000000a; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedB     = 0x0000000b; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedC     = 0x0000000c; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedD     = 0x0000000d; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedE     = 0x0000000e; //!< ID number for a reserved future renderer.
    static const unsigned int ReservedF     = 0x0000000f; //!< ID number for a reserved future renderer.
};

//! Renderer basic information structure.
struct RendererInfo
{
    std::string     rendererName;           //!< Rendering API name and version (e.g. "OpenGL 4.5.0").
    std::string     deviceName;             //!< Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").
    std::string     vendorName;             //!< Vendor name of the renderer device (e.g. "NVIDIA Corporation").
    std::string     shadingLanguageName;    //!< Shading language version (e.g. "GLSL 4.50").

    /**
    \brief Rendering API identification number.
    \remarks This can be a value of the RendererID entries.
    Since the render system is modular, a new render system can use its own ID number.
    \see RendererID
    */
    unsigned int    rendererID = 0;
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

    //! Latest suppported shading language.
    ShadingLanguage shadingLanguage                 = ShadingLanguage::Unsupported;

    //! Specifies whether render targets (also "frame buffer objects") are supported.
    bool            hasRenderTargets                = false;

    /**
    \brief Specifies whether 3D textures are supported.
    \see TextureType::Texture3D
    */
    bool            has3DTextures                   = false;

    /**
    \brief Specifies whether cube textures are supported.
    \see TextureType::TextureCube
    */
    bool            hasCubeTextures                 = false;

    /**
    \brief Specifies whether 1D- and 2D array textures are supported.
    \see TextureType::Texture1DArray
    \see TextureType::Texture2DArray
    */
    bool            hasTextureArrays                = false;

    /**
    \brief Specifies whether cube array textures are supported.
    \see TextureType::TextureCubeArray
    */
    bool            hasCubeTextureArrays            = false;

    /**
    \brief Specifies whether multi-sample textures are supported.
    \see TextureType::Texture2DMS
    \see TextureType::Texture2DMSArray
    */
    bool            hasMultiSampleTextures          = false;
    
    //! Specifies whether samplers are supported.
    bool            hasSamplers                     = false;

    /**
    \brief Specifies whether constant buffers (also "uniform buffer objects") are supported.
    \see BufferType::Constant
    */
    bool            hasConstantBuffers              = false;

    /**
    \brief Specifies whether storage buffers (also "read/write buffers") are supported.
    \see BufferType::Storage
    */
    bool            hasStorageBuffers               = false;

    /**
    \brief Specifies whether individual shader uniforms are supported (typically only for OpenGL 2.0+).
    \see ShaderProgram::LockShaderUniform
    */
    bool            hasUniforms                     = false;

    //! Specifies whether geometry shaders are supported.
    bool            hasGeometryShaders              = false;

    //! Specifies whether tessellation shaders are supported.
    bool            hasTessellationShaders          = false;

    //! Speciifes whether compute shaders are supported.
    bool            hasComputeShaders               = false;

    /**
    \brief Specifies whether hardware instancing is supported.
    \see RenderContext::DrawInstanced(unsigned int, unsigned int, unsigned int)
    \see RenderContext::DrawIndexedInstanced(unsigned int, unsigned int, unsigned int)
    \see RenderContext::DrawIndexedInstanced(unsigned int, unsigned int, unsigned int, int)
    */
    bool            hasInstancing                   = false;

    /**
    \brief Specifies whether hardware instancing with instance offsets is supported.
    \see RenderContext::DrawInstanced(unsigned int, unsigned int, unsigned int, unsigned int)
    \see RenderContext::DrawIndexedInstanced(unsigned int, unsigned int, unsigned int, int, unsigned int)
    */
    bool            hasOffsetInstancing             = false;

    //! Specifies whether multiple viewports, depth-ranges, and scissors are supported at once.
    bool            hasViewportArrays               = false;

    /**
    \brief Specifies whether conservative rasterization is supported.
    \see RasterizerDescriptor::conservativeRasterization
    */
    bool            hasConservativeRasterization    = false;

    /**
    \brief Specifies whether stream-output is supported.
    \see ShaderSource::streamOutput
    \see CommandBuffer::BeginStreamOutput
    */
    bool            hasStreamOutputs                = false;

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

    /**
    \brief Specifies maximum anisotropy texture filter.
    \see SamplerDescriptor::maxAnisotropy
    */
    int             maxAnisotropy                   = 0;

    /**
    \brief Specifies maximum number of work groups in a compute shader.
    \see RenderContext::DispatchCompute
    */
    Gs::Vector3ui   maxNumComputeShaderWorkGroups;
    
    //! Specifies maximum work group size in a compute shader.
    Gs::Vector3ui   maxComputeShaderWorkGroupSize;
};


} // /namespace LLGL


#endif



// ================================================================================
