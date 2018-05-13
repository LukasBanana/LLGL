/*
 * D3D11Fence.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_FENCE_H
#define LLGL_D3D11_FENCE_H


#include <LLGL/Fence.h>
//#include <d3d11_4.h>


namespace LLGL
{


class D3D11Fence : public Fence
{

    public:

        #if 0
        D3D11Fence(ID3D11Device5* device, UINT64 initialValue);

        void Submit(ID3D11DeviceContext4* context);
        bool Wait(ID3D11DeviceContext4* context);
        #endif

    private:

        #if 0
        ComPtr<ID3D11Fence> fence_;
        UINT64              value_  = 0;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
