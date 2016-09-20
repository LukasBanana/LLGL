/*
 * D3D12StateManager.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_STATE_MANAGER_H__
#define __LLGL_D3D12_STATE_MANAGER_H__


#include "../../ComPtr.h"
#include <LLGL/RenderContextFlags.h>
#include <vector>
#include <d3d12.h>


namespace LLGL
{


class D3D12StateManager
{

    public:

        D3D12StateManager(ComPtr<ID3D12GraphicsCommandList>& commandList);

        void SetViewports(const std::vector<Viewport>& viewports);
        void SubmitViewports();

        void SetScissors(const std::vector<Scissor>& scissors);
        void SubmitScissors();

    private:

        ComPtr<ID3D12GraphicsCommandList>   commandList_;

        std::vector<D3D12_VIEWPORT>         viewports_;
        std::vector<D3D12_RECT>             scissors_;

};


} // /namespace LLGL


#endif



// ================================================================================
