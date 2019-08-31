/*
 * D3D12SamplerDesc.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_SAMPLER_DESC_H
#define LLGL_D3D12_SAMPLER_DESC_H


#include <LLGL/ColorRGBA.h>
#include <d3d12.h>


namespace LLGL
{


// Wrapper structure for D3D12_SAMPLER_DESC
struct D3D12SamplerDesc : public D3D12_SAMPLER_DESC
{
    // Default initializes the sampler descriptor to match with an HLSL-defined root signature static sampler
    D3D12SamplerDesc();

    void SetTextureAddressModes(D3D12_TEXTURE_ADDRESS_MODE addressMode);
    void SetBorderColor(const ColorRGBAf& color);
};


} // /namespace LLGL


#endif



// ================================================================================
