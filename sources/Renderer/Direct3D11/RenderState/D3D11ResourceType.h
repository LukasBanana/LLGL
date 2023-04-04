/*
 * D3D11ResourceType.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_RESOURCE_TYPE_H
#define LLGL_D3D11_RESOURCE_TYPE_H


#include <cstdint>


namespace LLGL
{


// Internal enumeration for D3D resource heap segments.
enum D3DResourceType : std::uint32_t
{
    D3DResourceType_Invalid = 0,

    D3DResourceType_CBV,
    D3DResourceType_SRV,
    D3DResourceType_BufferSRV = D3DResourceType_SRV,
    D3DResourceType_TextureSRV,
    D3DResourceType_UAV,
    D3DResourceType_BufferUAV = D3DResourceType_UAV,
    D3DResourceType_TextureUAV,
    D3DResourceType_Sampler,
};


} // /namespace LLGL


#endif



// ================================================================================
