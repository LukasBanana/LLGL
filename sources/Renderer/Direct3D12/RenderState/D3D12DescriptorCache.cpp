/*
 * D3D12DescriptorCache.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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


namespace LLGL
{


D3D12DescriptorCache::D3D12DescriptorCache()
{
    dirtyBits_.bits = ~0u;
}

void D3D12DescriptorCache::Create(
    ID3D12Device*   device,
    UINT            initialNumResources,
    UINT            initialNumSamplers)
{
    device_ = device;
    descriptorHeaps_[0].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, initialNumResources);
    descriptorHeaps_[1].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, initialNumSamplers);
}

void D3D12DescriptorCache::Reset(UINT numResources, UINT numSamplers)
{
    /* Store new strides and resize descriptor heaps if necessary */
    currentStrides_[0] = numResources;
    if (descriptorHeaps_[0].GetSize() < numResources)
    {
        descriptorHeaps_[0].Reset(numResources);
        dirtyBits_.descHeapCbvSrvUav = 1;
    }

    currentStrides_[1] = numSamplers;
    if (descriptorHeaps_[1].GetSize() < numSamplers)
    {
        descriptorHeaps_[1].Reset(numSamplers);
        dirtyBits_.descHeapSampler = 1;
    }
}

void D3D12DescriptorCache::EmplaceDescriptor(Resource& resource, UINT location, D3D12_DESCRIPTOR_RANGE_TYPE descRangeType)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
            LLGL_ASSERT(location < currentStrides_[0]);
            if (EmplaceBufferDescriptor(LLGL_CAST(D3D12Buffer&, resource), descriptorHeaps_[0].GetCpuHandleWithOffset(location), descRangeType))
                dirtyBits_.descHeapCbvSrvUav = 1;
            break;

        case ResourceType::Texture:
            LLGL_ASSERT(location < currentStrides_[0]);
            if (EmplaceTextureDescriptor(LLGL_CAST(D3D12Texture&, resource), descriptorHeaps_[0].GetCpuHandleWithOffset(location), descRangeType))
                dirtyBits_.descHeapCbvSrvUav = 1;
            break;

        case ResourceType::Sampler:
            LLGL_ASSERT(location < currentStrides_[1]);
            if (EmplaceSamplerDescriptor(LLGL_CAST(D3D12Sampler&, resource), descriptorHeaps_[1].GetCpuHandleWithOffset(location), descRangeType))
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
        return descHeapPool.CopyDescriptors(descriptorHeaps_[0].GetCpuHandleStart(), 0, currentStrides_[0]);
    }
    return {};
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorCache::FlushSamplerDescriptors(D3D12StagingDescriptorHeapPool& descHeapPool)
{
    if (dirtyBits_.descHeapSampler)
    {
        dirtyBits_.descHeapSampler = 0;
        return descHeapPool.CopyDescriptors(descriptorHeaps_[1].GetCpuHandleStart(), 0, currentStrides_[1]);
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
