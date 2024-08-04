/*
 * D3D12RootSignature.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RootSignature.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>
#include <stdint.h>
#include <float.h>


namespace LLGL
{


void D3D12RootSignature::Clear()
{
    nativeRootParams_.clear();
    rootParams_.clear();
    staticSamplers_.clear();
}

void D3D12RootSignature::Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    nativeRootParams_.reserve(maxNumRootParamters);
    rootParams_.reserve(maxNumRootParamters);
    staticSamplers_.reserve(maxNumStaticSamplers);
}

void D3D12RootSignature::ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    Reset(maxNumRootParamters, maxNumStaticSamplers);
    while (maxNumRootParamters-- > 0)
        AppendRootParameter();
}

D3D12RootParameter* D3D12RootSignature::AppendRootParameter(UINT* outRootParameterIndex)
{
    /* Return optional root parameter index */
    if (outRootParameterIndex != nullptr)
        *outRootParameterIndex = static_cast<UINT>(rootParams_.size());

    /* Create new root paramter */
    nativeRootParams_.push_back({});
    rootParams_.push_back(&(nativeRootParams_.back()));

    /* Return root parameter */
    return &(rootParams_.back());
}

D3D12RootParameter* D3D12RootSignature::FindCompatibleRootParameter(
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    std::size_t                 first,
    UINT*                       outRootParameterIndex)
{
    /* Find compatible root parameter (search from back to front) */
    for_subrange_reverse(i, first, rootParams_.size())
    {
        if (rootParams_[i].IsCompatible(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, rangeType))
        {
            if (outRootParameterIndex != nullptr)
                *outRootParameterIndex = static_cast<UINT>(i);
            return &(rootParams_[i]);
        }
    }
    return nullptr;
}

D3D12RootParameter* D3D12RootSignature::FindCompatibleRootParameter(
    const D3D12_ROOT_CONSTANTS& rootConstants,
    D3D12_SHADER_VISIBILITY     visibility,
    std::size_t                 first,
    UINT*                       outRootParameterIndex)
{
    /* Find compatible root parameter (search from back to front) */
    for_subrange_reverse(i, first, rootParams_.size())
    {
        if (rootParams_[i].IsCompatible(rootConstants, visibility))
        {
            if (outRootParameterIndex != nullptr)
                *outRootParameterIndex = static_cast<UINT>(i);
            return &(rootParams_[i]);
        }
    }
    return nullptr;
}

D3D12_STATIC_SAMPLER_DESC* D3D12RootSignature::AppendStaticSampler()
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

    HRESULT hr = D3D12SerializeRootSignature(
        &signatureDesc,
        signatureversion,
        signature.ReleaseAndGetAddressOf(),
        error.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        if (error)
        {
            std::string errorStr = DXGetBlobString(error.Get());
            LLGL_TRAP("failed to serialize D3D12 root signature: %s", errorStr.c_str());
        }
        else
            DXThrowIfFailed(hr, "failed to serialize D3D12 root signature");
    }

    return signature;
}

static ComPtr<ID3D12RootSignature> DXCreateRootSignature(
    ID3D12Device*                               device,
    const ArrayView<D3D12_ROOT_PARAMETER>&      rootParameters,
    const ArrayView<D3D12_STATIC_SAMPLER_DESC>& staticSamplers,
    D3D12_ROOT_SIGNATURE_FLAGS                  flags,
    ComPtr<ID3DBlob>*                           serializedBlob)
{
    /* Create serialized root signature with the specified root parameters and static samplers */
    D3D12_ROOT_SIGNATURE_DESC signatureDesc;
    {
        if (rootParameters.empty())
        {
            signatureDesc.NumParameters = 0;
            signatureDesc.pParameters   = nullptr;
        }
        else
        {
            signatureDesc.NumParameters = static_cast<UINT>(rootParameters.size());
            signatureDesc.pParameters   = rootParameters.data();
        }

        if (staticSamplers.empty())
        {
            signatureDesc.NumStaticSamplers = 0;
            signatureDesc.pStaticSamplers   = nullptr;
        }
        else
        {
            signatureDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());
            signatureDesc.pStaticSamplers   = staticSamplers.data();
        }

        signatureDesc.Flags                 = flags;
    }
    ComPtr<ID3DBlob> signature = DXSerializeRootSignature(signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1);

    /* Create actual root signature */
    ComPtr<ID3D12RootSignature> rootSignature;
    HRESULT hr = device->CreateRootSignature(
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

ComPtr<ID3D12RootSignature> D3D12RootSignature::Finalize(
    ID3D12Device*               device,
    D3D12_ROOT_SIGNATURE_FLAGS  flags,
    ComPtr<ID3DBlob>*           serializedBlob)
{
    return DXCreateRootSignature(device, nativeRootParams_, staticSamplers_, flags, serializedBlob);
}


} // /namespace LLGL



// ================================================================================
