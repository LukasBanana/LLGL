/*
 * D3D12Fence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_FENCE_H
#define LLGL_D3D12_FENCE_H


#include <LLGL/Fence.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


// Wrapper class for a native D3D12 fence object and its associated event handle.
class D3D12NativeFence
{

    public:

        D3D12NativeFence() = default;

        D3D12NativeFence(const D3D12NativeFence&) = delete;
        D3D12NativeFence& operator = (const D3D12NativeFence&) = delete;

        // Constructs the native D3D12 fence and event handle. Also initializes it with an optional value.
        D3D12NativeFence(ID3D12Device* device, UINT64 initialValue = 0);

        // Destroys the native D3D12 fence and event handle.
        ~D3D12NativeFence();

        // Creates the native D3D12 fence and event handle.
        void Create(ID3D12Device* device, UINT64 initialValue = 0);

        // Waits until this fence has been signaled with the specified value.
        bool WaitForSignal(UINT64 signal, DWORD timeoutMillisecs = INFINITE);

        // Waits until this fense has been signaled with the specified value or a higher value.
        bool WaitForHigherSignal(UINT64 signal, DWORD timeoutMillisecs = INFINITE);

        // Returns the native ID3D12Fence object.
        inline ID3D12Fence* Get() const
        {
            return native_.Get();
        }

        // Returns the completed value. Once the signal has completed, this value will be the same as the signaled value; See GetSignaledValue().
        inline UINT64 GetCompletedValue() const
        {
            return native_->GetCompletedValue();
        }

    private:

        ComPtr<ID3D12Fence> native_;
        HANDLE              event_  = 0;

};

// D3D12 implementation of the <Fence> interface.
class D3D12Fence final : public Fence
{

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D12Fence(ID3D12Device* device, UINT64 initialValue = 0);

        // Sets the next signal value.
        UINT64 Signal();

        // Waits until the current signaled value is completed.
        bool Wait(UINT64 timeout);

        // Returns the native ID3D12Fence object.
        inline ID3D12Fence* GetNative() const
        {
            return native_.Get();
        }

        // Returns the current value this fence has been signaled with. This value will eventually become the completed value.
        inline UINT64 GetSignaledValue() const
        {
            return value_;
        }

        // Returns the completed value. Once the signal has completed, this value will be the same as the signaled value; See GetSignaledValue().
        inline UINT64 GetCompletedValue() const
        {
            return native_.GetCompletedValue();
        }

    private:

        D3D12NativeFence    native_;
        UINT64              value_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
