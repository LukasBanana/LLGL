/*
 * D3D12Resource.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_RESOURCE_H
#define LLGL_D3D12_RESOURCE_H


#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <utility>


namespace LLGL
{


// Helper struct to store a D3D12 resource with its usage state and transition state.
struct D3D12Resource
{
    D3D12Resource() = default;

    inline D3D12Resource(ComPtr<ID3D12Resource>&& native, D3D12_RESOURCE_STATES initialState) :
        native       { std::move(native) },
        usageState   { initialState      },
        currentState { initialState      }
    {
    }

    // Sets both the resource state for common usage and the initial states.
    inline void SetInitialState(D3D12_RESOURCE_STATES initialState)
    {
        usageState      = initialState;
        currentState    = initialState;
    }

    // Sets the resource state for common usage and the initial state individually and returns the initial state.
    inline D3D12_RESOURCE_STATES SetInitialAndUsageStates(D3D12_RESOURCE_STATES initial, D3D12_RESOURCE_STATES usage)
    {
        usageState    = usage;
        currentState  = initial;
        return initial;
    }

    // Returns the native resource object.
    inline ID3D12Resource* Get() const
    {
        return native.Get();
    }

    ComPtr<ID3D12Resource>  native;
    D3D12_RESOURCE_STATES   usageState      = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES   currentState    = D3D12_RESOURCE_STATE_COMMON;
};


} // /namespace LLGL


#endif



// ================================================================================
