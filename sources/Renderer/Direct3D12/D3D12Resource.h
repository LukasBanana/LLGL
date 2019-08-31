/*
 * D3D12Resource.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_H
#define LLGL_D3D12_RESOURCE_H


#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <utility>


namespace LLGL
{


//TODO: Rename to "D3D12GpuResource" to avoid overlapping terminology with <Resource> interface.
// Helper struct to store a D3D12 resource with its usage state and transition state.
struct D3D12Resource
{
    D3D12Resource() = default;

    inline D3D12Resource(ComPtr<ID3D12Resource>&& native, D3D12_RESOURCE_STATES initialState) :
        native          { std::move(native) },
        usageState      { initialState      },
        transitionState { initialState      }
    {
    }

    // Sets both the current usage and the transition states to the specified initial state.
    inline void SetInitialState(D3D12_RESOURCE_STATES initialState)
    {
        usageState      = initialState;
        transitionState = initialState;
    }

    // Returns the natvie resource object.
    inline ID3D12Resource* Get() const
    {
        return native.Get();
    }

    ComPtr<ID3D12Resource>  native;
    D3D12_RESOURCE_STATES   usageState      = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES   transitionState = D3D12_RESOURCE_STATE_COMMON;
};


} // /namespace LLGL


#endif



// ================================================================================
