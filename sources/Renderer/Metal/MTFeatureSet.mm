/*
 * MTFeatureSet.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTFeatureSet.h"
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
        Format::BC1UNorm,           Format::BC1UNorm_sRGB,
        Format::BC2UNorm,           Format::BC2UNorm_sRGB,
        Format::BC3UNorm,           Format::BC3UNorm_sRGB,
        Format::BC4UNorm,           Format::BC4SNorm,
        Format::BC5UNorm,           Format::BC5SNorm,
    };
}

// see https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps)
{
    caps.textureFormats = GetDefaultSupportedMTTextureFormats();

    auto& features = caps.features;
    auto& limits = caps.limits;

    const int version = FeatureSetToVersion(fset);

    /* Specify supported shading languages */
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
    features.hasDirectResourceBinding       = false;//true; //TODO: not supported yet
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = (version >= 101);
    features.hasMultiSampleTextures         = true;
    features.hasTextureViews                = true;
    features.hasTextureViewSwizzle          = true;
    features.hasBufferViews                 = true;
    features.hasSamplers                    = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasUniforms                    = false;
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
    limits.maxBufferSize                    = [device maxBufferLength];
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
}


} // /namespace LLGL



// ================================================================================
