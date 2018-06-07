/*
 * D3D12ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ResourceHeap.h"
//#include "D3D12PipelineLayout.h"
#include "../Buffer/D3D12ConstantBuffer.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include <LLGL/Resource.h>
#include <functional>


namespace LLGL
{


D3D12ResourceHeap::D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Create descriptor heaps */
    CreateHeapTypeCbvSrvUav(device, desc);
    CreateHeapTypeSampler(device, desc);

    /* Create descriptors */
    CreateConstantBufferViews(device, desc);
    CreateShaderResourceViews(device, desc);
    CreateUnorderedAccessViews(device, desc);
    CreateSamplers(device, desc);
}


/*
 * ======= Private: =======
 */

static void ErrNullPointerInResource()
{
    throw std::invalid_argument("cannot create resource heap with null pointer in resource view");
}

void D3D12ResourceHeap::CreateHeapTypeCbvSrvUav(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Determine number of view descriptors */
    UINT numDescriptors = 0;

    for (const auto& resourceView : desc.resourceViews)
    {
        if (auto resource = resourceView.resource)
        {
            switch (resource->QueryResourceType())
            {
                case ResourceType::ConstantBuffer:  // CBV
                case ResourceType::Texture:         // SRV
                case ResourceType::StorageBuffer:   // UAV
                    ++numDescriptors;
                    break;
                default:
                    break;
            }
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
    }
}

void D3D12ResourceHeap::CreateHeapTypeSampler(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Determine number of samplers descriptors */
    UINT numDescriptors = 0;

    for (const auto& resourceView : desc.resourceViews)
    {
        if (auto resource = resourceView.resource)
        {
            switch (resource->QueryResourceType())
            {
                case ResourceType::Sampler:
                    ++numDescriptors;
                    break;
                default:
                    break;
            }
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
    }
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
            if (resource->QueryResourceType() == resourceType)
                callback(*resource);
        }
    }
}

void D3D12ResourceHeap::CreateConstantBufferViews(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    ForEachResourceViewOfType(
        desc, ResourceType::ConstantBuffer,
        [&](Resource& resource)
        {
            auto& constantBufferD3D = LLGL_CAST(D3D12ConstantBuffer&, resource);
            constantBufferD3D.CreateResourceView(device, heapTypeCbvSrvUav_.Get());
        }
    );
}

void D3D12ResourceHeap::CreateShaderResourceViews(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    //TODO
}

void D3D12ResourceHeap::CreateUnorderedAccessViews(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    //TODO
}

void D3D12ResourceHeap::CreateSamplers(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    //TODO
}

void D3D12ResourceHeap::AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap)
{
    descriptorHeaps_[numDescriptorHeaps_++] = descriptorHeap;
}


} // /namespace LLGL



// ================================================================================
