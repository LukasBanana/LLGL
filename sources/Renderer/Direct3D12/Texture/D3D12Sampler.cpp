/*
 * D3D12Sampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Sampler.h"
#include "../Shader/D3D12RootParameter.h"
#include "../D3D12Types.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/Direct3D12/NativeHandle.h>


namespace LLGL
{


D3D12Sampler::D3D12Sampler(const SamplerDescriptor& desc)
{
    D3D12Sampler::ConvertDesc(nativeDesc_, desc);
}

bool D3D12Sampler::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleD3D = GetTypedNativeHandle<Direct3D12::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleD3D->type                       = Direct3D12::ResourceNativeType::SamplerDescriptor;
        nativeHandleD3D->samplerDesc.samplerDesc    = nativeDesc_;
        return true;
    }
    return false;
}

void D3D12Sampler::CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
{
    device->CreateSampler(&nativeDesc_, cpuDescriptorHandle);
}

template <typename TNativeSamplerDesc>
static void D3DConvertBaseSamplerDesc(TNativeSamplerDesc& outDesc, const SamplerDescriptor& inDesc)
{
    /* Translate and store to native sampler desctiptor */
    outDesc.Filter          = D3D12Types::Map(inDesc);
    outDesc.AddressU        = D3D12Types::Map(inDesc.addressModeU);
    outDesc.AddressV        = D3D12Types::Map(inDesc.addressModeV);
    outDesc.AddressW        = D3D12Types::Map(inDesc.addressModeW);
    outDesc.MipLODBias      = inDesc.mipMapLODBias;
    outDesc.MaxAnisotropy   = inDesc.maxAnisotropy;
    outDesc.ComparisonFunc  = (inDesc.compareEnabled ? D3D12Types::Map(inDesc.compareOp) : D3D12_COMPARISON_FUNC_NEVER);

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

void D3D12Sampler::ConvertDesc(D3D12_SAMPLER_DESC& outDesc, const SamplerDescriptor& inDesc)
{
    D3DConvertBaseSamplerDesc(outDesc, inDesc);

    /* Set dynamic sampler border color */
    outDesc.BorderColor[0] = inDesc.borderColor[0];
    outDesc.BorderColor[1] = inDesc.borderColor[1];
    outDesc.BorderColor[2] = inDesc.borderColor[2];
    outDesc.BorderColor[3] = inDesc.borderColor[3];
}

static D3D12_STATIC_BORDER_COLOR GetD3DBorderColor(const float (&color)[4])
{
    switch (GetStaticSamplerBorderColor(color))
    {
        default:
        case StaticSamplerBorderColor::TransparentBlack:    return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        case StaticSamplerBorderColor::OpaqueBlack:         return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        case StaticSamplerBorderColor::OpaqueWhite:         return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    }
}

void D3D12Sampler::ConvertDesc(D3D12_STATIC_SAMPLER_DESC& outDesc, const StaticSamplerDescriptor& inDesc)
{
    D3DConvertBaseSamplerDesc(outDesc, inDesc.sampler);

    /* Set static sampler border color */
    outDesc.BorderColor         = GetD3DBorderColor(inDesc.sampler.borderColor);

    /* Set static sampler binding point */
    outDesc.ShaderRegister      = inDesc.slot.index;
    outDesc.RegisterSpace       = inDesc.slot.set;
    outDesc.ShaderVisibility    = D3D12RootParameter::FindSuitableVisibility(inDesc.stageFlags);
}


} // /namespace LLGL



// ================================================================================
