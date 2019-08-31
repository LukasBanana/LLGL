/*
 * D3D12SamplerDesc.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12SamplerDesc.h"


namespace LLGL
{


D3D12SamplerDesc::D3D12SamplerDesc()
{
    Filter          = D3D12_FILTER_ANISOTROPIC;
    AddressU        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    AddressV        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    AddressW        = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    MipLODBias      = 0.0f;
    MaxAnisotropy   = 16;
    ComparisonFunc  = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    BorderColor[0]  = 1.0f;
    BorderColor[1]  = 1.0f;
    BorderColor[2]  = 1.0f;
    BorderColor[3]  = 1.0f;
    MinLOD          = 0.0f;
    MaxLOD          = D3D12_FLOAT32_MAX;
}

void D3D12SamplerDesc::SetTextureAddressModes(D3D12_TEXTURE_ADDRESS_MODE addressMode)
{
    AddressU = addressMode;
    AddressV = addressMode;
    AddressW = addressMode;
}

void D3D12SamplerDesc::SetBorderColor(const ColorRGBAf& color)
{
    BorderColor[0] = color.r;
    BorderColor[1] = color.g;
    BorderColor[2] = color.b;
    BorderColor[3] = color.a;
}


} // /namespace LLGL



// ================================================================================
