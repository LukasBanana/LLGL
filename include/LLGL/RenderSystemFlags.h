/*
 * RenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_FLAGS_H
#define LLGL_RENDER_SYSTEM_FLAGS_H


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
GLSL versions range from 110 (ver. 1.10) to 450 (ver. 4.50),
GLSL ES versions range from 100100 (ver. 1.00) to 100320 (ver. 3.20),
HLSL versions range from 200200 (ver. 2.0) to 200510 (ver. 5.1).
and Metal versions range from 300100 (ver. 1.0) to 300120 (ver. 1.2).
*/
enum class ShadingLanguage
{
    Unsupported = 0,        //!< Enumeration entry if shaders are not supported. Value is 0.

    GLSL_110    = 110,      //!< GLSL 1.10 (since OpenGL 2.0). Value is 110.
    GLSL_120    = 120,      //!< GLSL 1.20 (since OpenGL 2.1). Value is 120.
    GLSL_130    = 130,      //!< GLSL 1.30 (since OpenGL 3.0). Value is 130.
    GLSL_140    = 140,      //!< GLSL 1.40 (since OpenGL 3.1). Value is 140.
    GLSL_150    = 150,      //!< GLSL 1.50 (since OpenGL 3.2). Value is 150.
    GLSL_330    = 330,      //!< GLSL 3.30 (since OpenGL 3.3). Value is 330.
    GLSL_400    = 400,      //!< GLSL 4.00 (since OpenGL 4.0). Value is 400.
    GLSL_410    = 410,      //!< GLSL 4.10 (since OpenGL 4.1). Value is 410.
    GLSL_420    = 420,      //!< GLSL 4.20 (since OpenGL 4.2). Value is 420.
    GLSL_430    = 430,      //!< GLSL 4.30 (since OpenGL 4.3). Value is 430.
    GLSL_440    = 440,      //!< GLSL 4.40 (since OpenGL 4.4). Value is 440.
    GLSL_450    = 450,      //!< GLSL 4.50 (since OpenGL 4.5). Value is 450.
    GLSL_460    = 460,      //!< GLSL 4.60 (since OpenGL 4.6). Value is 460.

    GLSL_ES_100 = 100100,   //!< GLSL ES 1.00 (since OpenGL ES 2.0). Values is 100100.
    GLSL_ES_300 = 100300,   //!< GLSL ES 3.00 (since OpenGL ES 3.0). Values is 100300.
    GLSL_ES_310 = 100310,   //!< GLSL ES 3.10 (since OpenGL ES 3.1). Values is 100310.
    GLSL_ES_320 = 100320,   //!< GLSL ES 3.20 (since OpenGL ES 3.2). Values is 100320.

    HLSL_2_0    = 200200,   //!< HLSL 2.0 (since Direct3D 9). Value is 200200.
    HLSL_2_0a   = 200201,   //!< HLSL 2.0a (since Direct3D 9a). Value is 200201.
    HLSL_2_0b   = 200202,   //!< HLSL 2.0b (since Direct3D 9b). Value is 200202.
    HLSL_3_0    = 200300,   //!< HLSL 3.0 (since Direct3D 9c). Value is 200300.
    HLSL_4_0    = 200400,   //!< HLSL 4.0 (since Direct3D 10). Value is 200400.
    HLSL_4_1    = 200410,   //!< HLSL 4.1 (since Direct3D 10.1). Value is 200410.
    HLSL_5_0    = 200500,   //!< HLSL 5.0 (since Direct3D 11). Value is 200500.
    HLSL_5_1    = 200510,   //!< HLSL 5.1 (since Direct3D 12 and Direct3D 11.3). Value is 200510.

    Metal_1_0   = 300100,   //!< Metal shading language 1.0 (since iOS 8.0). Value is 300100.
    Metal_1_1   = 300110,   //!< Metal shading language 1.1 (since iOS 9.0 and OS X 10.11). Value is 300110.
    Metal_1_2   = 300120,   //!< Metal shading language 1.2 (since iOS 10.0 and macOS 10.12). Value is 300120.
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

//! Structure of image initialization for textures without initial image data.
struct ImageInitialization
{
    /**
    \brief Enables or disables the default initialization of texture images. By default enabled.
    \remarks This will be used when a texture is created and no initial image data is specified.
    If this is false and a texture is created without initial image data, the texture remains uninitialized.
    \see defaultImageColor
    \see defaultImageDepth
    */
    bool        enabled { true };

    /**
    \brief Specifies the default color for uninitialized textures. The default value is black (0, 0, 0, 0).
    \remarks This will be used when a texture is created and no initial image data is specified.
    */
    ColorRGBAub color   { 0, 0, 0, 0 };

    /**
    \brief Specifies the default depth value for uninitialized depth textures. The default value is 0.
    \remarks This will be used when a depth texture is created and no initial image data is specified.
    */
    float       depth   { 0.0f };
};

//! Render system configuration structure.
struct RenderSystemConfiguration
{
    //! Image initialization for textures without initial image data.
    ImageInitialization imageInitialization;

    /**
    \brief Specifies the number of threads that will be used internally by the render system. By default maxThreadCount.
    \remarks This is mainly used by the Direct3D render systems, e.g. inside the "CreateTexture" and "WriteTexture" functions
    to convert the image data into the respective hardware texture format. OpenGL does this automatically.
    \see maxThreadCount
    */
    size_t              threadCount         { maxThreadCount };
};

/**
\brief Renderer identification number enumeration.
\remarks There are several IDs for reserved future renderes, which are currently not supported (and maybe never supported).
You can use an ID greater than 'RendererID::Reserved' (which has a value of 0x000000ff) for your own renderer.
Or use one of the pre-defined IDs if you want to implement your own OpenGL/ Direct3D or whatever renderer.
\see RendererInfo::rendererID
*/
struct RendererID
{
    static const unsigned int Undefined     = 0x00000000; //!< Undefined ID number.

    static const unsigned int OpenGL        = 0x00000001; //!< ID number for an OpenGL renderer.
    static const unsigned int OpenGLES1     = 0x00000002; //!< ID number for an OpenGL ES 1 renderer.
    static const unsigned int OpenGLES2     = 0x00000003; //!< ID number for an OpenGL ES 2 renderer.
    static const unsigned int OpenGLES3     = 0x00000004; //!< ID number for an OpenGL ES 3 renderer.
    static const unsigned int Direct3D9     = 0x00000005; //!< ID number for a Direct3D 9 renderer.
    static const unsigned int Direct3D10    = 0x00000006; //!< ID number for a Direct3D 10 renderer.
    static const unsigned int Direct3D11    = 0x00000007; //!< ID number for a Direct3D 11 renderer.
    static const unsigned int Direct3D12    = 0x00000008; //!< ID number for a Direct3D 12 renderer.
    static const unsigned int Vulkan        = 0x00000009; //!< ID number for a Vulkan renderer.
    static const unsigned int Metal         = 0x0000000a; //!< ID number for a Metal renderer.

    static const unsigned int Reserved      = 0x000000ff; //!< Highest ID number for reserved future renderers. Value is 0x000000ff.
};

//! Renderer basic information structure.
struct RendererInfo
{
    std::string rendererName;           //!< Rendering API name and version (e.g. "OpenGL 4.5.0").
    std::string deviceName;             //!< Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").
    std::string vendorName;             //!< Vendor name of the renderer device (e.g. "NVIDIA Corporation").
    std::string shadingLanguageName;    //!< Shading language version (e.g. "GLSL 4.50").
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

    //! Specifies whether render targets (also "framebuffer objects") are supported.
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
    \see RenderContext::Dispatch
    */
    Gs::Vector3ui   maxNumComputeShaderWorkGroups;
    
    //! Specifies maximum work group size in a compute shader.
    Gs::Vector3ui   maxComputeShaderWorkGroupSize;
};


} // /namespace LLGL


#endif



// ================================================================================
