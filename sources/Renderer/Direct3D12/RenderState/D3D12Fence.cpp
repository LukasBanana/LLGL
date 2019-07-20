/*
 * D3D12Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Fence.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12Fence::D3D12Fence(ID3D12Device* device, UINT64 initialValue) :
    value_ { initialValue }
{
    /* Create D3D12 fence */
    auto hr = device->CreateFence(value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 fence");

    /* Create Win32 event handle */
    event_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
}

D3D12Fence::~D3D12Fence()
{
    CloseHandle(event_);
}

void D3D12Fence::SetName(const char* name)
{
    D3D12SetObjectName(fence_.Get(), name);
}

UINT64 D3D12Fence::NextValue()
{
    return ++value_;
}

// Converts the specified amount of nanoseconds into milliseconds (rounded up)
static DWORD NanosecsToMillisecs(UINT64 t)
{
    if (t == ~0)
        return INFINITE;
    else
        return static_cast<DWORD>((t + 999999) / 1000000);
}

bool D3D12Fence::Wait(UINT64 timeout)
{
    /* Wait until the fence has been crossed */
    if (fence_->GetCompletedValue() < value_)
    {
        auto hr = fence_->SetEventOnCompletion(value_, event_);
        DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
        return (WaitForSingleObject(event_, NanosecsToMillisecs(timeout)) == WAIT_OBJECT_0);
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
