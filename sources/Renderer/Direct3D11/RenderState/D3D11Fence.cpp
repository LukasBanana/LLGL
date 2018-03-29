/*
 * D3D11Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Fence.h"


namespace LLGL
{


#if 0

D3D11Fence::D3D11Fence(ID3D11Device5* device, UINT64 initialValue) :
    value_ { initialValue }
{
    device->CreateFence(value_, D3D11_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
}

void D3D11Fence::Submit(ID3D11DeviceContext4* context)
{
    ++value_;
    context->Signal(fence_.Get(), value_);
}

bool D3D11Fence::Wait(ID3D11DeviceContext4* context)
{
    return context->Wait(fence_.Get(), value_);
}

#endif


} // /namespace LLGL



// ================================================================================
