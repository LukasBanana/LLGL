/*
 * D3D12Sampler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Sampler.h"
#include "../D3D12Types.h"


namespace LLGL
{


D3D12Sampler::D3D12Sampler(const SamplerDescriptor& desc)
{
    /* Translate and store to native sampler desctiptor */
    nativeDesc_.Filter          = D3D12Types::Map(desc);
    nativeDesc_.AddressU        = D3D12Types::Map(desc.addressModeU);
    nativeDesc_.AddressV        = D3D12Types::Map(desc.addressModeV);
    nativeDesc_.AddressW        = D3D12Types::Map(desc.addressModeW);
    nativeDesc_.MipLODBias      = desc.mipMapLODBias;
    nativeDesc_.MaxAnisotropy   = desc.maxAnisotropy;
    nativeDesc_.ComparisonFunc  = (desc.compareEnabled ? D3D12Types::Map(desc.compareOp) : D3D12_COMPARISON_FUNC_ALWAYS);
    nativeDesc_.BorderColor[0]  = desc.borderColor.r;
    nativeDesc_.BorderColor[1]  = desc.borderColor.g;
    nativeDesc_.BorderColor[2]  = desc.borderColor.b;
    nativeDesc_.BorderColor[3]  = desc.borderColor.a;

    if (desc.mipMapping)
    {
        nativeDesc_.MinLOD = desc.minLOD;
        nativeDesc_.MaxLOD = desc.maxLOD;
    }
    else
    {
        nativeDesc_.MinLOD = 0.0f;
        nativeDesc_.MaxLOD = 0.0f;
    }
}

void D3D12Sampler::CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
{
    device->CreateSampler(&nativeDesc_, cpuDescriptorHandle);
}


} // /namespace LLGL



// ================================================================================
