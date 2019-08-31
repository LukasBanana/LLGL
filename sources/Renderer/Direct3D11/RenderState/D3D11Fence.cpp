/*
 * D3D11Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Fence.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D11Fence::D3D11Fence(ID3D11Device* device)
{
    /* Create event query */
    D3D11_QUERY_DESC queryDesc;
    {
        queryDesc.Query     = D3D11_QUERY_EVENT;
        queryDesc.MiscFlags = 0;
    }
    auto hr = device->CreateQuery(&queryDesc, query_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 query");
}

void D3D11Fence::Submit(ID3D11DeviceContext* context)
{
    context->End(query_.Get());
}

void D3D11Fence::Wait(ID3D11DeviceContext* context)
{
    while (context->GetData(query_.Get(), nullptr, 0, 0) == S_FALSE) { /* dummy */ }
}


} // /namespace LLGL



// ================================================================================
