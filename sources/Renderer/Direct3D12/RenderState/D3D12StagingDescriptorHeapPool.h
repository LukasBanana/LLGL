/*
 * D3D12StagingBufferPool.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_POOL_H
#define LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_POOL_H


#include "D3D12StagingDescriptorHeap.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


class D3D12Device;

class D3D12StagingDescriptorHeapPool
{

    public:

        D3D12StagingDescriptorHeapPool() = default;
        D3D12StagingDescriptorHeapPool(D3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);

        // Initializes the device object and chunk size.
        void InitializeDevice(D3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);

        // Resets all chunks in the pool.
        void Reset();

        // Copies the specified source descriptors into the native D3D descriptor heap.
        void CopyDescriptors(
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(UINT descriptor) const;

        // Increments the offset for the next range of descriptor handles.
        void IncrementOffset(UINT stride);

        // Returns the current descriptor heap that was used for the last CopyDescriptors operation.
        ID3D12DescriptorHeap* GetDescriptorHeap() const;

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT minNumDescriptors);

        // Restes all previously allocated chunks.
        void ResetChunks();

    private:

        D3D12Device*                            device_     = nullptr;
        D3D12_DESCRIPTOR_HEAP_TYPE              type_       = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

        std::vector<D3D12StagingDescriptorHeap> chunks_;
        std::size_t                             chunkIdx_   = 0;
        UINT                                    chunkSize_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
