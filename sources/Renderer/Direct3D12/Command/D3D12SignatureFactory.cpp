/*
 * D3D12SignatureFactory.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12SignatureFactory.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


static void DXCreateCommandSignature(ID3D12Device* device, ComPtr<ID3D12CommandSignature>& signature, D3D12_INDIRECT_ARGUMENT_TYPE argumentType, UINT stride)
{
    /* Initialize indirect argument descriptor */
    D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
    argumentDesc.Type = argumentType;

    /* Create command signature for indirect argument */
    D3D12_COMMAND_SIGNATURE_DESC signatureDesc;
    {
        signatureDesc.ByteStride        = stride;
        signatureDesc.NumArgumentDescs  = 1;
        signatureDesc.pArgumentDescs    = &argumentDesc;
        signatureDesc.NodeMask          = 0;
    }
    auto hr = device->CreateCommandSignature(&signatureDesc, nullptr, IID_PPV_ARGS(signature.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12CommandSignature");
}

void D3D12SignatureFactory::CreateDefaultSignatures(ID3D12Device* device)
{
    DXCreateCommandSignature(device, signatureDrawIndirect_,        D3D12_INDIRECT_ARGUMENT_TYPE_DRAW,         sizeof(D3D12_DRAW_ARGUMENTS        ));
    DXCreateCommandSignature(device, signatureDrawIndexedIndirect_, D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS));
    DXCreateCommandSignature(device, signatureDispatchIndirect_,    D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,     sizeof(D3D12_DISPATCH_ARGUMENTS    ));
}


} // /namespace LLGL



// ================================================================================
