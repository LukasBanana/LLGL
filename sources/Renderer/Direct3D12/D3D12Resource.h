/*
 * D3D12Resource.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_H
#define LLGL_D3D12_RESOURCE_H


#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <utility>


namespace LLGL
{


struct D3D12Resource
{
    D3D12Resource() = default;

    inline D3D12Resource(ComPtr<ID3D12Resource>&& native, D3D12_RESOURCE_STATES initialState) :
        native          { std::move(native) },
        usageState      { initialState      },
        transitionState { initialState      }
    {
    }

    inline void SetInitialState(D3D12_RESOURCE_STATES initialState)
    {
        usageState      = initialState;
        transitionState = initialState;
    }

    ComPtr<ID3D12Resource>  native;
    D3D12_RESOURCE_STATES   usageState      = D3D12_RESOURCE_STATE_COMMON;
    D3D12_RESOURCE_STATES   transitionState = D3D12_RESOURCE_STATE_COMMON;
};


} // /namespace LLGL


#endif



// ================================================================================
