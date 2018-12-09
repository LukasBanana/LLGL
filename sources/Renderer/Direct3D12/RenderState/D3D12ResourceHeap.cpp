/*
 * D3D12ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ResourceHeap.h"
//#include "D3D12PipelineLayout.h"
#include "../Buffer/D3D12Buffer.h"
#include "../Texture/D3D12Sampler.h"
#include "../Texture/D3D12Texture.h"
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
    auto cpuDescHandleCbvSrvUav = CreateHeapTypeCbvSrvUav(device, desc);
    auto cpuDescHandleSampler   = CreateHeapTypeSampler(device, desc);

    /* Create descriptors */
    CreateConstantBufferViews(device, cpuDescHandleCbvSrvUav, desc);
    CreateShaderResourceViews(device, cpuDescHandleCbvSrvUav, desc);
    CreateUnorderedAccessViews(device, cpuDescHandleCbvSrvUav, desc);
    CreateSamplers(device, cpuDescHandleSampler, desc);
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
            switch (resource->QueryResourceType())
            {
                case ResourceType::Buffer:          // SRV/UAV/CBV
                case ResourceType::Texture:         // SRV
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

        #ifdef LLGL_DEBUG
        heapTypeCbvSrvUav_->SetName(L"LLGL::D3D12ResourceHeap::heapTypeCbvSrvUav");
        #endif

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

        #ifdef LLGL_DEBUG
        heapTypeSampler_->SetName(L"LLGL::D3D12ResourceHeap::heapTypeSampler");
        #endif

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
            if (resource->QueryResourceType() == resourceType)
                callback(*resource);
        }
    }
}

void D3D12ResourceHeap::CreateConstantBufferViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc)
{
    UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        [&](Resource& resource)
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if ((bufferD3D.GetBindFlags() & BindFlags::ConstantBuffer) != 0)
            {
                bufferD3D.CreateConstantBufferView(device, cpuDescHandle);
                cpuDescHandle.ptr += cpuDescStride;
            }
        }
    );
}

void D3D12ResourceHeap::CreateShaderResourceViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc)
{
    UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Texture,
        [&](Resource& resource)
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            textureD3D.CreateResourceView(device, cpuDescHandle);
            cpuDescHandle.ptr += cpuDescStride;
        }
    );
}

void D3D12ResourceHeap::CreateUnorderedAccessViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc)
{
    //TODO
}

void D3D12ResourceHeap::CreateSamplers(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc)
{
    UINT cpuDescStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

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
