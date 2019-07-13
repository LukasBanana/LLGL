/*
 * MTFeatureSet.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

// see https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps)
{
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
    features.hasCommandBufferExt            = true;
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = (version >= 101);
    features.hasMultiSampleTextures         = true;
    features.hasSamplers                    = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasUniforms                    = false;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = (version >= 102);
    features.hasComputeShaders              = true;
    features.hasInstancing                  = true;
    features.hasOffsetInstancing            = true;
    features.hasIndirectDrawing             = true;
    features.hasViewportArrays              = (version >= 103);
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = false;
    features.hasIndirectDrawing             = (version >= 102);
    
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

    limits.maxComputeShaderWorkGroups[0]    = 512; //???
    limits.maxComputeShaderWorkGroups[1]    = 512; //???
    limits.maxComputeShaderWorkGroups[2]    = 512; //???

    MTLSize workGroupSize = [device maxThreadsPerThreadgroup];
    limits.maxComputeShaderWorkGroupSize[0] = static_cast<std::uint32_t>(workGroupSize.width);
    limits.maxComputeShaderWorkGroupSize[1] = static_cast<std::uint32_t>(workGroupSize.height);
    limits.maxComputeShaderWorkGroupSize[2] = static_cast<std::uint32_t>(workGroupSize.depth);
}


} // /namespace LLGL



// ================================================================================
