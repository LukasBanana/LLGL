/*
 * D3D12Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Fence.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12Fence::D3D12Fence(ID3D12Device* device, UINT64 initialValue) :
    value_ { initialValue }
{
    device->CreateFence(value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    event_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
}

D3D12Fence::~D3D12Fence()
{
    CloseHandle(event_);
}

void D3D12Fence::Submit(ID3D12CommandQueue* commandQueue)
{
    /* Schedule signal command into the qeue */
    ++value_;
    auto hr = commandQueue->Signal(fence_.Get(), value_);
    DXThrowIfFailed(hr, "failed to signal D3D12 fence with command queue");
}

bool D3D12Fence::Wait(UINT64 timeout)
{
    /* Wait until the fence has been crossed */
    if (fence_->GetCompletedValue() < value_)
    {
        DWORD timeoutMs = INFINITE;

        if (timeout != ~0)
        {
            if (timeout < 1000000)
                timeoutMs = 1;
            else
                timeoutMs = static_cast<DWORD>(timeout / 1000000);
        }

        auto hr = fence_->SetEventOnCompletion(value_, event_);
        DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");

        return (WaitForSingleObject(event_, timeoutMs) == WAIT_OBJECT_0);
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
