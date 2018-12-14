/*
 * D3D12CommandContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_CONTEXT_H
#define LLGL_D3D12_COMMAND_CONTEXT_H


#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <cstddef>


namespace LLGL
{


struct D3D12Resource;

class D3D12CommandContext
{

    public:

        void SetCommandList(ID3D12GraphicsCommandList* commandList);

        inline ID3D12GraphicsCommandList* GetCommandList() const
        {
            return commandList_;
        }

        void TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushBarrieres = true);
        void FlushResourceBarrieres();

        void ResolveRenderTarget(
            D3D12Resource&  dstResource,
            UINT            dstSubresource,
            D3D12Resource&  srcResource,
            UINT            srcSubresource,
            DXGI_FORMAT     format
        );

    private:

        D3D12_RESOURCE_BARRIER& NextResourceBarrier();

        static const UINT g_maxNumResourceBarrieres = 16;

        ID3D12GraphicsCommandList*  commandList_                                    = nullptr;

        D3D12_RESOURCE_BARRIER      resourceBarriers_[g_maxNumResourceBarrieres];
        UINT                        numResourceBarriers_                            = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
