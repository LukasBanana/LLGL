/*
 * D3D11ResourceType.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
