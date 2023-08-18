/*
 * D3D12CommandContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_COMMAND_CONTEXT_H
#define LLGL_D3D12_COMMAND_CONTEXT_H


#include "../../DXCommon/ComPtr.h"
#include "../RenderState/D3D12Fence.h"
#include "../RenderState/D3D12PipelineLayout.h"
#include "../RenderState/D3D12StagingDescriptorHeapPool.h"
#include "../RenderState/D3D12DescriptorCache.h"
#include "../Buffer/D3D12StagingBufferPool.h"
#include "../Buffer/D3D12IntermediateBufferPool.h"
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
            D3D12_COMMAND_LIST_TYPE commandListType         = D3D12_COMMAND_LIST_TYPE_DIRECT,
            UINT                    numAllocators           = ~0u,
            UINT64                  initialStagingChunkSize = (0xFFFF + 1),
            bool                    initialClose            = false
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
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_STATES oldState, bool flushImmediate = false);
        void TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);

        // Insert a resource barrier for an unordered access view (UAV).
        void InsertUAVBarrier(D3D12Resource& resource, bool flushImmediate = false);

        // Flush all accumulated resource barriers.
        void FlushResourceBarrieres();

        void ResolveSubresource(
            D3D12Resource&  dstResource,
            UINT            dstSubresource,
            D3D12Resource&  srcResource,
            UINT            srcSubresource,
            DXGI_FORMAT     format
        );

        void CopyTextureRegion(
            D3D12Resource&      dstResource,
            UINT                dstSubresource,
            UINT                dstX,
            UINT                dstY,
            UINT                dstZ,
            D3D12Resource&      srcResource,
            UINT                srcSubresource,
            const D3D12_BOX*    srcBox
        );

        void UpdateSubresource(
            D3D12Resource&  dstResource,
            UINT64          dstOffset,
            const void*     data,
            UINT64          dataSize
        );

        ID3D12Resource* AllocIntermediateBuffer(UINT64 size, UINT alignment = 256u);

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

        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptorsForStaging(
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        void EmplaceDescriptorForStaging(
            Resource&                   resource,
            UINT                        location,
            D3D12_DESCRIPTOR_RANGE_TYPE descRangeType
        );

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
            ID3D12Resource*         countBuffer             = nullptr,
            UINT64                  countBufferOffset       = 0
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
            ID3D12Resource*         countBuffer             = nullptr,
            UINT64                  countBufferOffset       = 0
        );

    public:

        // Returns the native D3D12 device this command context was created with.
        inline ID3D12Device* GetDevice() const
        {
            return device_;
        }

    private:

        static constexpr UINT maxNumAllocators          = 3;
        static constexpr UINT maxNumResourceBarrieres   = 16;
        static constexpr UINT maxNumDescriptorHeaps     = 2;

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

            ID3D12RootSignature*    graphicsRootSignature                   = nullptr;
            ID3D12RootSignature*    computeRootSignature                    = nullptr;
            ID3D12PipelineState*    pipelineState                           = nullptr;
            UINT                    numDescriptorHeaps                      = 0;
            ID3D12DescriptorHeap*   descriptorHeaps[maxNumDescriptorHeaps]  = {};
        };

    private:

        // Clears the internal cached states.
        void ClearCache();

        // Returns the next resource barrier and flushes previous barriers if the cache is full.
        D3D12_RESOURCE_BARRIER& NextResourceBarrier();

        // Switches to the next command allocator and resets it.
        void NextCommandAllocator();

        void FlushGraphicsStagingDescriptorTables();
        void FlushComputeStagingDescriptorTables();

        // Returns the current command allocator.
        inline ID3D12CommandAllocator* GetCommandAllocator() const
        {
            return commandAllocators_[currentAllocatorIndex_].Get();
        }

    private:

        ID3D12Device*                       device_                                     = nullptr;
        D3D12CommandQueue*                  commandQueue_                               = nullptr;

        ComPtr<ID3D12CommandAllocator>      commandAllocators_[maxNumAllocators];
        UINT                                currentAllocatorIndex_                      = 0;
        UINT                                numAllocators_                              = maxNumAllocators;

        UINT64                              allocatorFenceValues_[maxNumAllocators]     = {};
        D3D12NativeFence                    allocatorFence_;

        ComPtr<ID3D12GraphicsCommandList>   commandList_;

        D3D12_RESOURCE_BARRIER              resourceBarriers_[maxNumResourceBarrieres];
        UINT                                numResourceBarriers_                        = 0;

        D3D12StagingDescriptorHeapPool      stagingDescriptorPools_[maxNumAllocators][maxNumDescriptorHeaps];
        D3D12DescriptorHeapSetLayout        stagingDescriptorSetLayout_;
        D3D12RootParameterIndices           stagingDescriptorIndices_;
        D3D12DescriptorCache                descriptorCaches_[maxNumAllocators];

        D3D12StagingBufferPool              stagingBufferPools_[maxNumAllocators];
        D3D12IntermediateBufferPool         intermediateBufferPools_[maxNumAllocators];

        StateCache                          stateCache_;

};


} // /namespace LLGL


#endif



// ================================================================================
