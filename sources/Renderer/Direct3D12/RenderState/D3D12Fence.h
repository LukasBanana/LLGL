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
        D3D12Fence(ID3D12Device* device, UINT64 initialValue);
        ~D3D12Fence();

        // Re-creates this fence object.
        void Create(ID3D12Device* device);

        // Waits until the current fence value gets signaled.
        bool Wait(UINT64 timeoutNanosecs);

        // Waits until the specified value gets signaled.
        bool WaitForValue(UINT64 value, DWORD timeoutMillisecs);

        // Returns the next value to be used as fence signal.
        UINT64 NextValue();

        // Returns the next value to be used as fence signal after the specified value.
        UINT64 NextValue(UINT64 value);

        // Returns the native ID3D12Fence object.
        inline ID3D12Fence* GetNative() const
        {
            return fence_.Get();
        }

    private:

        ComPtr<ID3D12Fence> fence_;
        HANDLE              event_  = 0;
        UINT64              value_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
