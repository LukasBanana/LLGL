/*
 * D3D11Sampler.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Sampler.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D11Sampler::D3D11Sampler(ID3D11Device* device, const SamplerDescriptor& desc)
{
    /* Setup sampler state descriptor and create sampler state object */
    D3D11_SAMPLER_DESC nativeDesc;
    D3D11Sampler::ConvertDesc(nativeDesc, desc);
    auto hr = device->CreateSamplerState(&nativeDesc, native_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11SamplerState");
}

void D3D11Sampler::SetName(const char* name)
{
    D3D11SetObjectName(native_.Get(), name);
}

void D3D11Sampler::ConvertDesc(D3D11_SAMPLER_DESC& outDesc, const SamplerDescriptor& inDesc)
{
    outDesc.Filter          = D3D11Types::Map(inDesc);
    outDesc.AddressU        = D3D11Types::Map(inDesc.addressModeU);
    outDesc.AddressV        = D3D11Types::Map(inDesc.addressModeV);
    outDesc.AddressW        = D3D11Types::Map(inDesc.addressModeW);
    outDesc.MipLODBias      = inDesc.mipMapLODBias;
    outDesc.MaxAnisotropy   = inDesc.maxAnisotropy;
    outDesc.ComparisonFunc  = (inDesc.compareEnabled ? D3D11Types::Map(inDesc.compareOp) : D3D11_COMPARISON_ALWAYS);
    outDesc.BorderColor[0]  = inDesc.borderColor[0];
    outDesc.BorderColor[1]  = inDesc.borderColor[1];
    outDesc.BorderColor[2]  = inDesc.borderColor[2];
    outDesc.BorderColor[3]  = inDesc.borderColor[3];

    if (inDesc.mipMapEnabled)
    {
        outDesc.MinLOD = inDesc.minLOD;
        outDesc.MaxLOD = inDesc.maxLOD;
    }
    else
    {
        outDesc.MinLOD = 0.0f;
        outDesc.MaxLOD = 0.0f;
    }
}


} // /namespace LLGL



// ================================================================================
