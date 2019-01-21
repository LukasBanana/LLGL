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


// see https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
void LoadFeatureSetCaps(id<MTLDevice> device, MTLFeatureSet fset, RenderingCapabilities& caps)
{
    auto& features = caps.features;
    auto& limits = caps.limits;
    
    /* Specify supported shading languages */
    caps.shadingLanguages = { ShadingLanguage::Metal, ShadingLanguage::Metal_1_0 };

    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v1)
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_1_1);
    if (fset >= MTLFeatureSet_macOS_GPUFamily1_v2)
        caps.shadingLanguages.push_back(ShadingLanguage::Metal_1_2);
    if (fset >= MTLFeatureSet_macOS_GPUFamily2_v1)
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
    features.hasCubeArrayTextures           = (fset >= MTLFeatureSet_macOS_GPUFamily1_v1);
    features.hasMultiSampleTextures         = true;
    features.hasSamplers                    = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasUniforms                    = false;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = (fset >= MTLFeatureSet_macOS_GPUFamily1_v2);
    features.hasComputeShaders              = true;
    features.hasInstancing                  = true;
    features.hasOffsetInstancing            = true;
    features.hasIndirectDrawing             = true;
    features.hasViewportArrays              = (fset >= MTLFeatureSet_macOS_GPUFamily1_v3);
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = false;
    features.hasIndirectDrawing             = (fset >= MTLFeatureSet_macOS_GPUFamily1_v2);
    
    /* Specify limits */
    limits.maxBufferSize                    = [device maxBufferLength];
    limits.maxConstantBufferSize            = 65536u;
    limits.max1DTextureSize                 = 16384u;
    limits.max2DTextureSize                 = 16384u;
    limits.max3DTextureSize                 = 2048u;
    limits.maxCubeTextureSize               = 16384u;
    limits.maxTextureArrayLayers            = 2048u;
    limits.maxViewports                     = (fset >= MTLFeatureSet_macOS_GPUFamily1_v3 ? 16u : 1u);
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
