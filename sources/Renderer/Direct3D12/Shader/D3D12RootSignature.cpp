/*
 * D3D12RootSignatureBuilder.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RootSignature.h"
#include "../../DXCommon/DXCore.h"
#include <stdint.h>


namespace LLGL
{


void D3D12RootSignatureBuilder::Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    nativeRootParams_.reserve(maxNumRootParamters);
    rootParams_.reserve(maxNumRootParamters);
    staticSamplers_.reserve(maxNumStaticSamplers);
}

void D3D12RootSignatureBuilder::ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    Reset(maxNumRootParamters, maxNumStaticSamplers);
    while (maxNumRootParamters-- > 0)
        AppendRootParameter();
}

D3D12RootParameter* D3D12RootSignatureBuilder::AppendRootParameter()
{
    /* Create new root paramter */
    nativeRootParams_.push_back({});
    rootParams_.emplace_back(&(nativeRootParams_.back()));
    return &(rootParams_.back());
}

D3D12RootParameter* D3D12RootSignatureBuilder::FindCompatibleRootParameter(D3D12_DESCRIPTOR_RANGE_TYPE rangeType)
{
    /* Find compatible root parameter (search from back to front) */
    for (auto it = rootParams_.rbegin(); it != rootParams_.rend(); ++it)
    {
        if (it->IsCompatible(rangeType))
            return &(*it);
    }
    return nullptr;
}

D3D12_STATIC_SAMPLER_DESC* D3D12RootSignatureBuilder::AppendStaticSampler()
{
    D3D12_STATIC_SAMPLER_DESC samplerDesc;
    {
        samplerDesc.Filter              = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU            = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV            = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW            = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.MipLODBias          = 0.0f;
        samplerDesc.MaxAnisotropy       = 1;
        samplerDesc.ComparisonFunc      = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.BorderColor         = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        samplerDesc.MinLOD              = -FLT_MAX;
        samplerDesc.MaxLOD              = +FLT_MAX;
        samplerDesc.ShaderRegister      = 0;
        samplerDesc.RegisterSpace       = 0;
        samplerDesc.ShaderVisibility    = D3D12_SHADER_VISIBILITY_ALL;
    }
    staticSamplers_.push_back(samplerDesc);
    return &(staticSamplers_.back());
}

static ComPtr<ID3DBlob> DXSerializeRootSignature(
    const D3D12_ROOT_SIGNATURE_DESC& signatureDesc,
    const D3D_ROOT_SIGNATURE_VERSION signatureversion)
{
    ComPtr<ID3DBlob> signature, error;

    auto hr = D3D12SerializeRootSignature(
        &signatureDesc,
        signatureversion,
        signature.ReleaseAndGetAddressOf(),
        error.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        if (error)
        {
            auto errorStr = DXGetBlobString(error.Get());
            throw std::runtime_error("failed to serialize D3D12 root signature: " + errorStr);
        }
        else
            DXThrowIfFailed(hr, "failed to serialize D3D12 root signature");
    }

    return signature;
}

static ComPtr<ID3D12RootSignature> DXCreateRootSignature(
    ID3D12Device*                       device,
    const D3D12_ROOT_SIGNATURE_DESC&    signatureDesc,
    ComPtr<ID3DBlob>*                   serializedBlob)
{
    ComPtr<ID3D12RootSignature> rootSignature;

    /* Create serialized root signature */
    auto signature = DXSerializeRootSignature(signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1);

    /* Create actual root signature */
    auto hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf())
    );
    DXThrowIfFailed(hr, "failed to create D3D12 root signature");

    /* Return serialized signature blob */
    if (serializedBlob != nullptr)
        *serializedBlob = std::move(signature);

    return rootSignature;
}

ComPtr<ID3D12RootSignature> D3D12RootSignatureBuilder::Finalize(
    ID3D12Device*               device,
    D3D12_ROOT_SIGNATURE_FLAGS  flags,
    ComPtr<ID3DBlob>*           serializedBlob)
{
    D3D12_ROOT_SIGNATURE_DESC signatureDesc;
    {
        signatureDesc.NumParameters         = static_cast<UINT>(nativeRootParams_.size());
        signatureDesc.pParameters           = nativeRootParams_.data();

        if (staticSamplers_.empty())
        {
            signatureDesc.NumStaticSamplers = 0;
            signatureDesc.pStaticSamplers   = nullptr;
        }
        else
        {
            signatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());
            signatureDesc.pStaticSamplers   = staticSamplers_.data();
        }

        signatureDesc.Flags                 = flags;
    }
    return DXCreateRootSignature(device, signatureDesc, serializedBlob);
}


} // /namespace LLGL



// ================================================================================
