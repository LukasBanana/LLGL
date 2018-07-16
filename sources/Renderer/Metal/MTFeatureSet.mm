/*
 * MTFeatureSet.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTFeatureSet.h"
#include <initializer_list>
#include <algorithm>


namespace LLGL
{


static bool AnyOf(MTLFeatureSet featureSet, const std::initializer_list<MTLFeatureSet>& featureSetList)
{
    return (std::find(featureSetList.begin(), featureSetList.end(), featureSet) != featureSetList.end());
}

void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps)
{
    auto& features = caps.features;
    auto& limits = caps.limits;
    
    /* Specify supported shading languages */
    caps.shadingLanguages =
    {
        ShadingLanguage::Metal,
        ShadingLanguage::Metal_1_0,
        ShadingLanguage::Metal_1_1,
        ShadingLanguage::Metal_1_2,
    };
    
    /* Specify features */
    features.hasCommandBufferExt            = true;
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = AnyOf(fset, { MTLFeatureSet_macOS_GPUFamily1_v1, MTLFeatureSet_macOS_GPUFamily1_v2, MTLFeatureSet_macOS_GPUFamily1_v3 });
    features.hasMultiSampleTextures         = true;
    features.hasSamplers                    = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = false;
    features.hasUniforms                    = false;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = AnyOf(fset, { MTLFeatureSet_macOS_GPUFamily1_v2, MTLFeatureSet_macOS_GPUFamily1_v3 });
    features.hasComputeShaders              = true;
    features.hasInstancing                  = true;
    features.hasOffsetInstancing            = true;
    features.hasViewportArrays              = AnyOf(fset, { MTLFeatureSet_macOS_GPUFamily1_v3 });
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = false;
    
    /* Specify limits */
    MTLSize workGroupSize = [device maxThreadsPerThreadgroup];
    limits.maxComputeShaderWorkGroupSize[0] = static_cast<std::uint32_t>(workGroupSize.width);
    limits.maxComputeShaderWorkGroupSize[1] = static_cast<std::uint32_t>(workGroupSize.height);
    limits.maxComputeShaderWorkGroupSize[2] = static_cast<std::uint32_t>(workGroupSize.depth);
}


} // /namespace LLGL



// ================================================================================
