/*
 * D3D11StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_STATE_MANAGER_H__
#define __LLGL_D3D11_STATE_MANAGER_H__


#include "../../ComPtr.h"
#include <LLGL/RenderContextFlags.h>
#include <vector>
#include <d3d11.h>


namespace LLGL
{


class D3D11StateManager
{

    public:

        D3D11StateManager(ComPtr<ID3D11DeviceContext>& context);

        void SetViewports(std::size_t numViewports, const Viewport* viewports);
        void SetScissors(std::size_t numScissors, const Scissor* scissors);

    private:

        ComPtr<ID3D11DeviceContext> context_;

};


} // /namespace LLGL


#endif



// ================================================================================
