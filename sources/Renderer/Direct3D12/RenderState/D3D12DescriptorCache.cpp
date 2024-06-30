/*
 * D3D12DescriptorCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12DescriptorCache.h"
#include "D3D12StagingDescriptorHeapPool.h"
#include "../Buffer/D3D12Buffer.h"
#include "../Texture/D3D12Texture.h"
#include "../Texture/D3D12Sampler.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <thread>
#include <chrono>
#include <algorithm>


namespace LLGL
{


static constexpr UINT g_dhIndexCbvSrvUav    = 0;
static constexpr UINT g_dhIndexSampler      = 1;
static constexpr UINT g_dhMinCacheSizes[]   = { 64, 16 };

D3D12DescriptorCache::D3D12DescriptorCache()
{
    Clear();
}

void D3D12DescriptorCache::Create(
    ID3D12Device*   device,
    UINT            initialNumResources,
    UINT            initialNumSamplers)
{
    device_ = device;
    descriptorHeaps_[g_dhIndexCbvSrvUav].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, std::max(g_dhMinCacheSizes[g_dhIndexCbvSrvUav], initialNumResources));
    descriptorHeaps_[g_dhIndexSampler  ].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,     std::max(g_dhMinCacheSizes[g_dhIndexSampler  ], initialNumSamplers ));
}

void D3D12DescriptorCache::Reset(UINT numResources, UINT numSamplers)
{
    /* Store new sizes and resize descriptor heaps if necessary */
    currentStrides_[g_dhIndexCbvSrvUav] = numResources;
    if (descriptorHeaps_[g_dhIndexCbvSrvUav].GetSize() < numResources)
    {
        descriptorHeaps_[g_dhIndexCbvSrvUav].Reset(numResources);
        dirtyBits_.descHeapCbvSrvUav = 1;
    }

    currentStrides_[g_dhIndexSampler] = numSamplers;
    if (descriptorHeaps_[g_dhIndexSampler].GetSize() < numSamplers)
    {
        descriptorHeaps_[g_dhIndexSampler].Reset(numSamplers);
        dirtyBits_.descHeapSampler = 1;
    }
}

void D3D12DescriptorCache::Clear()
{
    dirtyBits_.descHeapCbvSrvUav    = 1;
    dirtyBits_.descHeapSampler      = 1;
}

void D3D12DescriptorCache::EmplaceDescriptor(Resource& resource, UINT location, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
            LLGL_ASSERT(location < currentStrides_[g_dhIndexCbvSrvUav]);
            if (EmplaceBufferDescriptor(LLGL_CAST(D3D12Buffer&, resource), descriptorHeaps_[g_dhIndexCbvSrvUav].GetCpuHandleWithOffset(location), descRangeType))
                dirtyBits_.descHeapCbvSrvUav = 1;
            break;

        case ResourceType::Texture:
            LLGL_ASSERT(location < currentStrides_[g_dhIndexCbvSrvUav]);
            if (EmplaceTextureDescriptor(LLGL_CAST(D3D12Texture&, resource), descriptorHeaps_[g_dhIndexCbvSrvUav].GetCpuHandleWithOffset(location), descRangeType))
                dirtyBits_.descHeapCbvSrvUav = 1;
            break;

        case ResourceType::Sampler:
            LLGL_ASSERT(location < currentStrides_[g_dhIndexSampler]);
            if (EmplaceSamplerDescriptor(LLGL_CAST(D3D12Sampler&, resource), descriptorHeaps_[g_dhIndexSampler].GetCpuHandleWithOffset(location), descRangeType))
                dirtyBits_.descHeapSampler = 1;
            break;

        default:
            break;
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorCache::FlushCbvSrvUavDescriptors(D3D12StagingDescriptorHeapPool& descHeapPool)
{
    if (dirtyBits_.descHeapCbvSrvUav)
    {
        dirtyBits_.descHeapCbvSrvUav = 0;
        return descHeapPool.CopyDescriptors(descriptorHeaps_[g_dhIndexCbvSrvUav].GetCpuHandleStart(), 0, currentStrides_[g_dhIndexCbvSrvUav]);
    }
    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorCache::FlushSamplerDescriptors(D3D12StagingDescriptorHeapPool& descHeapPool)
{
    if (dirtyBits_.descHeapSampler)
    {
        dirtyBits_.descHeapSampler = 0;
        return descHeapPool.CopyDescriptors(descriptorHeaps_[g_dhIndexSampler].GetCpuHandleStart(), 0, currentStrides_[g_dhIndexSampler]);
    }
    return {};
}


/*
 * ======= Private: =======
 */

bool D3D12DescriptorCache::EmplaceBufferDescriptor(D3D12Buffer& bufferD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType)
{
    switch (descRangeType)
    {
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            bufferD3D.CreateShaderResourceView(device_, cpuDescHandle);
            return true;

        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            bufferD3D.CreateUnorderedAccessView(device_, cpuDescHandle);
            return true;

        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            bufferD3D.CreateConstantBufferView(device_, cpuDescHandle);
            return true;

        default:
            return false;
    }
}

bool D3D12DescriptorCache::EmplaceTextureDescriptor(D3D12Texture& textureD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType)
{
    switch (descRangeType)
    {
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            textureD3D.CreateShaderResourceView(device_, cpuDescHandle);
            return true;

        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            textureD3D.CreateUnorderedAccessView(device_, cpuDescHandle);
            return true;

        default:
            return false;
    }
}

bool D3D12DescriptorCache::EmplaceSamplerDescriptor(D3D12Sampler& samplerD3D, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType)
{
    switch (descRangeType)
    {
        case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
            samplerD3D.CreateResourceView(device_, cpuDescHandle);
            return true;

        default:
            return false;
    }
}


} // /namespace LLGL



// ================================================================================
