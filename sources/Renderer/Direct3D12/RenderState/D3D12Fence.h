/*
 * D3D12Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_FENCE_H
#define LLGL_D3D12_FENCE_H


#include <LLGL/Fence.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Fence : public Fence
{

    public:

        D3D12Fence(ID3D12Device* device, UINT64 initialValue);
        ~D3D12Fence();

        void Submit(ID3D12CommandQueue* commandQueue);
        bool Wait(UINT64 timeout);

    private:

        ComPtr<ID3D12Fence> fence_;
        HANDLE              event_  = 0;
        UINT64              value_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
