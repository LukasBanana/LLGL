/*
 * RenderSystemFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDER_SYSTEM_FLAGS_H
#define LLGL_C99_RENDER_SYSTEM_FLAGS_H


#include <LLGL-C/Format.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


#define LLGL_RENDERERID_UNDEFINED   ( 0x00000000 )

#define LLGL_RENDERERID_NULL        ( 0x00000001 )
#define LLGL_RENDERERID_OPENGL      ( 0x00000002 )
#define LLGL_RENDERERID_OPENGLES1   ( 0x00000003 )
#define LLGL_RENDERERID_OPENGLES2   ( 0x00000004 )
#define LLGL_RENDERERID_OPENGLES3   ( 0x00000005 )
#define LLGL_RENDERERID_DIRECT3D9   ( 0x00000006 )
#define LLGL_RENDERERID_DIRECT3D10  ( 0x00000007 )
#define LLGL_RENDERERID_DIRECT3D11  ( 0x00000008 )
#define LLGL_RENDERERID_DIRECT3D12  ( 0x00000009 )
#define LLGL_RENDERERID_VULKAN      ( 0x0000000a )
#define LLGL_RENDERERID_METAL       ( 0x0000000b )

#define LLGL_RENDERERID_RESERVED    ( 0x000000ff )


/* ----- Enumerations ----- */

typedef enum LLGLShadingLanguage
{
    LLGLShadingLanguageGLSL            = (0x10000),        //!< GLSL (OpenGL Shading Language).
    LLGLShadingLanguageGLSL_110        = (0x10000 | 110),  //!< GLSL 1.10 (since OpenGL 2.0).
    LLGLShadingLanguageGLSL_120        = (0x10000 | 120),  //!< GLSL 1.20 (since OpenGL 2.1).
    LLGLShadingLanguageGLSL_130        = (0x10000 | 130),  //!< GLSL 1.30 (since OpenGL 3.0).
    LLGLShadingLanguageGLSL_140        = (0x10000 | 140),  //!< GLSL 1.40 (since OpenGL 3.1).
    LLGLShadingLanguageGLSL_150        = (0x10000 | 150),  //!< GLSL 1.50 (since OpenGL 3.2).
    LLGLShadingLanguageGLSL_330        = (0x10000 | 330),  //!< GLSL 3.30 (since OpenGL 3.3).
    LLGLShadingLanguageGLSL_400        = (0x10000 | 400),  //!< GLSL 4.00 (since OpenGL 4.0).
    LLGLShadingLanguageGLSL_410        = (0x10000 | 410),  //!< GLSL 4.10 (since OpenGL 4.1).
    LLGLShadingLanguageGLSL_420        = (0x10000 | 420),  //!< GLSL 4.20 (since OpenGL 4.2).
    LLGLShadingLanguageGLSL_430        = (0x10000 | 430),  //!< GLSL 4.30 (since OpenGL 4.3).
    LLGLShadingLanguageGLSL_440        = (0x10000 | 440),  //!< GLSL 4.40 (since OpenGL 4.4).
    LLGLShadingLanguageGLSL_450        = (0x10000 | 450),  //!< GLSL 4.50 (since OpenGL 4.5).
    LLGLShadingLanguageGLSL_460        = (0x10000 | 460),  //!< GLSL 4.60 (since OpenGL 4.6).

    LLGLShadingLanguageESSL            = (0x20000),        //!< ESSL (OpenGL ES Shading Language).
    LLGLShadingLanguageESSL_100        = (0x20000 | 100),  //!< ESSL 1.00 (since OpenGL ES 2.0).
    LLGLShadingLanguageESSL_300        = (0x20000 | 300),  //!< ESSL 3.00 (since OpenGL ES 3.0).
    LLGLShadingLanguageESSL_310        = (0x20000 | 310),  //!< ESSL 3.10 (since OpenGL ES 3.1).
    LLGLShadingLanguageESSL_320        = (0x20000 | 320),  //!< ESSL 3.20 (since OpenGL ES 3.2).

    LLGLShadingLanguageHLSL            = (0x30000),        //!< HLSL (High Level Shading Language).
    LLGLShadingLanguageHLSL_2_0        = (0x30000 | 200),  //!< HLSL 2.0 (since Direct3D 9).
    LLGLShadingLanguageHLSL_2_0a       = (0x30000 | 201),  //!< HLSL 2.0a (since Direct3D 9a).
    LLGLShadingLanguageHLSL_2_0b       = (0x30000 | 202),  //!< HLSL 2.0b (since Direct3D 9b).
    LLGLShadingLanguageHLSL_3_0        = (0x30000 | 300),  //!< HLSL 3.0 (since Direct3D 9c).
    LLGLShadingLanguageHLSL_4_0        = (0x30000 | 400),  //!< HLSL 4.0 (since Direct3D 10).
    LLGLShadingLanguageHLSL_4_1        = (0x30000 | 410),  //!< HLSL 4.1 (since Direct3D 10.1).
    LLGLShadingLanguageHLSL_5_0        = (0x30000 | 500),  //!< HLSL 5.0 (since Direct3D 11).
    LLGLShadingLanguageHLSL_5_1        = (0x30000 | 510),  //!< HLSL 5.1 (since Direct3D 11.3).
    LLGLShadingLanguageHLSL_6_0        = (0x30000 | 600),  //!< HLSL 6.0 (since Direct3D 12). Shader model 6.0 adds wave intrinsics and 64-bit integer types to HLSL.
    LLGLShadingLanguageHLSL_6_1        = (0x30000 | 601),  //!< HLSL 6.1 (since Direct3D 12). Shader model 6.1 adds \c SV_ViewID and \c SV_Barycentrics semantics to HLSL.
    LLGLShadingLanguageHLSL_6_2        = (0x30000 | 602),  //!< HLSL 6.2 (since Direct3D 12). Shader model 6.2 adds 16-bit scalar types to HLSL.
    LLGLShadingLanguageHLSL_6_3        = (0x30000 | 603),  //!< HLSL 6.3 (since Direct3D 12). Shader model 6.3 adds ray tracing (DXR) to HLSL.
    LLGLShadingLanguageHLSL_6_4        = (0x30000 | 604),  //!< HLSL 6.4 (since Direct3D 12). Shader model 6.4 adds machine learning intrinsics to HLSL.

    LLGLShadingLanguageMetal           = (0x40000),        //!< Metal Shading Language.
    LLGLShadingLanguageMetal_1_0       = (0x40000 | 100),  //!< Metal 1.0 (since iOS 8.0).
    LLGLShadingLanguageMetal_1_1       = (0x40000 | 110),  //!< Metal 1.1 (since iOS 9.0 and OS X 10.11).
    LLGLShadingLanguageMetal_1_2       = (0x40000 | 120),  //!< Metal 1.2 (since iOS 10.0 and macOS 10.12).
    LLGLShadingLanguageMetal_2_0       = (0x40000 | 200),  //!< Metal 2.0 (since iOS 11.0 and macOS 10.13).
    LLGLShadingLanguageMetal_2_1       = (0x40000 | 210),  //!< Metal 2.1 (since iOS 12.0 and macOS 10.14).

    LLGLShadingLanguageSPIRV           = (0x50000),        //!< SPIR-V Shading Language.
    LLGLShadingLanguageSPIRV_100       = (0x50000 | 100),  //!< SPIR-V 1.0.

    LLGLShadingLanguageVersionBitmask  = 0x0000ffff,       //!< Bitmask for the version number of each shading language enumeration entry.
}
LLGLShadingLanguage;

typedef enum LLGLScreenOrigin
{
    LLGLScreenOriginLowerLeft,
    LLGLScreenOriginUpperLeft,
}
LLGLScreenOrigin;

typedef enum LLGLClippingRange
{
    LLGLClippingRangeMinusOneToOne,
    LLGLClippingRangeZeroToOne,
}
LLGLClippingRange;

typedef enum LLGLCPUAccess
{
    LLGLCPUAccessReadOnly,
    LLGLCPUAccessWriteOnly,
    LLGLCPUAccessWriteDiscard,
    LLGLCPUAccessReadWrite,
}
LLGLCPUAccess;


/* ----- Flags ----- */

enum LLGLRenderSystemFlags
{
    LLGLRenderSystemDebugDevice = (1 << 0),
};


/* ----- Structures ----- */

typedef struct LLGLRendererInfo
{
    const char*         rendererName;
    const char*         deviceName;
    const char*         vendorName;
    const char*         shadingLanguageName;
    size_t              numExtensionNames;
    const char* const*  extensionNames;
}
LLGLRendererInfo;

typedef struct LLGLRenderSystemDescriptor
{
    const char*             moduleName;
    long                    flags;
    //LLGLRenderingProfiler*  profiler;
    //LLGLRenderingDebugger*  debugger;
    const void*             rendererConfig;
    size_t                  rendererConfigSize;

    #ifdef LLGL_OS_ANDROID
    android_app*            androidApp;
    #endif
}
LLGLRenderSystemDescriptor;

typedef struct LLGLRenderingFeatures
{
    bool hasRenderTargets;
    bool has3DTextures;
    bool hasCubeTextures;
    bool hasArrayTextures;
    bool hasCubeArrayTextures;
    bool hasMultiSampleTextures;
    bool hasMultiSampleArrayTextures;
    bool hasTextureViews;
    bool hasTextureViewSwizzle;
    bool hasBufferViews;
    bool hasSamplers;
    bool hasConstantBuffers;
    bool hasStorageBuffers;
    bool hasUniforms;
    bool hasGeometryShaders;
    bool hasTessellationShaders;
    bool hasTessellatorStage;
    bool hasComputeShaders;
    bool hasInstancing;
    bool hasOffsetInstancing;
    bool hasIndirectDrawing;
    bool hasViewportArrays;
    bool hasConservativeRasterization;
    bool hasStreamOutputs;
    bool hasLogicOp;
    bool hasPipelineStatistics;
    bool hasRenderCondition;
}
LLGLRenderingFeatures;

typedef struct LLGLRenderingLimits
{
    float       lineWidthRange[2];
    uint32_t    maxTextureArrayLayers;
    uint32_t    maxColorAttachments;
    uint32_t    maxPatchVertices;
    uint32_t    max1DTextureSize;
    uint32_t    max2DTextureSize;
    uint32_t    max3DTextureSize;
    uint32_t    maxCubeTextureSize;
    uint32_t    maxAnisotropy;
    uint32_t    maxComputeShaderWorkGroups[3];
    uint32_t    maxComputeShaderWorkGroupSize[3];
    uint32_t    maxViewports;
    uint32_t    maxViewportSize[2];
    uint64_t    maxBufferSize;
    uint64_t    maxConstantBufferSize;
    uint32_t    maxStreamOutputs;
    uint32_t    maxTessFactor;
    uint64_t    minConstantBufferAlignment;
    uint64_t    minSampledBufferAlignment;
    uint64_t    minStorageBufferAlignment;
}
LLGLRenderingLimits;

typedef struct LLGLRenderingCapabilities
{
    LLGLScreenOrigin            screenOrigin;
    LLGLClippingRange           clippingRange;
    size_t                      numShadingLanguages;
    const LLGLShadingLanguage*  shadingLanguages;
    size_t                      numTextureFormats;
    const LLGLFormat*           textureFormats;
    LLGLRenderingFeatures       features;
    LLGLRenderingLimits         limits;
}
LLGLRenderingCapabilities;


#endif



// ================================================================================
