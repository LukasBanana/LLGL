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
    D3D11_SAMPLER_DESC samplerDesc;
    {
        samplerDesc.Filter          = D3D11Types::Map(desc);
        samplerDesc.AddressU        = D3D11Types::Map(desc.addressModeU);
        samplerDesc.AddressV        = D3D11Types::Map(desc.addressModeV);
        samplerDesc.AddressW        = D3D11Types::Map(desc.addressModeW);
        samplerDesc.MipLODBias      = desc.mipMapLODBias;
        samplerDesc.MaxAnisotropy   = desc.maxAnisotropy;
        samplerDesc.ComparisonFunc  = (desc.compareEnabled ? D3D11Types::Map(desc.compareOp) : D3D11_COMPARISON_ALWAYS);
        samplerDesc.BorderColor[0]  = desc.borderColor.r;
        samplerDesc.BorderColor[1]  = desc.borderColor.g;
        samplerDesc.BorderColor[2]  = desc.borderColor.b;
        samplerDesc.BorderColor[3]  = desc.borderColor.a;

        if (desc.mipMapping)
        {
            samplerDesc.MinLOD = desc.minLOD;
            samplerDesc.MaxLOD = desc.maxLOD;
        }
        else
        {
            samplerDesc.MinLOD = 0.0f;
            samplerDesc.MaxLOD = 0.0f;
        }
    }
    auto hr = device->CreateSamplerState(&samplerDesc, native_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 sampler state");
}

void D3D11Sampler::SetName(const char* name)
{
    D3D11SetObjectName(native_.Get(), name);
}


} // /namespace LLGL



// ================================================================================
