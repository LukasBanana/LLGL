/*
 * D3D12CommandContext.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_CONTEXT_H
#define LLGL_D3D12_COMMAND_CONTEXT_H


#include "../../DXCommon/ComPtr.h"
#include "../RenderState/D3D12Fence.h"
#include "../RenderState/D3D12PipelineLayout.h"
#include "../RenderState/D3D12StagingDescriptorHeapPool.h"
#include <d3d12.h>
#include <cstddef>
#include <cstdint>


namespace LLGL
{


struct D3D12Resource;
class D3D12Device;
class D3D12CommandQueue;

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

        D3D12CommandContext(
            D3D12Device&        device,
            D3D12CommandQueue&  commandQueue
        );

        // Creats the command list and internal command allocators.
        void Create(
            D3D12Device&            device,
            D3D12CommandQueue&      commandQueue,
            D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT,
            UINT                    numAllocators   = ~0u,
            bool                    initialClose    = false
        );

        void Close();
        void Execute();
        void Reset();

        // Calls Close, Execute, and Reset with the internal command queue and allocator.
        void Finish(bool waitIdle = false);

        // Returns the command list of this context.
        inline ID3D12GraphicsCommandList* GetCommandList() const
        {
            return commandList_.Get();
        }

        // Transition all subresources to the specified new state.
        void TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);

        #if 0 //TODO: not used yet
        // Transition the specified subresource to the specified new state.
        void TransitionSubresource(
            D3D12Resource&          resource,
            UINT                    subresource,
            D3D12_RESOURCE_STATES   oldState,
            D3D12_RESOURCE_STATES   newState,
            bool                    flushImmediate = false
        );
        #endif

        // Insert a resource barrier for an unordered access view (UAV).
        void InsertUAVBarrier(D3D12Resource& resource, bool flushImmediate = false);

        // Flush all accumulated resource barriers.
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
        void SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* const* descriptorHeaps);

        void PrepareStagingDescriptorHeaps(
            const D3D12DescriptorHeapSetLayout& layout,
            const D3D12RootParameterIndices&    indices
        );

        void SetGraphicsConstant(UINT parameterIndex, D3D12Constant value, UINT offset);
        void SetComputeConstant(UINT parameterIndex, D3D12Constant value, UINT offset);

        void SetGraphicsRootParameter(UINT parameterIndex, D3D12_ROOT_PARAMETER_TYPE parameterType, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddr);
        void SetComputeRootParameter(UINT parameterIndex, D3D12_ROOT_PARAMETER_TYPE parameterType, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddr);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptor) const;

        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptors(
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        void NextDescriptorHeap();
        void NextDescriptorSet();

        void DrawInstanced(
            UINT vertexCountPerInstance,
            UINT instanceCount,
            UINT startVertexLocation,
            UINT startInstanceLocation
        );

        void DrawIndexedInstanced(
            UINT    indexCountPerInstance,
            UINT    instanceCount,
            UINT    startIndexLocation,
            INT     baseVertexLocation,
            UINT    startInstanceLocation
        );

        void DrawIndirect(
            ID3D12CommandSignature* commandSignature,
            UINT                    maxCommandCount,
            ID3D12Resource*         argumentBuffer,
            UINT64                  argumentBufferOffset,
            ID3D12Resource*         countBuffer,
            UINT64                  countBufferOffset
        );

        void Dispatch(
            UINT threadGroupCountX,
            UINT threadGroupCountY,
            UINT threadGroupCountZ
        );

        void DispatchIndirect(
            ID3D12CommandSignature* commandSignature,
            UINT                    maxCommandCount,
            ID3D12Resource*         argumentBuffer,
            UINT64                  argumentBufferOffset,
            ID3D12Resource*         countBuffer,
            UINT64                  countBufferOffset
        );

    private:

        static const UINT g_maxNumAllocators        = 3;
        static const UINT g_maxNumResourceBarrieres = 16;
        static const UINT g_maxNumDescriptorHeaps   = 2;

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
                    std::uint32_t   descriptorHeaps         : 1;
                };
                std::uint32_t       value;
            }
            dirtyBits;

            ID3D12RootSignature*    graphicsRootSignature                       = nullptr;
            ID3D12RootSignature*    computeRootSignature                        = nullptr;
            ID3D12PipelineState*    pipelineState                               = nullptr;
            UINT                    numDescriptorHeaps                          = 0;
            ID3D12DescriptorHeap*   descriptorHeaps[g_maxNumDescriptorHeaps]    = {};
        };

    private:

        // Clears the internal cached states.
        void ClearCache();

        // Returns the next resource barrier and flushes previous barriers if the cache is full.
        D3D12_RESOURCE_BARRIER& NextResourceBarrier();

        // Switches to the next command allocator and resets it.
        void NextCommandAllocator();

        // Increments the write offsets for the staging descriptor pools.
        void NextStagingDescriptors(UINT numResourceViews, UINT numSamplers);

        void FlushGraphicsStagingDescriptorTables();
        void FlushComputeStagingDescriptorTables();

        // Returns the current command allocator.
        inline ID3D12CommandAllocator* GetCommandAllocator() const
        {
            return commandAllocators_[currentAllocatorIndex_].Get();
        }

    private:

        D3D12CommandQueue*                  commandQueue_                                   = nullptr;

        ComPtr<ID3D12CommandAllocator>      commandAllocators_[g_maxNumAllocators];
        UINT                                currentAllocatorIndex_                          = 0;
        UINT                                numAllocators_                                  = g_maxNumAllocators;

        UINT64                              allocatorFenceValues_[g_maxNumAllocators]       = {};
        D3D12Fence                          allocatorFence_;

        ComPtr<ID3D12GraphicsCommandList>   commandList_;

        D3D12_RESOURCE_BARRIER              resourceBarriers_[g_maxNumResourceBarrieres];
        UINT                                numResourceBarriers_                            = 0;

        D3D12StagingDescriptorHeapPool      stagingDescriptorPools_[g_maxNumAllocators][g_maxNumDescriptorHeaps];
        D3D12DescriptorHeapSetLayout        stagingDescriptorSetLayout_;
        D3D12RootParameterIndices           stagingDescriptorIndices_;

        StateCache                          stateCache_;

};


} // /namespace LLGL


#endif



// ================================================================================
