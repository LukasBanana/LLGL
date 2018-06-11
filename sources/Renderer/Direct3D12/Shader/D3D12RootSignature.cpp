/*
 * D3D12RootSignature.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RootSignature.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


void D3D12RootSignature::Reset(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    nativeRootParams_.reserve(maxNumRootParamters);
    rootParams_.reserve(maxNumRootParamters);
}

void D3D12RootSignature::ResetAndAlloc(UINT maxNumRootParamters, UINT maxNumStaticSamplers)
{
    Reset(maxNumRootParamters, maxNumStaticSamplers);
    while (maxNumRootParamters-- > 0)
        AppendRootParameter();
}

D3D12RootParameter* D3D12RootSignature::AppendRootParameter()
{
    /* Create new root paramter */
    nativeRootParams_.push_back({});
    rootParams_.emplace_back(&(nativeRootParams_.back()));
    return &(rootParams_.back());
}

D3D12RootParameter* D3D12RootSignature::FindCompatibleRootParameter(D3D12_DESCRIPTOR_RANGE_TYPE rangeType)
{
    /* Find compatible root parameter (search from back to front) */
    for (auto it = rootParams_.rbegin(); it != rootParams_.rend(); ++it)
    {
        if (it->IsCompatible(rangeType))
            return &(*it);
    }
    return nullptr;
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

static ComPtr<ID3D12RootSignature> DXCreateRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& signatureDesc)
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

    return rootSignature;
}

ComPtr<ID3D12RootSignature> D3D12RootSignature::Finalize(ID3D12Device* device, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
    D3D12_ROOT_SIGNATURE_DESC signatureDesc;
    {
        signatureDesc.NumParameters     = static_cast<UINT>(nativeRootParams_.size());
        signatureDesc.pParameters       = nativeRootParams_.data();
        signatureDesc.NumStaticSamplers = 0;
        signatureDesc.pStaticSamplers   = nullptr;
        signatureDesc.Flags             = flags;
    }
    return DXCreateRootSignature(device, signatureDesc);
}


} // /namespace LLGL



// ================================================================================
