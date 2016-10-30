/*
 * D3D12StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_STATE_MANAGER_H
#define LLGL_D3D12_STATE_MANAGER_H


#include "../../ComPtr.h"
#include <LLGL/RenderContextFlags.h>
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12StateManager
{

    public:

        void SetViewports(unsigned int numViewports, const Viewport* viewportArray);
        void SubmitViewports(ID3D12GraphicsCommandList* commandList);

        void SetScissors(unsigned int numScissors, const Scissor* scissorArray);
        void SubmitScissors(ID3D12GraphicsCommandList* commandList);

    private:

        std::vector<D3D12_VIEWPORT> viewports_;
        std::vector<D3D12_RECT>     scissors_;

};


} // /namespace LLGL


#endif



// ================================================================================
