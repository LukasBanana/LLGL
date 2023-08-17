/*
 * RenderSystemFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_SYSTEM_FLAGS_H
#define LLGL_RENDER_SYSTEM_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Constants.h>
#include <LLGL/RendererConfiguration.h>

#include <LLGL/Platform/Platform.h>
#if defined LLGL_OS_ANDROID
#   include <android_native_app_glue.h>
//#elif defined LLGL_OS_IOS
//#   include <LLGL/Platform/>
#endif

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>


namespace LLGL
{


class RenderingProfiler;
class RenderingDebugger;

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
    HLSL_5_1        = (0x30000 | 510),  //!< HLSL 5.1 (since Direct3D 11.3).
    HLSL_6_0        = (0x30000 | 600),  //!< HLSL 6.0 (since Direct3D 12). Shader model 6.0 adds wave intrinsics and 64-bit integer types to HLSL.
    HLSL_6_1        = (0x30000 | 601),  //!< HLSL 6.1 (since Direct3D 12). Shader model 6.1 adds \c SV_ViewID and \c SV_Barycentrics semantics to HLSL.
    HLSL_6_2        = (0x30000 | 602),  //!< HLSL 6.2 (since Direct3D 12). Shader model 6.2 adds 16-bit scalar types to HLSL.
    HLSL_6_3        = (0x30000 | 603),  //!< HLSL 6.3 (since Direct3D 12). Shader model 6.3 adds ray tracing (DXR) to HLSL.
    HLSL_6_4        = (0x30000 | 604),  //!< HLSL 6.4 (since Direct3D 12). Shader model 6.4 adds machine learning intrinsics to HLSL.

    Metal           = (0x40000),        //!< Metal Shading Language.
    Metal_1_0       = (0x40000 | 100),  //!< Metal 1.0 (since iOS 8.0).
    Metal_1_1       = (0x40000 | 110),  //!< Metal 1.1 (since iOS 9.0 and OS X 10.11).
    Metal_1_2       = (0x40000 | 120),  //!< Metal 1.2 (since iOS 10.0 and macOS 10.12).
    Metal_2_0       = (0x40000 | 200),  //!< Metal 2.0 (since iOS 11.0 and macOS 10.13).
    Metal_2_1       = (0x40000 | 210),  //!< Metal 2.1 (since iOS 12.0 and macOS 10.14).

    SPIRV           = (0x50000),        //!< SPIR-V Shading Language.
    SPIRV_100       = (0x50000 | 100),  //!< SPIR-V 1.0.

    VersionBitmask  = 0x0000ffff,       //!< Bitmask for the version number of each shading language enumeration entry.
};

/**
\brief Screen coordinate system origin enumeration.
\see RenderingCapabilities::screenOrigin
*/
enum class ScreenOrigin
{
    /**
    \brief Specifies a screen origin in the lower-left.
    \note Native screen origin in: OpenGL (If \c GL_ARB_clip_control is \e not supported).
    */
    LowerLeft,

    /**
    \brief Specifies a screen origin in the upper-left.
    \note Native screen origin in: Direct3D 11, Direct3D 12, Vulkan, OpenGL (If \c GL_ARB_clip_control \e is supported), Metal.
    */
    UpperLeft,
};

/**
\brief Clipping depth range enumeration.
\see RenderingCapabilities::clippingRange
*/
enum class ClippingRange
{
    /**
    \brief Specifies the clipping depth range [-1, 1].
    \note Native clipping depth range in: OpenGL.
    */
    MinusOneToOne,

    /**
    \brief Specifies the clipping depth range [0, 1].
    \note Native clipping depth range in: Direct3D 11, Direct3D 12, Vulkan, Metal.
    */
    ZeroToOne,
};

/**
\brief Classifications of CPU access to mapped resources.
\see RenderSystem::MapBuffer
\see CPUAccessFlags
*/
enum class CPUAccess
{
    /**
    \brief CPU read access to a mapped resource.
    \remarks If this is used for RenderSystem::MapBuffer,
    the respective buffer must have been created with the CPUAccessFlags::Read access flag.
    */
    ReadOnly,

    /**
    \brief CPU write access to a mapped resource.
    \remarks If this is used for RenderSystem::MapBuffer,
    the respective buffer must have been created with the CPUAccessFlags::Write access flag.
    */
    WriteOnly,

    /**
    \brief CPU write access to a mapped resource, where the previous content \e can be discarded.
    \remarks If this is used for RenderSystem::MapBuffer,
    the respective buffer must have been created with the CPUAccessFlags::Write access flag.
    \note Whether the previous content is discarded depends on the rendering API.
    */
    WriteDiscard,

    /**
    \brief CPU read and write access to a mapped resource.
    \remarks If this is used for RenderSystem::MapBuffer,
    the respective buffer must have been created with both the CPUAccessFlags::Read and the CPUAccessFlags::Write access flags.
    */
    ReadWrite,
};


/* ----- Flags ----- */

/**
\brief Render system flags enumeration.
\see RenderSystemDescriptor::flags
*/
struct RenderSystemFlags
{
    enum
    {
        /**
        \brief Specifies that a debug device is requested for the render system backend.
        \remarks This is only a hint to LLGL since not every backend supports native debug layers.
        Here is an overview of what impact this flag has to the respective renderer:
        - Direct3D 12: A debug controller of type \c ID3D12Debug will be created and GPU validation via \c EnableDebugLayer and \c SetEnableGPUBasedValidation (D3D12.1) will be enabled.<br>
          See https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation
        - Direct3D 11: \c D3D11_CREATE_DEVICE_DEBUG will be added to the Direct3D device instance.<br>
          See https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-layers#debug-layer
        - Vulkan: A debug callback will be registered via \c vkCreateDebugReportCallbackEXT if the Vulkan extension \c "VK_EXT_debug_report" is available.<br>
          See https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_debug_report.html
        - OpenGL: A debug callback will be registered via \c glDebugMessageCallback if the OpenGL extension \c "GL_KHR_debug" is vailable.<br>
          See https://www.khronos.org/opengl/wiki/Debug_Output
        - Metal: Not supported.
        */
        DebugDevice = (1 << 0),
    };
};


/* ----- Structures ----- */

/**
\brief Renderer identification number enumeration.
\remarks There are several IDs for reserved future renderes, which are currently not supported (and maybe never supported).
You can use an ID greater than 'RendererID::Reserved' (which has a value of 0x000000ff) for your own renderer.
Or use one of the pre-defined IDs if you want to implement your own OpenGL/ Direct3D or whatever renderer.
\see RendererInfo::rendererID
*/
struct RendererID
{
    static constexpr int Undefined  = 0x00000000; //!< Undefined ID number.

    static constexpr int Null       = 0x00000001; //!< ID number for a Null renderer. This renderer does not render anything but provides the same interface for debugging purposes.
    static constexpr int OpenGL     = 0x00000002; //!< ID number for an OpenGL renderer.
    static constexpr int OpenGLES1  = 0x00000003; //!< ID number for an OpenGL ES 1 renderer.
    static constexpr int OpenGLES2  = 0x00000004; //!< ID number for an OpenGL ES 2 renderer.
    static constexpr int OpenGLES3  = 0x00000005; //!< ID number for an OpenGL ES 3 renderer.
    static constexpr int Direct3D9  = 0x00000006; //!< ID number for a Direct3D 9 renderer.
    static constexpr int Direct3D10 = 0x00000007; //!< ID number for a Direct3D 10 renderer.
    static constexpr int Direct3D11 = 0x00000008; //!< ID number for a Direct3D 11 renderer.
    static constexpr int Direct3D12 = 0x00000009; //!< ID number for a Direct3D 12 renderer.
    static constexpr int Vulkan     = 0x0000000A; //!< ID number for a Vulkan renderer.
    static constexpr int Metal      = 0x0000000B; //!< ID number for a Metal renderer.

    static constexpr int Reserved   = 0x000000FF; //!< Highest ID number for reserved future renderers. Value is 0x000000ff.
};

/**
\brief Renderer basic information structure.
\see RenderSystem::GetRendererInfo
*/
struct RendererInfo
{
    //! Rendering API name and version (e.g. "OpenGL 4.6").
    std::string                 rendererName;

    //! Renderer device name (e.g. "GeForce GTX 1070/PCIe/SSE2").
    std::string                 deviceName;

    //! Vendor name of the renderer device (e.g. "NVIDIA Corporation").
    std::string                 vendorName;

    //! Shading language version (e.g. "GLSL 4.50").
    std::string                 shadingLanguageName;

    //! List of enabled renderer extensions (e.g. "GL_ARB_direct_state_access" or "VK_EXT_conditional_rendering").
    std::vector<std::string>    extensionNames;
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

    //! Constructor to initialize the descriptor with the module name from an std::string.
    inline RenderSystemDescriptor(const std::string& moduleName) :
        moduleName { moduleName }
    {
    }

    //! Constructor to initialize the descriptor with the module name from a null terminated string.
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
    If LLGL was built with the \c LLGL_BUILD_STATIC_LIB option, this member is ignored.
    */
    std::string         moduleName;

    /**
    \brief Render system flags. This can be a bitwise OR combination of RenderSystemFlags entries. By default 0.
    \remarks Use this to create a native debug layer for the device context.
    \see RenderSystemFlags
    */
    long                flags               = 0;

    /**
    \brief profiler Optional pointer to a rendering profiler. This is only supported if LLGL was compiled with the \c LLGL_ENABLE_DEBUG_LAYER flag.
    \remarks If this is used, the counters of the profiler must be reset manually.
    */
    RenderingProfiler*  profiler            = nullptr;

    /**
    \brief debugger Optional pointer to a rendering debugger. This is only supported if LLGL was compiled with the \c LLGL_ENABLE_DEBUG_LAYER flag.
    \remarks If the default debugger is used (i.e. no sub class of RenderingDebugger), then all reports will be send to the Log.
    In order to see any reports from the Log, use either Log::RegisterCallback or Log::RegisterCallbackStd.
    */
    RenderingDebugger*  debugger            = nullptr;

    /**
    \brief Optional raw pointer to a renderer specific configuration structure.
    \remarks This can be used to pass some refinement configurations to the render system when the module is loaded.
    Example usage (for Vulkan renderer):
    \code
    // Initialize Vulkan specific configurations (e.g. always allocate at least 1GB of VRAM for each device memory chunk).
    LLGL::RendererConfigurationVulkan config;
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
    \see RendererConfigurationVulkan
    \see RendererConfigurationOpenGL
    \see RendererConfigurationOpenGLES3
    */
    const void*         rendererConfig      = nullptr;

    /**
    \brief Specifies the size (in bytes) of the structure where the \c rendererConfig member points to (use \c sizeof with the respective structure). By default 0.
    \remarks If \c rendererConfig is null then this member is ignored.
    \see rendererConfig
    */
    std::size_t         rendererConfigSize  = 0;

    #ifdef LLGL_OS_ANDROID

    /**
    \brief Android specific application descriptor. This descriptor is defined by the "native app glue" from the Android NDK.
    \remarks This \b must be specified when compiling for the Android platform.
    \remarks Here is an example for the main entry point on Android:
    \code
    #include <LLGL/LLGL.h>

    ...

    void MyMain(const LLGL::RenderSystemDescriptor& desc)
    {
       myRenderSystem = LLGL::RenderSystem::Load(desc);
       ...
    }

    #if defined LLGL_OS_ANDROID

    // Android specific main function
    void android_main(android_app* state)
    {
        LLGL::RenderSystemDescriptor desc{ "OpenGLES3" };
        desc.androidApp = state;
        MyMain(desc);
    }

    #else

    // Standard C/C++ main function
    int main()
    {
        MyMain("OpenGL");
        return 0;
    }

    #endif
    \endcode
    \note Only supported on: Android.
    */
    android_app*    androidApp;

    #endif
};

/**
\brief Contains the attributes for all supported rendering features.
\see RenderingCapabilities::features
*/
struct RenderingFeatures
{
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
    \remarks This implies RenderingFeatures::hasCubeTextures to be true as well.
    \see TextureType::TextureCubeArray
    */
    bool hasCubeArrayTextures           = false;

    /**
    \brief Specifies whether multi-sample textures are supported.
    \see TextureType::Texture2DMS
    */
    bool hasMultiSampleTextures         = false;

    /**
    \brief Specifies whether multi-sample array textures are supported.
    \remarks This implies RenderingFeatures::hasMultiSampleTextures to be true as well.
    \see TextureType::Texture2DMSArray
    */
    bool hasMultiSampleArrayTextures    = false;

    /**
    \brief Specifies whether texture views are supported.
    \remarks Texture views can share their image data with another texture resource in a different range and format.
    \see TextureViewDescriptor
    */
    bool hasTextureViews                = false;

    /**
    \brief Specifies whether texture views can have swizzling (a.k.a. component mapping).
    \remarks This feature implies that \c hasTextureViews is true.
    \note Only supported with: Direct3D 12, Vulkan, OpenGL, Metal.
    \see TextureViewDescriptor::swizzle
    */
    bool hasTextureViewSwizzle          = false;

    /**
    \brief Specifies whether buffer views are supported.
    \remarks Buffer views can shared their data with another buffer resource in a different range and format.
    \see BufferViewDescriptor
    */
    bool hasBufferViews                 = false;

    //! Specifies whether samplers are supported.
    //! \todo Rename to \c hasNativeSamplerStates or remove entirely.
    bool hasSamplers                    = false;

    /**
    \brief Specifies whether constant buffers (also "uniform buffer objects") are supported.
    \see BindFlags::ConstantBuffer
    */
    bool hasConstantBuffers             = false;

    /**
    \brief Specifies whether storage buffers (also "read/write buffers") are supported.
    \see BindFlags::Sampled
    \see BindFlags::Storage
    */
    bool hasStorageBuffers              = false;

    /**
    \brief Specifies whether individual shader uniforms are supported.
    \note Only supported with: OpenGL.
    \see CommandBuffer::SetUniform
    \see CommandBuffer::SetUniforms
    */
    bool hasUniforms                    = false;

    /**
    \brief Specifies whether geometry shaders are supported.
    \see ShaderType::Geometry
    */
    bool hasGeometryShaders             = false;

    /**
    \brief Specifies whether tessellation shaders are supported.
    \remarks This feature implies that \c hasTessellatorStage is true.
    \note Only supported with: Direct3D 12, Direct3D 11, Vulkan, OpenGL.
    \see ShaderType::TessControl
    \see ShaderType::TessEvaluation
    */
    bool hasTessellationShaders         = false;

    /**
    \brief Specifies whether tessellator stage is supported.
    \remarks The Metal backend supports a tessellator stage but no dedicated tessellation shaders.
    The tessellation control shader and the tessellation evaluation shader are usually defined
    by a compute kernel and a post-tessellation vertex function respectively.
    \see TessellationDescriptor.
    */
    bool hasTessellatorStage            = false;

    /**
    \brief Specifies whether compute shaders are supported.
    \see ShaderType::Compute
    \see CommandBuffer::Dispatch
    \see CommandBuffer::DispatchIndirect
    */
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
    \brief Specifies whether indirect draw commands are supported.
    \see CommandBuffer::DrawIndirect
    \see CommandBuffer::DrawIndexedIndirect
    */
    bool hasIndirectDrawing             = false;

    /**
    \brief Specifies whether multiple viewports, depth-ranges, and scissors at once are supported.
    \see RenderingLimits::maxViewports
    */
    bool hasViewportArrays              = false;

    /**
    \brief Specifies whether conservative rasterization is supported.
    \see RasterizerDescriptor::conservativeRasterization
    */
    bool hasConservativeRasterization   = false;

    /**
    \brief Specifies whether stream-output is supported.
    \see VertexShaderAttributes::outputAttribs
    \see CommandBuffer::BeginStreamOutput
    \see RenderingLimits::maxStreamOutputs
    */
    bool hasStreamOutputs               = false;

    /**
    \brief Specifies whether logic fragment operations are supported.
    \note For Direct3D 11, feature level 11.1 is required.
    \see BlendDescriptor::logicOp
    */
    bool hasLogicOp                     = false;

    /**
    \brief Specifies whether queries for pipeline statistics are supported.
    \see QueryType::PipelineStatistics
    \see QueryPipelineStatistics
    */
    bool hasPipelineStatistics          = false;

    /**
    \brief Specifies whether queries for conditional rendering are supported.
    \see QueryHeapDescriptor::renderCondition
    \see CommandBuffer:BeginRenderCondition
    */
    bool hasRenderCondition             = false;
};

/**
\brief Contains all rendering limitations such as maximum buffer size, maximum texture resolution etc.
\see RenderingCapabilities::limits
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
    std::uint32_t   maxTextureArrayLayers               = 0;

    /**
    \brief Specifies the maximum number of color attachments for each render target.
    \remarks This value <b>must not</b> be greater than 8.
    \see RenderTargetDescriptor::attachments
    \see RenderPassDescriptor::colorAttachments
    \see BlendDescriptor::targets
    */
    std::uint32_t   maxColorAttachments                 = 0;

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
    std::uint32_t   maxComputeShaderWorkGroups[3]       = { 0, 0, 0 };

    //! Specifies the maximum work group size in a compute shader.
    std::uint32_t   maxComputeShaderWorkGroupSize[3]    = { 0, 0, 0 };

    /**
    \brief Specifies the maximum number of viewports and scissor rectangles the render system supports. Upper limit is specified by \c LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS.
    \see CommandBuffer::SetViewports
    \see CommandBuffer::SetScissors
    \see GraphicsPipelineDescriptor::viewports
    \see GraphicsPipelineDescriptor::scissors
    \see RenderingFeatures::hasViewportArrays
    */
    std::uint32_t   maxViewports                        = 0;

    /**
    \brief Specifies the maximum width and height of each viewport and scissor rectangle.
    \see Viewport::width
    \see Viewport::height
    \see Scissor::width
    \see Scissor::height
    */
    std::uint32_t   maxViewportSize[2]                  = { 0, 0 };

    /**
    \brief Specifies the maximum size (in bytes) that is supported for hardware buffers (vertex, index, storage buffers).
    \remarks Constant buffers are a special case for which \c maxConstantBufferSize can be used.
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

    /**
    \brief Specifies the maximum number of simultaneous stream-output buffers.
    \remarks This must not be larger than \c LLGL_MAX_NUM_SO_BUFFERS which is 4.
    \see CommandBuffer::BeginStreamOutput
    \see RenderingFeatures::hasStreamOutputs
    */
    std::uint32_t   maxStreamOutputs                    = 0;

    /**
    \brief Specifies the maximum tessellation factor.
    \remarks Metal for example supports 64 on macOS and 16 on iOS.
    \see TessellationDescriptor::maxTessFactor
    */
    std::uint32_t   maxTessFactor                       = 0;

    /**
    \brief Specifies the minimum alignment (in bytes) for Constant Buffer Views (CBV).
    \see BufferViewDescriptor::offset
    \see BufferViewDescriptor::size
    */
    std::uint64_t   minConstantBufferAlignment          = 0;

    /**
    \brief Specifies the minimum alignment (in bytes) for sampled buffers, aka. Shader Resource Views (SRV).
    \see BufferViewDescriptor::offset
    \see BufferViewDescriptor::size
    */
    std::uint64_t   minSampledBufferAlignment           = 0;

    /**
    \brief Specifies the minimum alignment (in bytes) for storage buffers, aka. Unordered Access Views (UAV).
    \see BufferViewDescriptor::offset
    \see BufferViewDescriptor::size
    */
    std::uint64_t   minStorageBufferAlignment           = 0;

    /**
    \brief Specifies the maximum number of samples for color buffers. Common values are 4, 8, 16, or 32.
    \remarks Most renderers will return at least a value of 4.
    \see RenderPassDescriptor::samples
    \see RenderTargetDescriptor::samples
    \see TextureDescriptor::samples
    */
    std::uint32_t   maxColorBufferSamples               = 0;

    /**
    \brief Specifies the maximum number of samples for depth buffers. Common values are 4, 8, 16, or 32.
    \remarks Most renderers will return at least a value of 4.
    \see RenderPassDescriptor::samples
    \see RenderTargetDescriptor::samples
    \see TextureDescriptor::samples
    */
    std::uint32_t   maxDepthBufferSamples               = 0;

    /**
    \brief Specifies the maximum number of samples for stencil buffers. Common values are 4, 8, 16, or 32.
    \remarks Most renderers will return at least a value of 4.
    \see RenderPassDescriptor::samples
    \see RenderTargetDescriptor::samples
    \see TextureDescriptor::samples
    */
    std::uint32_t   maxStencilBufferSamples             = 0;

    /**
    \brief Specifies the maximum number of samples for a RenderTarget with no attachments. Common values are 4, 8, 16, or 32.
    \remarks Most renderers will return at least a value of 4.
    \see RenderTargetDescriptor::samples
    */
    std::uint32_t   maxNoAttachmentSamples              = 0;
};

/**
\brief Structure with all attributes describing the rendering capabilities of the render system.
\see RenderSystem::GetRenderingCaps
*/
struct RenderingCapabilities
{
    /**
    \brief Screen coordinate system origin.
    \remarks This determines the native coordinate space of viewports, scissors, and framebuffers.
    If the native screen origin is lower-left, LLGL emulates it to always maintain the upper-left as the screen origin.
    */
    ScreenOrigin                    screenOrigin        = ScreenOrigin::UpperLeft;

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
    \brief Specifies the list of supported hardware texture formats.
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
myRequirements.features.hasStorageBuffers               = true;
myRequirements.features.hasComputeShaders               = true;
myRequirements.limits.maxComputeShaderWorkGroups[0]     = 1024;
myRequirements.limits.maxComputeShaderWorkGroups[1]     = 1024;
myRequirements.limits.maxComputeShaderWorkGroups[2]     = 1;
myRequirements.limits.maxComputeShaderWorkGroupSize[0]  = 8;
myRequirements.limits.maxComputeShaderWorkGroupSize[1]  = 8;
myRequirements.limits.maxComputeShaderWorkGroupSize[2]  = 8;

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
    const ValidateRenderingCapsFunc&    callback        = {}
);


} // /namespace LLGL


#endif



// ================================================================================
