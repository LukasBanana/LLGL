/*
 * WGSLResourceReflection.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SL_RESOURCE_REFLECTION_H
#define LLGL_WG_SL_RESOURCE_REFLECTION_H


#include <LLGL/Report.h>
#include <vector>
#include <map>
#include <string>
#include <cstddef>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct WGSLResourceType
{
    WGPUTextureViewDimension    textureViewDimension    = WGPUTextureViewDimension_Undefined;
    WGPUTextureSampleType       textureSampleType       = WGPUTextureSampleType_Undefined;
    WGPUTextureFormat           storageTextureFormat    = WGPUTextureFormat_Undefined;
    WGPUStorageTextureAccess    storageTextureAccess    = WGPUStorageTextureAccess_Undefined;
    WGPUSamplerBindingType      samplerBindingType      = WGPUSamplerBindingType_Undefined;
    WGPUBool                    multisampled            = WGPU_FALSE;
};

class WGSLResourceReflection
{

    public:

        bool Reflect(StringView sourceWGSL, Report* outReport = nullptr);

        const WGSLResourceType* FindResource(const char* name) const;

    private:

        std::vector<WGSLResourceType>       resourceTypes_;
        std::map<std::string, std::size_t>  resources_;
        std::map<std::string, std::size_t>  typeAliases_;

};


} // /namespace LLGL


#endif



// ================================================================================
