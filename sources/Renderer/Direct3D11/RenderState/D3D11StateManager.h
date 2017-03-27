/*
 * D3D11StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STATE_MANAGER_H
#define LLGL_D3D11_STATE_MANAGER_H


#include "../../DXCommon/ComPtr.h"
#include <LLGL/RenderContextFlags.h>
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class D3D11StateManager
{

    public:

        D3D11StateManager(ComPtr<ID3D11DeviceContext>& context);

        void SetViewports(unsigned int numViewports, const Viewport* viewportArray);
        void SetScissors(unsigned int numScissors, const Scissor* scissorArray);

    private:

        ComPtr<ID3D11DeviceContext> context_;

};


} // /namespace LLGL


#endif



// ================================================================================
