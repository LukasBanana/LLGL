/*
 * D3D11StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_STATE_MANAGER_H
#define LLGL_D3D11_STATE_MANAGER_H


#include "../../DXCommon/ComPtr.h"
#include <LLGL/GraphicsPipelineFlags.h>
#include <vector>
#include <cstdint>
#include <d3d11.h>


namespace LLGL
{


//TODO: add function to bind ID3D11... objects to avoid unnecessary bindings (can be easily tracked by their pointers).
class D3D11StateManager
{

    public:

        D3D11StateManager(ComPtr<ID3D11DeviceContext>& context);

        void SetViewports(std::uint32_t numViewports, const Viewport* viewportArray);
        void SetScissors(std::uint32_t numScissors, const Scissor* scissorArray);

    private:

        ComPtr<ID3D11DeviceContext> context_;

};


} // /namespace LLGL


#endif



// ================================================================================
