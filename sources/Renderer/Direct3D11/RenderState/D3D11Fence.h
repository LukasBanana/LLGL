/*
 * D3D11Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
