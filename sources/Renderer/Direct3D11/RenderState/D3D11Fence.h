/*
 * D3D11Fence.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_FENCE_H
#define LLGL_D3D11_FENCE_H


#include <LLGL/Fence.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d11.h>
#include <cstdint>


namespace LLGL
{


class D3D11Fence final : public Fence
{

    public:

        D3D11Fence(ID3D11Device* device);

        void Submit(ID3D11DeviceContext* context);
        void Wait(ID3D11DeviceContext* context);

    private:

        ComPtr<ID3D11Query> query_;

};


} // /namespace LLGL


#endif



// ================================================================================
