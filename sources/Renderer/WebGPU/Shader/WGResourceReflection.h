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
    WGPUTextureViewDimension    textureViewDimension    = WGPUTextureViewDimension_Undefined;
    WGPUTextureSampleType       textureSampleType       = WGPUTextureSampleType_Undefined;
    WGPUTextureFormat           storageTextureFormat    = WGPUTextureFormat_Undefined;
    WGPUStorageTextureAccess    storageTextureAccess    = WGPUStorageTextureAccess_Undefined;
    WGPUSamplerBindingType      samplerBindingType      = WGPUSamplerBindingType_Undefined;
    WGPUBool                    multisampled            = WGPU_FALSE;
};

bool ReflectWGSLShaderSource(WGResourceReflectionTable& outReflectionTable, StringView sourceWGSL, Report* outReport = nullptr);


} // /namespace LLGL


#endif



// ================================================================================
