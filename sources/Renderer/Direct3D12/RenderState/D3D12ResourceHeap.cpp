/*
 * D3D12ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ResourceHeap.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12ObjectUtils.h"
#include "../Buffer/D3D12Buffer.h"
#include "../Texture/D3D12Sampler.h"
#include "../Texture/D3D12Texture.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include <LLGL/Resource.h>
#include <LLGL/ResourceHeapFlags.h>
#include <functional>


namespace LLGL
{


D3D12ResourceHeap::D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Create descriptor heaps */
    auto cpuDescHandleCbvSrvUav = CreateHeapTypeCbvSrvUav(device, desc);
    auto cpuDescHandleSampler   = CreateHeapTypeSampler(device, desc);

    /* Create descriptors */
    std::size_t bindingIndex = 0;
    CreateConstantBufferViews(device, desc, cpuDescHandleCbvSrvUav, bindingIndex);
    CreateShaderResourceViews(device, desc, cpuDescHandleCbvSrvUav, bindingIndex);
    CreateUnorderedAccessViews(device, desc, cpuDescHandleCbvSrvUav, bindingIndex);
    CreateSamplers(device, desc, cpuDescHandleSampler);
}

void D3D12ResourceHeap::SetName(const char* name)
{
    D3D12SetObjectNameSubscript(heapTypeCbvSrvUav_.Get(), name, ".CbvSrvUav");
    D3D12SetObjectNameSubscript(heapTypeSampler_.Get(), name, ".Sampler");
}


/*
 * ======= Private: =======
 */

static void ErrNullPointerInResource()
{
    throw std::invalid_argument("cannot create resource heap with null pointer in resource view");
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12ResourceHeap::CreateHeapTypeCbvSrvUav(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Determine number of view descriptors */
    UINT numDescriptors = 0;

    for (const auto& resourceView : desc.resourceViews)
    {
        if (auto resource = resourceView.resource)
        {
            /* Search SRV/UAV/CBV resources */
            auto resType = resource->GetResourceType();
            if (resType == ResourceType::Buffer || resType == ResourceType::Texture)
                ++numDescriptors;
        }
        else
            ErrNullPointerInResource();
    }

    if (numDescriptors > 0)
    {
        /* Create descriptor heap for views (CBV, SRV, UAV) */
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        {
            heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.NumDescriptors = numDescriptors;
            heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            heapDesc.NodeMask       = 0;
        }
        auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(heapTypeCbvSrvUav_.ReleaseAndGetAddressOf()));
        DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap of type D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV");

        /* Store in array for quick access */
        AppendDescriptorHeapToArray(heapTypeCbvSrvUav_.Get());

        return heapTypeCbvSrvUav_->GetCPUDescriptorHandleForHeapStart();
    }

    return {};
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12ResourceHeap::CreateHeapTypeSampler(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Determine number of samplers descriptors */
    UINT numDescriptors = 0;

    for (const auto& resourceView : desc.resourceViews)
    {
        if (auto resource = resourceView.resource)
        {
            /* Search samplers */
            if (resource->GetResourceType() == ResourceType::Sampler)
                ++numDescriptors;
        }
        else
            ErrNullPointerInResource();
    }

    if (numDescriptors > 0)
    {
        /* Create descriptor heap for samplers */
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
        {
            heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            heapDesc.NumDescriptors = numDescriptors;
            heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            heapDesc.NodeMask       = 0;
        }
        auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(heapTypeSampler_.ReleaseAndGetAddressOf()));
        DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap of type D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER");

        /* Store in array for quick access */
        AppendDescriptorHeapToArray(heapTypeSampler_.Get());

        return heapTypeSampler_->GetCPUDescriptorHandleForHeapStart();
    }

    return {};
}

static void ForEachResourceViewOfType(
    const ResourceHeapDescriptor&                   desc,
    const ResourceType                              resourceType,
    const std::function<void(Resource& resource)>&  callback)
{
    for (const auto& resourceView : desc.resourceViews)
    {
        if (auto resource = resourceView.resource)
        {
            if (resource->GetResourceType() == resourceType)
                callback(*resource);
        }
    }
}

static const D3D12PipelineLayout* GetD3DPipelineLayout(const ResourceHeapDescriptor& desc)
{
    if (!desc.pipelineLayout)
        throw std::invalid_argument("cannot create resource heap without pipeline layout");
    return LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
}

// Returns true if the specified resource binding flags match the binding flags in the pipeline layout; if true, the binding index is increased.
static bool MatchBindFlags(
    const D3D12PipelineLayout&  pipelineLayout,
    long                        resourceBindFlags,
    long                        requiredBindFlags,
    std::size_t&                bindingIndex)
{
    if ((resourceBindFlags & requiredBindFlags) != 0 &&
        (pipelineLayout.GetBindFlagsByIndex(bindingIndex) & requiredBindFlags) != 0)
    {
        ++bindingIndex;
        return true;
    }
    return false;
}

void D3D12ResourceHeap::CreateConstantBufferViews(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex)
{
    UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        [&](Resource& resource)
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::ConstantBuffer, bindingIndex))
            {
                bufferD3D.CreateConstantBufferView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );
}

void D3D12ResourceHeap::CreateShaderResourceViews(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex)
{
    const UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    /* First create SRVs for all sampled-buffers; it needs to be in the same order as the root parameters are build in <D3D12PipelineLayout> */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        [&](Resource& resource)
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::Sampled, bindingIndex))
            {
                bufferD3D.CreateShaderResourceView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );

    /* Now create SRVs for all sampled-textures */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Texture,
        [&](Resource& resource)
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, textureD3D.GetBindFlags(), BindFlags::Sampled, bindingIndex))
            {
                textureD3D.CreateShaderResourceView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );
}

void D3D12ResourceHeap::CreateUnorderedAccessViews(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex)
{
    const UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    /* First create UAVs for all RW-buffers; it needs to be in the same order as the root parameters are build in <D3D12PipelineLayout> */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        [&](Resource& resource)
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::Storage, bindingIndex))
            {
                bufferD3D.CreateUnorderedAccessView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );

    /* Now create UAVs for all RW-textures */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Texture,
        [&](Resource& resource)
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, textureD3D.GetBindFlags(), BindFlags::Storage, bindingIndex))
            {
                textureD3D.CreateUnorderedAccessView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );
}

void D3D12ResourceHeap::CreateSamplers(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle)
{
    const UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Sampler,
        [&](Resource& resource)
        {
            auto& samplerD3D = LLGL_CAST(D3D12Sampler&, resource);
            samplerD3D.CreateResourceView(device, cpuDescHandle);
            cpuDescHandle.ptr += cpuDescStride;
        }
    );
}

void D3D12ResourceHeap::AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap)
{
    descriptorHeaps_[numDescriptorHeaps_++] = descriptorHeap;
}


} // /namespace LLGL



// ================================================================================
