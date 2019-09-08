/*
 * D3D12Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_FENCE_H
#define LLGL_D3D12_FENCE_H


#include <LLGL/Fence.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Fence final : public Fence
{

    public:

        void SetName(const char* name) override;

    public:

        D3D12Fence() = default;
        D3D12Fence(ID3D12Device* device, UINT64 initialValue = 0);
        ~D3D12Fence();

        // Re-creates this fence object.
        void Create(ID3D12Device* device);

        // Waits until the current fence value gets signaled.
        bool Wait(UINT64 timeoutNanosecs);

        // Waits until the specified value gets signaled.
        bool WaitForValue(UINT64 value, DWORD timeoutMillisecs = INFINITE);

        // Waits until the specified value gets signaled and stores the next value.
        bool WaitForValueAndUpdate(UINT64& value, DWORD timeoutMillisecs = INFINITE);

        // Returns the native ID3D12Fence object.
        inline ID3D12Fence* GetNative() const
        {
            return native_.Get();
        }

        // Returns the next value that will eventually be signaled.
        inline UINT64 GetNextValue() const
        {
            return value_;
        }

    private:

        ComPtr<ID3D12Fence> native_;
        HANDLE              event_  = 0;
        UINT64              value_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
