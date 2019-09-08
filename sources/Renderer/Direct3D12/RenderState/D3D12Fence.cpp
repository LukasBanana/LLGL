/*
 * D3D12Fence.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Fence.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include <algorithm>


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
    D3D12SetObjectName(native_.Get(), name);
}

void D3D12Fence::Create(ID3D12Device* device)
{
    /* Create D3D12 fence */
    auto hr = device->CreateFence(value_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(native_.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12Fence");

    /* Create Win32 event handle */
    event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event_)
        DXThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "failed to create Win32 event object for D3D12 fence");

    /* Initialize first next value */
    ++value_;
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
    return WaitForValue(GetNextValue(), NanosecsToMillisecs(timeoutNanosecs));
}

bool D3D12Fence::WaitForValue(UINT64 value, DWORD timeoutMillisecs)
{
    /* Store next value */
    value_ = std::max(value_, value) + 1;

    /* Wait until the fence has been crossed */
    const UINT64 signaledValue = native_->GetCompletedValue();

    if (signaledValue < value)
    {
        auto hr = native_->SetEventOnCompletion(value, event_);
        DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
        return (WaitForSingleObjectEx(event_, timeoutMillisecs, FALSE) == WAIT_OBJECT_0);
    }

    return true;
}

bool D3D12Fence::WaitForValueAndUpdate(UINT64& value, DWORD timeoutMillisecs)
{
    if (WaitForValue(value, timeoutMillisecs))
    {
        value = GetNextValue();
        return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
