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
#include <cstdint>


namespace LLGL
{


struct D3D12Resource;
class D3D12Fence;

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

        D3D12CommandContext();

        void SetCommandList(ID3D12GraphicsCommandList* commandList);
        void SetCommandQueueAndAllocator(ID3D12CommandQueue* commandQueue, ID3D12CommandAllocator* commandAllocator);

        void Close();
        void Execute(ID3D12CommandQueue* commandQueue);
        void Reset(ID3D12CommandAllocator* commandAllocator);

        // Calls Close, Execute, and Reset with the internal command queue and allocator.
        void Finish(D3D12Fence* fence = nullptr);

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

        void SetGraphicsRootSignature(ID3D12RootSignature* rootSignature);
        void SetComputeRootSignature(ID3D12RootSignature* rootSignature);
        void SetPipelineState(ID3D12PipelineState* pipelineState);

        void SetGraphicsConstant(UINT parameterIndex, D3D12Constant value, UINT offset);
        void SetComputeConstant(UINT parameterIndex, D3D12Constant value, UINT offset);

    private:

        struct StateCache
        {
            union
            {
                struct
                {
                    std::uint32_t   pipelineState           : 1;
                    std::uint32_t   graphicsRootSignature   : 1;
                    std::uint32_t   computeRootSignature    : 1;
                };
                std::uint32_t       value;
            }
            dirtyBits;

            ID3D12RootSignature*    graphicsRootSignature   = nullptr;
            ID3D12RootSignature*    computeRootSignature    = nullptr;
            ID3D12PipelineState*    pipelineState           = nullptr;
        };

    private:

        D3D12_RESOURCE_BARRIER& NextResourceBarrier();

        void ClearCache();

    private:

        static const UINT g_maxNumResourceBarrieres = 16;

        ID3D12GraphicsCommandList*  commandList_                                    = nullptr;
        ID3D12CommandQueue*         commandQueue_                                   = nullptr;
        ID3D12CommandAllocator*     commandAllocator_                               = nullptr;

        D3D12_RESOURCE_BARRIER      resourceBarriers_[g_maxNumResourceBarrieres];
        UINT                        numResourceBarriers_                            = 0;

        StateCache                  stateCache_;

};


} // /namespace LLGL


#endif



// ================================================================================
