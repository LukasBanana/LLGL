/*
 * D3D12DescriptorCache.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_DESCRIPTOR_CACHE_H
#define LLGL_D3D12_DESCRIPTOR_CACHE_H


#include "D3D12DescriptorHeap.h"


namespace LLGL
{


class Resource;
class D3D12Buffer;
class D3D12Texture;
class D3D12Sampler;
class D3D12StagingDescriptorHeapPool;

// D3D12 descriptor heap wrapper to manage shader-visible descriptor heaps.
class D3D12DescriptorCache
{

    public:

        // Initializes the cache as invaldiated.
        D3D12DescriptorCache();

        // Creates the internal native D3D descriptor heaps. These are always a shader-visible descriptor heap.
        void Create(
            ID3D12Device*   device,
            UINT            initialNumResources,
            UINT            initialNumSamplers
        );

        // Resets the descriptor heap if the size must be increased and invalidates the cache.
        void Reset(UINT numResources, UINT numSamplers);

        // Emplaces a descriptor into the cache for the specified resource.
        void EmplaceDescriptor(Resource& resource, UINT location, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType);

        // Flushes any invalidated CBV/SRV/UAV descriptors into the specified descriptor heap pools.
        D3D12_GPU_DESCRIPTOR_HANDLE FlushCbvSrvUavDescriptors(D3D12StagingDescriptorHeapPool& descHeapPool);

        // Flushes any invalidated sampler descriptors into the specified descriptor heap pools.
        D3D12_GPU_DESCRIPTOR_HANDLE FlushSamplerDescriptors(D3D12StagingDescriptorHeapPool& descHeapPool);

    private:

        bool EmplaceBufferDescriptor(D3D12Buffer& bufferD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType);
        bool EmplaceTextureDescriptor(D3D12Texture& textureD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType);
        bool EmplaceSamplerDescriptor(D3D12Sampler& samplerD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType);

    private:

        ID3D12Device*       device_             = nullptr;
        D3D12DescriptorHeap descriptorHeaps_[2];
        UINT                currentStrides_[2]  = {};

        union
        {
            struct
            {
                UINT        descHeapCbvSrvUav   : 1;
                UINT        descHeapSampler     : 1;
            };
            UINT            bits;
        }
        dirtyBits_;

};


} // /namespace LLGL


#endif



// ================================================================================
