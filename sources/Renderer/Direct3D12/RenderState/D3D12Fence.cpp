/*
 * D3D12Fence.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Fence.h"
#include "../D3D12ObjectUtils.h"
#include "../../../Core/Assertion.h"
#include "../../DXCommon/DXCore.h"
#include <algorithm>


namespace LLGL
{


/*
 * D3D12NativeFence
 */

D3D12NativeFence::D3D12NativeFence(ID3D12Device* device, UINT64 initialValue)
{
    Create(device, initialValue);
}

D3D12NativeFence::~D3D12NativeFence()
{
    CloseHandle(event_);
}

void D3D12NativeFence::Create(ID3D12Device* device, UINT64 initialValue)
{
    LLGL_ASSERT(native_.Get() == nullptr);

    /* Create D3D12 fence */
    HRESULT hr = device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(native_.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, "ID3D12Fence");

    /* Create Win32 event handle */
    event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!event_)
        DXThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "failed to create Win32 event object for D3D12 fence");
}

bool D3D12NativeFence::WaitForSignal(UINT64 signal, DWORD timeoutMillisecs)
{
    /* Wait until the fence has been crossed */
    HRESULT hr = native_->SetEventOnCompletion(signal, event_);
    DXThrowIfFailed(hr, "failed to set 'on completion'-event for D3D12 fence");
    return (WaitForSingleObjectEx(event_, timeoutMillisecs, FALSE) == WAIT_OBJECT_0);
}

bool D3D12NativeFence::WaitForHigherSignal(UINT64 signal, DWORD timeoutMillisecs)
{
    if (GetCompletedValue() < signal)
        return WaitForSignal(signal, timeoutMillisecs);
    else
        return true;
}


/*
 * D3D12Fence
 */

D3D12Fence::D3D12Fence(ID3D12Device* device, UINT64 initialValue) :
    native_ { device, initialValue },
    value_  { initialValue + 1     }
{
}

void D3D12Fence::SetDebugName(const char* name)
{
    D3D12SetObjectName(native_.Get(), name);
}

// Converts the specified amount of nanoseconds into milliseconds (rounded up)
static DWORD NanosecsToMillisecs(UINT64 t)
{
    if (t == ~0ull)
        return INFINITE;
    else
        return static_cast<DWORD>((t + 999999) / 1000000);
}

UINT64 D3D12Fence::Signal()
{
    return ++value_;
}

bool D3D12Fence::Wait(UINT64 timeout)
{
    if (value_ > native_.GetCompletedValue())
    {
        native_.WaitForSignal(value_, NanosecsToMillisecs(timeout));
        value_ = native_.GetCompletedValue();
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
