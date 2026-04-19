/*
 * WGResourceReflection.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_RESOURCE_REFLECTION_H
#define LLGL_WG_RESOURCE_REFLECTION_H


#include <LLGL/Report.h>
#include <vector>
#include <map>
#include <string>
#include <cstddef>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct WGResourceReflectionTable;

struct WGResourceReflection
{
    std::uint32_t               groupIndex              = 0;
    std::uint32_t               bindingIndex            = 0;
    WGPUTextureViewDimension    textureViewDimension    = WGPUTextureViewDimension_Undefined;
    WGPUTextureSampleType       textureSampleType       = WGPUTextureSampleType_BindingNotUsed;
    WGPUTextureFormat           storageTextureFormat    = WGPUTextureFormat_Undefined;
    WGPUStorageTextureAccess    storageTextureAccess    = WGPUStorageTextureAccess_BindingNotUsed;
    WGPUSamplerBindingType      samplerBindingType      = WGPUSamplerBindingType_BindingNotUsed;
    WGPUBool                    multisampled            = WGPU_FALSE;
};

bool ReflectWGSLShaderSource(WGResourceReflectionTable& outReflectionTable, StringView sourceWGSL, Report* outReport = nullptr);


} // /namespace LLGL


#endif



// ================================================================================
