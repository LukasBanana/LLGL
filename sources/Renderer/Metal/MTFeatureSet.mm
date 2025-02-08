/*
 * MTFeatureSet.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTFeatureSet.h"
#include "MTDevice.h"
#include "OSXAvailability.h"
#include <LLGL/ShaderFlags.h>
#include <AvailabilityMacros.h>
#include <initializer_list>
#include <algorithm>


namespace LLGL
{


// Converts the specified Metal feature set into a fixed version number
static int FeatureSetToVersion(MTLFeatureSet fset)
{
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_11
    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v1)
        return 101; // 1.1
    #endif
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v2)
        return 102; // 1.2
    #endif
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v3)
        return 103; // 1.3
    #endif
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v4)
        return 104; // 1.4
    if (fset >= MTLFeatureSet_macOS_GPUFamily2_v1)
        return 201; // 2.1
    #endif
    return 100; // 1.0
}

static std::vector<Format> GetDefaultSupportedMTTextureFormats()
{
    return
    {
        Format::A8UNorm,
        Format::R8UNorm,            Format::R8SNorm,            Format::R8UInt,             Format::R8SInt,
        Format::R16UNorm,           Format::R16SNorm,           Format::R16UInt,            Format::R16SInt,            Format::R16Float,
        Format::R32UInt,            Format::R32SInt,            Format::R32Float,
        Format::RG8UNorm,           Format::RG8SNorm,           Format::RG8UInt,            Format::RG8SInt,
        Format::RG16UNorm,          Format::RG16SNorm,          Format::RG16UInt,           Format::RG16SInt,           Format::RG16Float,
        Format::RG32UInt,           Format::RG32SInt,           Format::RG32Float,
        Format::RGBA8UNorm,         Format::RGBA8UNorm_sRGB,    Format::RGBA8SNorm,         Format::RGBA8UInt,          Format::RGBA8SInt,
        Format::RGBA16UNorm,        Format::RGBA16SNorm,        Format::RGBA16UInt,         Format::RGBA16SInt,         Format::RGBA16Float,
        Format::RGBA32UInt,         Format::RGBA32SInt,         Format::RGBA32Float,
        Format::BGRA8UNorm,         Format::BGRA8UNorm_sRGB,
        Format::RGB10A2UNorm,       Format::RGB10A2UInt,        Format::RG11B10Float,       Format::RGB9E5Float,

        Format::D16UNorm,           Format::D24UNormS8UInt,     Format::D32Float,           Format::D32FloatS8X24UInt,

        #ifndef LLGL_OS_IOS
        Format::BC1UNorm,           Format::BC1UNorm_sRGB,
        Format::BC2UNorm,           Format::BC2UNorm_sRGB,
        Format::BC3UNorm,           Format::BC3UNorm_sRGB,
        Format::BC4UNorm,           Format::BC4SNorm,
        Format::BC5UNorm,           Format::BC5SNorm,
        #endif

        Format::ASTC4x4,            Format::ASTC4x4_sRGB,
        Format::ASTC5x4,            Format::ASTC5x4_sRGB,
        Format::ASTC5x5,            Format::ASTC5x5_sRGB,
        Format::ASTC6x5,            Format::ASTC6x5_sRGB,
        Format::ASTC6x6,            Format::ASTC6x6_sRGB,
        Format::ASTC8x5,            Format::ASTC8x5_sRGB,
        Format::ASTC8x6,            Format::ASTC8x6_sRGB,
        Format::ASTC8x8,            Format::ASTC8x8_sRGB,
        Format::ASTC10x5,           Format::ASTC10x5_sRGB,
        Format::ASTC10x6,           Format::ASTC10x6_sRGB,
        Format::ASTC10x8,           Format::ASTC10x8_sRGB,
        Format::ASTC10x10,          Format::ASTC10x10_sRGB,
        Format::ASTC12x10,          Format::ASTC12x10_sRGB,
        Format::ASTC12x12,          Format::ASTC12x12_sRGB,

        Format::ETC2UNorm,          Format::ETC2UNorm_sRGB,
    };
}

static NSUInteger GetMaxMTBufferSize(id<MTLDevice> device)
{
    /* Assume minimum of 256 MB (268,435,456 bytes) if maxBufferLength is not supported */
    constexpr NSUInteger minBufferSize256MB = 1024 * 1024 * 256;
    if (@available(iOS 12.0, macOS 10.14, *))
        return [device maxBufferLength];
    else
        return minBufferSize256MB;
}

// see https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps)
{
    /* Set clipping origin and depth range */
    caps.screenOrigin   = ScreenOrigin::UpperLeft;
    caps.clippingRange  = ClippingRange::ZeroToOne;

    /* Query supported hardware texture formats */
    caps.textureFormats = GetDefaultSupportedMTTextureFormats();

    /* Specify supported shading languages */
    const int version = FeatureSetToVersion(fset);

    caps.shadingLanguages = { ShadingLanguage::Metal, ShadingLanguage::Metal_1_0 };

    if (version >= 101)
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_1_1);
    if (version >= 102)
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_1_2);
    if (version >= 201)
    {
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_2_0);
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_2_1);
    }

    /* Specify features */
    auto& features = caps.features;

    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = (version >= 101);
    features.hasMultiSampleTextures         = true;
    features.hasMultiSampleArrayTextures    = false;
    features.hasTextureViews                = true;
    features.hasTextureViewSwizzle          = LLGL_OSX_AVAILABLE(macOS 13.0, iOS 16.0, *);
    features.hasTextureViewFormatSwizzle    = true;
    features.hasBufferViews                 = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = false;
    features.hasTessellatorStage            = true;
    features.hasComputeShaders              = true;
    features.hasInstancing                  = true;
    features.hasOffsetInstancing            = true;
    features.hasIndirectDrawing             = true;
    features.hasViewportArrays              = (version >= 103);
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = false;

    /* Specify limits */
    auto& limits = caps.limits;

    limits.maxBufferSize                    = GetMaxMTBufferSize(device);
    limits.maxConstantBufferSize            = 65536u;
    limits.max1DTextureSize                 = 16384u;
    limits.max2DTextureSize                 = 16384u;
    limits.max3DTextureSize                 = 2048u;
    limits.maxCubeTextureSize               = 16384u;
    limits.maxTextureArrayLayers            = 2048u;
    limits.maxViewports                     = (version >= 103 ? 16u : 1u);
    limits.maxViewportSize[0]               = 16384u; //???
    limits.maxViewportSize[1]               = 16384u; //???
    limits.maxColorAttachments              = 8u;

    limits.maxComputeShaderWorkGroups[0]    = 512u; //???
    limits.maxComputeShaderWorkGroups[1]    = 512u; //???
    limits.maxComputeShaderWorkGroups[2]    = 512u; //???

    MTLSize workGroupSize = [device maxThreadsPerThreadgroup];
    limits.maxComputeShaderWorkGroupSize[0] = static_cast<std::uint32_t>(workGroupSize.width);
    limits.maxComputeShaderWorkGroupSize[1] = static_cast<std::uint32_t>(workGroupSize.height);
    limits.maxComputeShaderWorkGroupSize[2] = static_cast<std::uint32_t>(workGroupSize.depth);

    #ifdef LLGL_OS_IOS
    limits.maxTessFactor                    = 16u;
    #else
    limits.maxTessFactor                    = 64u;
    #endif

    caps.limits.minConstantBufferAlignment  = 256u; //???
    caps.limits.minSampledBufferAlignment   = 32u;  //???
    caps.limits.minStorageBufferAlignment   = 32u;  //???

    /*
    Determine maximum number of samples for multi-sampled textures.
    All devices support at least 4, but only some devices supported up to 8 samples.
    */
    const NSUInteger maxSamples = MTDevice::FindSuitableSampleCount(device, 8u);
    caps.limits.maxColorBufferSamples       = static_cast<std::uint32_t>(maxSamples);
    caps.limits.maxDepthBufferSamples       = static_cast<std::uint32_t>(maxSamples);
    caps.limits.maxStencilBufferSamples     = static_cast<std::uint32_t>(maxSamples);
    caps.limits.maxNoAttachmentSamples      = static_cast<std::uint32_t>(maxSamples);

    caps.limits.storageResourceStageFlags   = StageFlags::AllStages;
}


} // /namespace LLGL



// ================================================================================
