/*
 * D3D9EmulatedSampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9EmulatedSampler.h"
#include "../D3D9Core.h"
#include "../D3D9Types.h"
#include "../RenderState/D3D9StateManager.h"


namespace LLGL
{


static void ConvertSamplerDescToD3D9SamplerState(D3D9SamplerState& dst, const SamplerDescriptor& src)
{
    dst.addressU        = D3D9Types::ToD3DTextureAddress(src.addressModeU);
    dst.addressV        = D3D9Types::ToD3DTextureAddress(src.addressModeV);
    dst.addressW        = D3D9Types::ToD3DTextureAddress(src.addressModeW);
    dst.borderColor     = D3D9Types::ToD3DColor(src.borderColor);
    dst.mipFilter       = (src.mipMapEnabled ? D3D9Types::ToD3DTextureFilter(src.mipMapFilter) : D3DTEXF_NONE);
    dst.mipMapLodBias   = FloatToDWORD(src.mipMapLODBias);
    dst.maxMipLevel     = static_cast<DWORD>(src.minLOD); // `minLOD` maps to D3D9's `D3DSAMP_MAXMIPLEVEL` as it describes the LOD index of the larges MIP-map
    dst.maxAnisotropy   = static_cast<DWORD>(src.maxAnisotropy);

    if (src.maxAnisotropy > 1)
    {
        dst.magFilter = D3DTEXF_ANISOTROPIC;
        dst.minFilter = D3DTEXF_ANISOTROPIC;
    }
    else
    {
        dst.magFilter = D3D9Types::ToD3DTextureFilter(src.magFilter);
        dst.minFilter = D3D9Types::ToD3DTextureFilter(src.minFilter);
    }
}

D3D9EmulatedSampler::D3D9EmulatedSampler(const SamplerDescriptor& desc)
{
    ConvertSamplerDescToD3D9SamplerState(d3dState_, desc);
}

void D3D9EmulatedSampler::SetDebugName(const char* /*name*/)
{
    // dummy
}

bool D3D9EmulatedSampler::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
