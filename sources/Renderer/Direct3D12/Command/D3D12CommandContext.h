/*
 * D3D12CommandContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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

union D3D12Constant
{
    inline D3D12Constant(UINT value) :
        u32 { value }
    {
    }
    inline D3D12Constant(FLOAT value) :
        f32 { value }
    {
    }

    UINT    u32;
    FLOAT   f32;
};

class D3D12CommandContext
{

    public:

        void SetCommandList(ID3D12GraphicsCommandList* commandList);

        inline ID3D12GraphicsCommandList* GetCommandList() const
        {
            return commandList_;
        }

        void TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
        void InsertUAVBarrier(D3D12Resource& resource, bool flushImmediate = false);
        void FlushResourceBarrieres();

        void ResolveRenderTarget(
            D3D12Resource&  dstResource,
            UINT            dstSubresource,
            D3D12Resource&  srcResource,
            UINT            srcSubresource,
            DXGI_FORMAT     format
        );

        void SetGraphicsConstant(UINT parameterIndex, D3D12Constant value, UINT offset);
        void SetComputeConstant(UINT parameterIndex, D3D12Constant value, UINT offset);

    private:

        D3D12_RESOURCE_BARRIER& NextResourceBarrier();

    private:

        static const UINT g_maxNumResourceBarrieres = 16;

        ID3D12GraphicsCommandList*  commandList_                                    = nullptr;

        D3D12_RESOURCE_BARRIER      resourceBarriers_[g_maxNumResourceBarrieres];
        UINT                        numResourceBarriers_                            = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
