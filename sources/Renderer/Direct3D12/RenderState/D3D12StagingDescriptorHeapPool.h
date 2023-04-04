/*
 * D3D12StagingDescriptorHeapPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_POOL_H
#define LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_POOL_H


#include "D3D12StagingDescriptorHeap.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


/*
Pool of D3D12 staging descriptor heaps.
The number of chunks in this pools is preferrably always 1, so the initial chunk should be allocated with a decent size.
*/
class D3D12StagingDescriptorHeapPool
{

    public:

        D3D12StagingDescriptorHeapPool() = default;
        D3D12StagingDescriptorHeapPool(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);

        // Initializes the device object and chunk size.
        void InitializeDevice(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type);

        // Resets all chunks in the pool.
        void Reset();

        // Copies the specified source descriptors into the native D3D descriptor heap.
        D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptors(
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        // Returns the current descriptor heap that was used for the last CopyDescriptors operation.
        ID3D12DescriptorHeap* GetDescriptorHeap() const;

        // Returns the GPU descriptor handle at the current offset.
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandleWithOffset() const;

        // Returns the CPU descriptor handle at the current offset plus the specified descriptor index.
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleWithOffset(UINT descriptor) const;

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT minNumDescriptors);

        // Restes all previously allocated chunks.
        void ResetChunks();

        // Increments the offset for the next range of descriptor handles.
        void IncrementOffset(UINT stride);

    private:

        ID3D12Device*                           device_         = nullptr;
        D3D12_DESCRIPTOR_HEAP_TYPE              type_           = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

        std::vector<D3D12StagingDescriptorHeap> chunks_;
        std::size_t                             chunkIdx_       = 0;
        UINT                                    chunkSize_      = 0;

        UINT                                    pendingOffset_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
