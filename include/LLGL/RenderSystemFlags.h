/*
 * RenderSystemFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_SYSTEM_FLAGS_H
#define LLGL_RENDER_SYSTEM_FLAGS_H


#include "Export.h"
#include "CommandBufferFlags.h"
#include "TextureFlags.h"
#include "Constants.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Shading language version enumation.
\remarks These enumeration entries can be casted to an integer using the bitmask ShadingLanguage::VersionBitmask to get the respective version number:
\code
// 'versionNo' will have the value 330
static const auto versionGLSL330 = static_cast<std::uint32_t>(LLGL::ShadingLanguage::GLSL_330);
static const auto versionBitmask = static_cast<std::uint32_t>(LLGL::ShadingLanguage::VersionBitmask);
static const auto versionNo      = versionGLSL330 & versionBitmask;
\endcode
*/
enum class ShadingLanguage
{
    GLSL            = (0x10000),        //!< GLSL (OpenGL Shading Language).
    GLSL_110        = (0x10000 | 110),  //!< GLSL 1.10 (since OpenGL 2.0).
    GLSL_120        = (0x10000 | 120),  //!< GLSL 1.20 (since OpenGL 2.1).
    GLSL_130        = (0x10000 | 130),  //!< GLSL 1.30 (since OpenGL 3.0).
    GLSL_140        = (0x10000 | 140),  //!< GLSL 1.40 (since OpenGL 3.1).
    GLSL_150        = (0x10000 | 150),  //!< GLSL 1.50 (since OpenGL 3.2).
    GLSL_330        = (0x10000 | 330),  //!< GLSL 3.30 (since OpenGL 3.3).
    GLSL_400        = (0x10000 | 400),  //!< GLSL 4.00 (since OpenGL 4.0).
    GLSL_410        = (0x10000 | 410),  //!< GLSL 4.10 (since OpenGL 4.1).
    GLSL_420        = (0x10000 | 420),  //!< GLSL 4.20 (since OpenGL 4.2).
    GLSL_430        = (0x10000 | 430),  //!< GLSL 4.30 (since OpenGL 4.3).
    GLSL_440        = (0x10000 | 440),  //!< GLSL 4.40 (since OpenGL 4.4).
    GLSL_450        = (0x10000 | 450),  //!< GLSL 4.50 (since OpenGL 4.5).
    GLSL_460        = (0x10000 | 460),  //!< GLSL 4.60 (since OpenGL 4.6).

    ESSL            = (0x20000),        //!< ESSL (OpenGL ES Shading Language).
    ESSL_100        = (0x20000 | 100),  //!< ESSL 1.00 (since OpenGL ES 2.0).
    ESSL_300        = (0x20000 | 300),  //!< ESSL 3.00 (since OpenGL ES 3.0).
    ESSL_310        = (0x20000 | 310),  //!< ESSL 3.10 (since OpenGL ES 3.1).
    ESSL_320        = (0x20000 | 320),  //!< ESSL 3.20 (since OpenGL ES 3.2).

    HLSL            = (0x30000),        //!< HLSL (High Level Shading Language).
    HLSL_2_0        = (0x30000 | 200),  //!< HLSL 2.0 (since Direct3D 9).
    HLSL_2_0a       = (0x30000 | 201),  //!< HLSL 2.0a (since Direct3D 9a).
    HLSL_2_0b       = (0x30000 | 202),  //!< HLSL 2.0b (since Direct3D 9b).
    HLSL_3_0        = (0x30000 | 300),  //!< HLSL 3.0 (since Direct3D 9c).
    HLSL_4_0        = (0x30000 | 400),  //!< HLSL 4.0 (since Direct3D 10).
    HLSL_4_1        = (0x30000 | 410),  //!< HLSL 4.1 (since Direct3D 10.1).
    HLSL_5_0        = (0x30000 | 500),  //!< HLSL 5.0 (since Direct3D 11).
    HLSL_5_1        = (0x30000 | 510),  //!< HLSL 5.1 (since Direct3D 12 and Direct3D 11.3).

    Metal           = (0x40000),        //!< Metal Shading Language. \note Not supported yet
    Metal_1_0       = (0x40000 | 100),  //!< Metal 1.0 (since iOS 8.0). \note Not supported yet
    Metal_1_1       = (0x40000 | 110),  //!< Metal 1.1 (since iOS 9.0 and OS X 10.11). \note Not supported yet
    Metal_1_2       = (0x40000 | 120),  //!< Metal 1.2 (since iOS 10.0 and macOS 10.12). \note Not supported yet

    SPIRV           = (0x50000),        //!< SPIR-V Shading Language.
    SPIRV_100       = (0x50000 | 100),  //!< SPIR-V 1.0.

    VersionBitmask  = 0x0000ffff,       //!< Bitmask for the version number of each shading language enumeration entry.
};

//! Screen coordinate system origin enumeration.
enum class ScreenOrigin
{
    /**
    \brief Specifies a screen origin in the lower-left.
    \note Native screen origin in: OpenGL.
    */
    LowerLeft,

    /**
    \brief Specifies a screen origin in the upper-left.
    \note Native screen origin in: Direct3D 11, Direct3D 12, Vulkan.
    */
    UpperLeft,
};

//! Clipping depth range enumeration.
enum class ClippingRange
{
    /**
    \brief Specifies the clipping depth range [-1, 1].
    \note Native clipping depth range in: OpenGL.
    */
    MinusOneToOne,

    /**
    \brief Specifies the clipping depth range [0, 1].
    \note Native clipping depth range in: Direct3D 11, Direct3D 12, Vulkan.
    */
    ZeroToOne,
};

/**
\brief Classifications of CPU access to hardware buffers and textures.
\see RenderSystem::MapBuffer
*/
enum class CPUAccess
{
    ReadOnly,   //!< CPU read access only.
    WriteOnly,  //!< CPU write access only.
    ReadWrite,  //!< CPU read and write access.
};


/* ----- Structures ----- */

//! Structure of image initialization for textures without initial image data.
struct ImageInitialization
{
    /**
    \brief Enables or disables the default initialization of texture images. By default true.
    \remarks This will be used when a texture is created and no initial image data is specified.
    If this is false and a texture is created without initial image data, the texture remains uninitialized.
    \note Reading or sampling uninitialized textures is undefined behavior.
    */
    bool        enabled     = true;

    /**
    \brief Specifies the default value to clear uninitialized textures.
    \todo Currently only supports initialization of color and depth. Default initialization of stencil values is not supported yet.
    */
    ClearValue  clearValue;
};

//! Render system configuration structure.
struct RenderSystemConfiguration
{
    //! Image initialization for textures without initial image data.
    ImageInitialization imageInitialization;

    /**
    \brief Specifies the number of threads that will be used internally by the render system. By default Constants::maxThreadCount.
    \remarks This is mainly used by the Direct3D render systems, e.g. inside the "CreateTexture" and "WriteTexture" functions
    to convert the image data into the respective hardware texture format. OpenGL does this automatically.
    \see Constants::maxThreadCount
    */
    std::size_t         threadCount         = Constants::maxThreadCount;
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
    static const int Undefined  = 0x00000000; //!< Undefined ID number.

    static const int OpenGL     = 0x00000001; //!< ID number for an OpenGL renderer.
    static const int OpenGLES1  = 0x00000002; //!< ID number for an OpenGL ES 1 renderer.
    static const int OpenGLES2  = 0x00000003; //!< ID number for an OpenGL ES 2 renderer.
    static const int OpenGLES3  = 0x00000004; //!< ID number for an OpenGL ES 3 renderer.
    static const int Direct3D9  = 0x00000005; //!< ID number for a Direct3D 9 renderer.
    static const int Direct3D10 = 0x00000006; //!< ID number for a Direct3D 10 renderer.
    static const int Direct3D11 = 0x00000007; //!< ID number for a Direct3D 11 renderer.
    static const int Direct3D12 = 0x00000008; //!< ID number for a Direct3D 12 renderer.
    static const int Vulkan     = 0x00000009; //!< ID number for a Vulkan renderer.
    static const int Metal      = 0x0000000a; //!< ID number for a Metal renderer.

    static const int Reserved   = 0x000000ff; //!< Highest ID number for reserved future renderers. Value is 0x000000ff.
};

//! Renderer basic information structure.
struct RendererInfo
{
    std::string rendererName;           //!< Rendering API name and version (e.g. "OpenGL 4.5.0").
    std::string deviceName;             //!< Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").
    std::string vendorName;             //!< Vendor name of the renderer device (e.g. "NVIDIA Corporation").
    std::string shadingLanguageName;    //!< Shading language version (e.g. "GLSL 4.50").
};

/**
\brief Application descriptor structure.
\remarks This is currently only used for the Vulkan renderer, when a debug or validation layer is enabled.
*/
struct ApplicationDescriptor
{
    std::string     applicationName;    //!< Descriptive string of the application.
    std::uint32_t   applicationVersion; //!< Version number of the application.
    std::string     engineName;         //!< Descriptive string of the engine or middleware.
    std::uint32_t   engineVersion;      //!< Version number of the engine or middleware.
};

/**
\brief Structure for a Vulkan renderer specific configuration.
\remarks The nomenclature here is "Renderer" instead of "RenderSystem" since the configuration is renderer specific
and does not denote a configuration of the entire system.
*/
struct VulkanRendererConfiguration
{
    /**
    \brief Application descriptor used when a Vulkan debug or validation layer is enabled.
    \see ApplicationDescriptor
    */
    ApplicationDescriptor   application;

    /**
    \brief Minimal allocation size for a device memory chunk. By default 1024*1024, i.e. 1 MB of VRAM.
    \remarks Vulkan only allows a limited set of device memory objects (e.g. 4096 on a GPU with 8 GB of VRAM).
    This member specifies the minimum size used for hardware memory allocation of such a memory chunk.
    The Vulkan render system automatically manages sub-region allocation and defragmentation.
    */
    std::uint64_t           minDeviceMemoryAllocationSize   = 1024*1024;

    /**
    \brief Specifies whether fragmentation of the device memory blocks shall be kept low. By default false.
    \remarks If this is true, each buffer and image allocation first tries to find a reusable device memory block
    within a single VkDeviceMemory chunk (which might be potentially slower).
    Whenever a VkDeviceMemory chunk is full, the memory manager tries to reduce fragmentation anyways.
    */
    bool                    reduceDeviceMemoryFragmentation = false;
};

/**
\brief Render system descriptor structure.
\remarks This can be used for some refinements of a specific renderer, e.g. to configure the Vulkan device memory manager.
\see RenderSystem::Load
*/
struct RenderSystemDescriptor
{
    RenderSystemDescriptor() = default;
    RenderSystemDescriptor(const RenderSystemDescriptor&) = default;
    RenderSystemDescriptor& operator = (const RenderSystemDescriptor&) = default;

    //! Constructor to initialize the descriptor with the module name form an std::string.
    inline RenderSystemDescriptor(const std::string& moduleName) :
        moduleName { moduleName }
    {
    }

    //! Constructor to initialize the descriptor with the module name form a null terminated string.
    inline RenderSystemDescriptor(const char* moduleName) :
        moduleName { moduleName }
    {
    }

    /**
    \brief Specifies the name from which the new render system is to be loaded.
    \remarks This denotes a shared library (*.dll-files on Windows, *.so-files on Unix systems).
    If compiled in debug mode, the postfix "D" is appended to the module name.
    Moreover, the platform dependent file extension is always added automatically
    as well as the prefix "LLGL_", i.e. a module name "OpenGL" will be
    translated to "LLGL_OpenGLD.dll", if compiled on Windows in Debug mode.
    */
    std::string moduleName;

    /**
    \brief Optional raw pointer to a renderer specific configuration structure.
    \remarks This can be used to pass some refinement configurations to the render system when the module is loaded.
    Example usage (for Vulkan renderer):
    \code
    // Initialize Vulkan specific configurations (e.g. always allocate at least 1GB of VRAM for each device memory chunk).
    LLGL::VulkanRendererConfiguration config;
    config.minDeviceMemoryAllocationSize = 1024*1024*1024;

    // Initialize render system descriptor
    LLGL::RenderSystemDescriptor rendererDesc;
    rendererDesc.moduleName         = "Vulkan";
    rendererDesc.rendererConfig     = &config;
    rendererDesc.rendererConfigSize = sizeof(config);

    // Load Vulkan render system
    auto renderer = LLGL::RenderSystem::Load(rendererDesc);
    \endcode
    \see rendererConfigSize
    \see VulkanRendererConfiguration
    */
    const void* rendererConfig      = nullptr;

    /**
    \brief Specifies the size (in bytes) of the structure where the 'rendererConfig' member points to (use 'sizeof' with the respective structure). By default 0.
    \remarks If 'rendererConfig' is null then this member is ignored.
    \see rendererConfig
    */
    std::size_t rendererConfigSize  = 0;
};

/**
\brief Contains the attributes for all supported rendering features.
\see RenderingCapabilities
*/
struct RenderingFeatures
{
    /**
    \brief Specifies whether the render system supports extended command buffers with dynamic state access for shader resources.
    \remarks This is only supported by older graphics APIs such as OpenGL and Direct3D 11.
    \see RenderSystem::CreateCommandBufferExt
    \see CommandBufferExt
    */
    bool hasCommandBufferExt            = false;

    //! Specifies whether render targets (also "framebuffer objects") are supported.
    bool hasRenderTargets               = false;

    /**
    \brief Specifies whether 3D textures are supported.
    \see TextureType::Texture3D
    */
    bool has3DTextures                  = false;

    /**
    \brief Specifies whether cube textures are supported.
    \see TextureType::TextureCube
    */
    bool hasCubeTextures                = false;

    /**
    \brief Specifies whether 1D- and 2D array textures are supported.
    \see TextureType::Texture1DArray
    \see TextureType::Texture2DArray
    */
    bool hasArrayTextures               = false;

    /**
    \brief Specifies whether cube array textures are supported.
    \see TextureType::TextureCubeArray
    */
    bool hasCubeArrayTextures           = false;

    /**
    \brief Specifies whether multi-sample textures are supported.
    \see TextureType::Texture2DMS
    \see TextureType::Texture2DMSArray
    */
    bool hasMultiSampleTextures         = false;

    //! Specifies whether samplers are supported.
    bool hasSamplers                    = false;

    /**
    \brief Specifies whether constant buffers (also "uniform buffer objects") are supported.
    \see BufferType::Constant
    */
    bool hasConstantBuffers             = false;

    /**
    \brief Specifies whether storage buffers (also "read/write buffers") are supported.
    \see BufferType::Storage
    */
    bool hasStorageBuffers              = false;

    /**
    \brief Specifies whether individual shader uniforms are supported (typically only for OpenGL 2.0+).
    \see ShaderProgram::LockShaderUniform
    */
    bool hasUniforms                    = false;

    //! Specifies whether geometry shaders are supported.
    bool hasGeometryShaders             = false;

    //! Specifies whether tessellation shaders are supported.
    bool hasTessellationShaders         = false;

    //! Speciifes whether compute shaders are supported.
    bool hasComputeShaders              = false;

    /**
    \brief Specifies whether hardware instancing is supported.
    \see CommandBuffer::DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t)
    \see CommandBuffer::DrawIndexedInstanced(std::uint32_t, std::uint32_t, std::uint32_t)
    \see CommandBuffer::DrawIndexedInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t)
    */
    bool hasInstancing                  = false;

    /**
    \brief Specifies whether hardware instancing with instance offsets is supported.
    \see CommandBuffer::DrawInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)
    \see CommandBuffer::DrawIndexedInstanced(std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t)
    */
    bool hasOffsetInstancing            = false;

    /**
    \brief Specifies whether multiple viewports, depth-ranges, and scissors at once are supported.
    \see RenderingLimits::maxNumViewports
    */
    bool hasViewportArrays              = false;

    /**
    \brief Specifies whether conservative rasterization is supported.
    \see RasterizerDescriptor::conservativeRasterization
    */
    bool hasConservativeRasterization   = false;

    /**
    \brief Specifies whether stream-output is supported.
    \see ShaderSource::streamOutput
    \see CommandBuffer::BeginStreamOutput
    */
    bool hasStreamOutputs               = false;

    /**
    \brief Specifies whether logic fragment operations are supported.
    \note For Direct3D 11, feature level 11.1 is required.
    \see BlendDescriptor::logicOp
    */
    bool hasLogicOp                     = false;
};

/**
\brief Contains all rendering limitations such as maximum buffer size, maximum texture resolution etc.
\see RenderingCapabilities
*/
struct RenderingLimits
{
    /**
    \brief Specifies the range for rasterizer line widths. By default [1, 1].
    \note Only supported with: OpenGL, Vulkan.
    \see RasterizerDescriptor::lineWidth
    */
    float           lineWidthRange[2]                   = { 1.0f, 1.0f };

    /**
    \brief Specifies the maximum number of texture array layers (for 1D-, 2D-, and cube textures).
    \see TextureDescriptor::arrayLayers
    */
    std::uint32_t   maxNumTextureArrayLayers            = 0;

    /**
    \brief Specifies the maximum number of attachment points for each render target.
    \see RenderTargetDescriptor::attachments
    \see RenderPassDescriptor::colorAttachments
    */
    std::uint32_t   maxNumRenderTargetAttachments       = 0;

    /**
    \brief Specifies the maximum number of patch control points.
    \see PrimitiveTopology::Patches1
    \see PrimitiveTopology::Patches32
    */
    std::uint32_t   maxPatchVertices                    = 0;

    /**
    \brief Specifies the maximum size of each 1D texture.
    \see TextureDescriptor::extent
    */
    std::uint32_t   max1DTextureSize                    = 0;

    /**
    \brief Specifies the maximum size of each 2D texture (for width and height).
    \see TextureDescriptor::extent
    */
    std::uint32_t   max2DTextureSize                    = 0;

    /**
    \brief Specifies the maximum size of each 3D texture (for width, height, and depth).
    \see TextureDescriptor::extent
    */
    std::uint32_t   max3DTextureSize                    = 0;

    /**
    \brief Specifies the maximum size of each cube texture (for width and height).
    \see TextureDescriptor::extent
    */
    std::uint32_t   maxCubeTextureSize                  = 0;

    /**
    \brief Specifies the maximum anisotropy texture filter.
    \see SamplerDescriptor::maxAnisotropy
    */
    std::uint32_t   maxAnisotropy                       = 0;

    /**
    \brief Specifies the maximum number of work groups in a compute shader.
    \see CommandBuffer::Dispatch
    */
    std::uint32_t   maxNumComputeShaderWorkGroups[3]    = { 0, 0, 0 };

    //! Specifies the maximum work group size in a compute shader.
    std::uint32_t   maxComputeShaderWorkGroupSize[3]    = { 0, 0, 0 };

    /**
    \brief Specifies the maximum number of viewports and scissor rectangles. Most render systems have a maximum of 16.
    \see CommandBuffer::SetViewports
    \see CommandBuffer::SetScissors
    \see RenderingFeatures::hasViewportArrays
    */
    std::uint32_t   maxNumViewports                     = 0;

    /**
    \brief Specifies the maximum width and height of each viewport and scissor rectangle.
    \see Viewport::width
    \see Viewport::height
    */
    std::uint32_t   maxViewportSize[2]                  = { 0, 0 };

    /**
    \brief Specifies the maximum size (in bytes) that is supported for hardware buffers (vertex, index, storage buffers).
    \remarks Constant buffers are a special case for which 'maxConstantBufferSize' can be used.
    \see BufferDescriptor::size
    \see maxConstantBufferSize
    */
    std::uint64_t   maxBufferSize                       = 0;

    /**
    \brief Specifies the maximum size (in bytes) that is supported for hardware constant buffers.
    \remarks This is typically a lot smaller than the maximum size for other types of buffers.
    \see BufferDescriptor::size
    */
    std::uint64_t   maxConstantBufferSize               = 0;
};

/**
\brief Structure with all attributes describing the rendering capabilities of the render system.
\see RenderSystem::GetRenderingCaps
*/
struct RenderingCapabilities
{
    #if 1
    /**
    \brief Screen coordinate system origin.
    \remarks This determines the coordinate space of viewports, scissors, and framebuffers.
    */
    ScreenOrigin                    screenOrigin        = ScreenOrigin::UpperLeft;
    #else//TODO: use these members:
    ScreenOrigin                    screenSpaceOrigin   = ScreenOrigin::UpperLeft;
    ScreenOrigin                    textureSpaceOrigin  = ScreenOrigin::UpperLeft;
    #endif

    //! Specifies the clipping depth range.
    ClippingRange                   clippingRange       = ClippingRange::ZeroToOne;

    /**
    \brief Specifies the list of supported shading languages.
    \remarks This also specifies whether shaders can be loaded in source or binary form (using "Compile" or "LoadBinary" functions of the "Shader" interface).
    \see Shader::Compile
    \see Shader::LoadBinary
    */
    std::vector<ShadingLanguage>    shadingLanguages;

    /**
    \brief Specifies the list of supported texture formats.
    \see Format
    */
    std::vector<Format>             textureFormats;

    /**
    \brief Specifies all supported hardware features.
    \remarks Especially with OpenGL these features can vary between different hardware and GL versions.
    */
    RenderingFeatures               features;

    /**
    \brief Specifies all rendering limitations.
    \remarks Especially with OpenGL these features can vary between different hardware and GL versions.
    */
    RenderingLimits                 limits;
};


/* ----- Functions ----- */

/**
\brief Callback interface for the ValidateRenderingCaps function.
\param[in] info Specifies a description why an attribute did not fulfill the requirement.
\param[in] attrib Name of the attribute which did not fulfill the requirement.
\return True to continue the validation process, or false to break the validation process.
\see ValidateRenderingCaps
\ingroup group_callbacks
*/
using ValidateRenderingCapsFunc = std::function<bool(const std::string& info, const std::string& attrib)>;

/**
\brief Validates the presence of the specified required rendering capabilities.
\param[in] presentCaps Specifies the rendering capabilities that are present for a certain renderer.
\param[in] requiredCaps Specifies the rendering capabilities that are required for the host application to work properly.
\param[in] callback Optional callback to retrieve information about the attributes that did not fulfill the requirement.
If this is null the validation process breaks with the first attribute that did not fulfill the requirement. By default null.
\return True on success, otherwise at least one attribute did not fulfill the requirement.
\remarks Here is an example usage to print out all attributes that did not fulfill the requirement:
\code
// Initialize the requirements
LLGL::RenderingCapabilities myRequirements;
myRequirements.features.hasStorageBuffers = true;
myRequirements.features.hasComputeShaders = true;
myRequirements.limits.maxNumComputeShaderWorkGroups[0] = 1024;
myRequirements.limits.maxNumComputeShaderWorkGroups[1] = 1024;
myRequirements.limits.maxNumComputeShaderWorkGroups[2] = 1;
myRequirements.limits.maxComputeShaderWorkGroupSize[0] = 8;
myRequirements.limits.maxComputeShaderWorkGroupSize[1] = 8;
myRequirements.limits.maxComputeShaderWorkGroupSize[2] = 8;

// Validate rendering capabilities supported by the render system
LLGL::ValidateRenderingCaps(
    myRenderer->GetRenderingCaps(),
    myRequirements,
    [](const std::string& info, const std::string& attrib) {
        std::cerr << info << ": " << attrib << std::endl;
        return true;
    }
);
\endcode
\note The following attributes of the RenderingCapabilities structure are ignored: 'screenOrigin' and 'clippingRange'.
\see RenderingCapabilities
\see ValidateRenderingCapsFunc
*/
LLGL_EXPORT bool ValidateRenderingCaps(
    const RenderingCapabilities&        presentCaps,
    const RenderingCapabilities&        requiredCaps,
    const ValidateRenderingCapsFunc&    callback = nullptr
);


} // /namespace LLGL


#endif



// ================================================================================
