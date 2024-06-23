/*
 * D3D11PipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11PipelineLayout.h"
#include "D3D11ResourceType.h"
#include "D3D11StateManager.h"
#include "../Texture/D3D11Sampler.h"
#include "../../DXCommon/DXCore.h"
#include "../../ResourceUtils.h"


namespace LLGL
{


D3D11PipelineLayout::D3D11PipelineLayout(ID3D11Device* device, const PipelineLayoutDescriptor& desc) :
    heapBindings_ { GetExpandedHeapDescriptors(desc.heapBindings) },
    uniforms_     { desc.uniforms                                 }
{
    BuildDynamicResourceBindings(desc.bindings);
    BuildStaticSamplers(device, desc.staticSamplers);
}

std::uint32_t D3D11PipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size());
}

std::uint32_t D3D11PipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindings_.size());
}

std::uint32_t D3D11PipelineLayout::GetNumStaticSamplers() const
{
    return static_cast<std::uint32_t>(staticSamplers_.size());
}

std::uint32_t D3D11PipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniforms_.size());
}

void D3D11PipelineLayout::BindGraphicsStaticSamplers(D3D11StateManager& stateMngr) const
{
    /* Bind static samplers one-by-one to graphics pipeline */
    for (const D3D11StaticSampler& staticSampler : staticSamplers_)
        stateMngr.SetGraphicsStaticSampler(staticSampler);
}

void D3D11PipelineLayout::BindComputeStaticSamplers(D3D11StateManager& stateMngr) const
{
    /* Bind static samplers one-by-one to graphics pipeline */
    for (const D3D11StaticSampler& staticSampler : staticSamplers_)
        stateMngr.SetComputeStaticSampler(staticSampler);
}


/*
 * ======= Private: =======
 */

static D3DResourceType ToD3DResourceType(const BindingDescriptor& desc)
{
    switch (desc.type)
    {
        case ResourceType::Buffer:
            if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
                return D3DResourceType_CBV;
            if ((desc.bindFlags & BindFlags::Sampled) != 0)
                return D3DResourceType_BufferSRV;
            if ((desc.bindFlags & BindFlags::Storage) != 0)
                return D3DResourceType_BufferUAV;
            break;

        case ResourceType::Texture:
            if ((desc.bindFlags & BindFlags::Sampled) != 0)
                return D3DResourceType_TextureSRV;
            if ((desc.bindFlags & BindFlags::Storage) != 0)
                return D3DResourceType_TextureUAV;
            break;

        case ResourceType::Sampler:
            return D3DResourceType_Sampler;

        default:
            break;
    }
    return D3DResourceType_Invalid;
}

void D3D11PipelineLayout::BuildDynamicResourceBindings(const std::vector<BindingDescriptor>& bindingDescs)
{
    bindings_.reserve(bindingDescs.size());
    for (const BindingDescriptor& desc : bindingDescs)
        bindings_.push_back(D3D11PipelineResourceBinding{ ToD3DResourceType(desc), desc.slot.index, desc.stageFlags });
}

static ComPtr<ID3D11SamplerState> DXCreateSamplerState(ID3D11Device* device, const D3D11_SAMPLER_DESC& desc)
{
    ComPtr<ID3D11SamplerState> nativeSamplerState;
    HRESULT hr = device->CreateSamplerState(&desc, nativeSamplerState.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11SamplerState");
    return nativeSamplerState;
}

void D3D11PipelineLayout::BuildStaticSamplers(ID3D11Device* device, const std::vector<StaticSamplerDescriptor>& staticSamplerDescs)
{
    D3D11_SAMPLER_DESC nativeDesc;
    staticSamplers_.reserve(staticSamplerDescs.size());
    for (const StaticSamplerDescriptor& desc : staticSamplerDescs)
    {
        D3D11Sampler::ConvertDesc(nativeDesc, desc.sampler);
        staticSamplers_.push_back(D3D11StaticSampler{ desc.slot.index, desc.stageFlags, DXCreateSamplerState(device, nativeDesc) });
    }
}


} // /namespace LLGL



// ================================================================================
