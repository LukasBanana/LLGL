/*
 * D3D12Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
    Create(device);
}

D3D12Fence::~D3D12Fence()
{
    CloseHandle(event_);
}

void D3D12Fence::SetName(const char* name)
{
    D3D12SetObjectName(fence_.Get(), name);
}

void D3D12Fence::Create(ID3D12Device* device)
{
    /* Create D3D12 fence */
    auto hr = device->CreateFence(value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12Fence");

    /* Create Win32 event handle */
    //event_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event_)
        DXThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "failed to create Win32 event object for D3D12 fence");
}

// Converts the specified amount of nanoseconds into milliseconds (rounded up)
static DWORD NanosecsToMillisecs(UINT64 t)
{
    if (t == ~0ull)
        return INFINITE;
    else
        return static_cast<DWORD>((t + 999999) / 1000000);
}

bool D3D12Fence::Wait(UINT64 timeoutNanosecs)
{
    return WaitForValue(value_, NanosecsToMillisecs(timeoutNanosecs));
}

bool D3D12Fence::WaitForValue(UINT64 value, DWORD timeoutMillisecs)
{
    /* Wait until the fence has been crossed */
    UINT64 signaledValue = fence_->GetCompletedValue();
    if (/*fence_->GetCompletedValue()*/signaledValue < value)
    {
        auto hr = fence_->SetEventOnCompletion(value, event_);
        DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
        return (WaitForSingleObjectEx(event_, timeoutMillisecs, FALSE) == WAIT_OBJECT_0);
    }
    return true;
}

UINT64 D3D12Fence::NextValue()
{
    return ++value_;
}

UINT64 D3D12Fence::NextValue(UINT64 value)
{
    value_ = value;
    return NextValue();
}


} // /namespace LLGL



// ================================================================================
