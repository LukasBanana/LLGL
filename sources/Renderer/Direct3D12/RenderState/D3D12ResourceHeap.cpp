/*
 * D3D12ResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12ResourceHeap.h"
#include "D3D12PipelineLayout.h"
#include "D3D12DescriptorHeap.h"
#include "../D3D12ObjectUtils.h"
#include "../Buffer/D3D12Buffer.h"
#include "../Texture/D3D12Sampler.h"
#include "../Texture/D3D12Texture.h"
#include "../../DXCommon/DXCore.h"
#include "../../ResourceUtils.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Resource.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <functional>
#include <algorithm>


namespace LLGL
{


D3D12ResourceHeap::D3D12ResourceHeap(
    ID3D12Device*                               device,
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Get pipeline layout object */
    auto* pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    const std::uint32_t numBindings         = pipelineLayoutD3D->GetNumHeapBindings();
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    numDescriptorSets_  = numResourceViews / numBindings;
    isBindless_         = pipelineLayoutD3D->HasBindlessHeap();

    /* Store meta data which pipelines will be used by this resource heap */
    long convolutedStageFlags = pipelineLayoutD3D->GetConvolutedStageFlags();

    /* Store descriptor handle strides per descriptor set */
    const D3D12RootSignatureLayout& descHeapLayout  = pipelineLayoutD3D->GetDescriptorHeapLayout();
    numDescriptorsPerSet_[0]    = descHeapLayout.SumResourceViews();
    numDescriptorsPerSet_[1]    = descHeapLayout.SumSamplers();

    /* Keep copy of root parameter map */
    descriptorMap_ = pipelineLayoutD3D->GetDescriptorHeapMap();

    /* Create descriptor heaps */
    if (numDescriptorsPerSet_[0] > 0)
        CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptorsPerSet_[0] * numDescriptorSets_);
    if (numDescriptorsPerSet_[1] > 0)
        CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, numDescriptorsPerSet_[1] * numDescriptorSets_);

    /* Allocate barrier resources if any UAV barriers are specified */
    if ((pipelineLayoutD3D->GetBarrierFlags() & BarrierFlags::Storage) != 0)
    {
        /* Allocate empty heap for ID3D12Resource object that require a UAV barrier */
        uavResourceSetStride_   = descHeapLayout.SumUAVs();
        uavResourceIndexOffset_ = descHeapLayout.SumCBVsAndSRVs();
        uavResourceHeap_.resize(uavResourceSetStride_ * numDescriptorSets_);

        /* Allocate packed buffer for resource barriers with UINT for number of active barriers and an array of D3D12_RESOURCE_BARRIER[N] */
        barrierStride_ = sizeof(UINT) + sizeof(D3D12_RESOURCE_BARRIER) * uavResourceSetStride_;
        barriers_.resize(barrierStride_ * numDescriptorSets_);
    }

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        CreateResourceViewHandles(device, 0, initialResourceViews);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

std::uint32_t D3D12ResourceHeap::CreateResourceViewHandles(
    ID3D12Device*                               device,
    std::uint32_t                               firstDescriptor,
    const ArrayView<ResourceViewDescriptor>&    resourceViews)
{
    /* Quit if there's nothing to do */
    if (resourceViews.empty())
        return 0;

    const std::uint32_t numBindings     = static_cast<std::uint32_t>(descriptorMap_.size());
    const std::uint32_t numDescriptors  = numDescriptorSets_ * numBindings;

    /* Silently quit on out of bounds; debug layer must report these errors */
    if (firstDescriptor >= numDescriptors)
        return 0;
    if (firstDescriptor + resourceViews.size() > numDescriptors)
        return 0;

    /* Get CPU descriptor heap starts */
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = {};
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandles[2] = {};

    if (descriptorHeaps_[0])
        cpuDescHandles[0] = descriptorHeaps_[0]->GetCPUDescriptorHandleForHeapStart();
    if (descriptorHeaps_[1])
        cpuDescHandles[1] = descriptorHeaps_[1]->GetCPUDescriptorHandleForHeapStart();

    /* Write each resource view into respective descriptor heap */
    std::uint32_t numWritten = 0;
    std::uint32_t uavChangeSetRange[2] = { UINT32_MAX, 0 };

    for (const ResourceViewDescriptor& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get CPU descriptor handle address for current root parameter */
        const std::uint32_t                 descriptorSet       = firstDescriptor / numBindings;
        const D3D12DescriptorHeapLocation&  descriptorLocation  = descriptorMap_[firstDescriptor % numBindings];
        const UINT                          handleOffset        = descriptorHandleStrides_[descriptorLocation.heap] * descriptorLocation.index;
        const UINT                          setOffset           = descriptorSetStrides_[descriptorLocation.heap] * (firstDescriptor / numBindings);

        cpuDescHandle.ptr = cpuDescHandles[descriptorLocation.heap].ptr + handleOffset + setOffset;

        /* Write current resource view to descriptor heap */
        switch (descriptorLocation.type)
        {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                /* Replace UAV resource to pre-compute barriers */
                if (CreateShaderResourceView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource), uavChangeSetRange);
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                if (CreateUnorderedAccessView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource), uavChangeSetRange);
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                if (CreateConstantBufferView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource), uavChangeSetRange);
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                if (CreateSampler(device, cpuDescHandle, desc))
                    ++numWritten;
                break;
        }

        ++firstDescriptor;
    }

    /* Update resource barriers for all affected descriptor sets if any of the UAV resource entries have changed */
    for_subrange(i, uavChangeSetRange[0], uavChangeSetRange[1])
        UpdateBarriers(i);

    return numWritten;
}

void D3D12ResourceHeap::InsertResourceBarriers(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet)
{
    if (descriptorSet < numDescriptorSets_ && HasBarriers())
    {
        const char* barrierHeapStart = &barriers_[descriptorSet * barrierStride_];
        const UINT numBarriers = *reinterpret_cast<const UINT*>(barrierHeapStart);
        if (numBarriers > 0)
        {
            const auto* barriers = reinterpret_cast<const D3D12_RESOURCE_BARRIER*>(barrierHeapStart + sizeof(UINT));
            commandList->ResourceBarrier(numBarriers, barriers);
        }
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12ResourceHeap::GetCPUDescriptorHandleForHeapStart(D3D12_DESCRIPTOR_HEAP_TYPE heapType, std::uint32_t descriptorSet) const
{
    const UINT heapTypeIndex = static_cast<UINT>(heapType);
    LLGL_ASSERT(heapTypeIndex < sizeof(descriptorHeaps_)/sizeof(descriptorHeaps_[0]));
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = descriptorHeaps_[heapTypeIndex]->GetCPUDescriptorHandleForHeapStart();
    cpuDescHandle.ptr += descriptorSetStrides_[heapTypeIndex] * descriptorSet;
    return cpuDescHandle;
}

UINT D3D12ResourceHeap::GetNumDescriptorsPerSet(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const
{
    const UINT heapTypeIndex = static_cast<UINT>(heapType);
    LLGL_ASSERT(heapTypeIndex < sizeof(descriptorHeaps_)/sizeof(descriptorHeaps_[0]));
    return numDescriptorsPerSet_[heapTypeIndex];
}

ID3D12DescriptorHeap* D3D12ResourceHeap::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const
{
    const UINT heapTypeIndex = static_cast<UINT>(heapType);
    LLGL_ASSERT(heapTypeIndex < sizeof(descriptorHeaps_)/sizeof(descriptorHeaps_[0]));
    return descriptorHeaps_[static_cast<UINT>(heapType)].Get();
}

void D3D12ResourceHeap::SetDebugName(const char* name)
{
    D3D12SetObjectNameSubscript(descriptorHeaps_[0].Get(), name, ".ResourceViews");
    D3D12SetObjectNameSubscript(descriptorHeaps_[1].Get(), name, ".Samplers");
}

bool D3D12ResourceHeap::IsBindless() const
{
    return isBindless_;
}

std::uint32_t D3D12ResourceHeap::GetNumDescriptorSets() const
{
    return numDescriptorSets_;
}


/*
 * ======= Private: =======
 */

static void ErrNullPointerInResource()
{
    throw std::invalid_argument("cannot create resource heap with null pointer in resource view");
}

void D3D12ResourceHeap::CreateDescriptorHeap(
    ID3D12Device*                   device,
    D3D12_DESCRIPTOR_HEAP_TYPE      heapType,
    UINT                            numDescriptors)
{
    const UINT heapTypeIndex = static_cast<UINT>(heapType);
    LLGL_ASSERT(heapTypeIndex < sizeof(descriptorHeaps_)/sizeof(descriptorHeaps_[0]));

    /* Store handle stride for heap type */
    const UINT descHandleStride = device->GetDescriptorHandleIncrementSize(heapType);
    descriptorHandleStrides_[heapTypeIndex] = descHandleStride;
    descriptorSetStrides_[heapTypeIndex]    = descHandleStride * numDescriptorsPerSet_[heapTypeIndex];

    /*
    Create shader-invisible descriptor heap.
    During binding to the command context, the descriptors will be copied to a shader-visible descriptor heap.
    */
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type           = heapType;
        heapDesc.NumDescriptors = numDescriptors;
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask       = 0;
    }
    descriptorHeaps_[heapTypeIndex] = D3D12DescriptorHeap::CreateNativeOrThrow(device, heapDesc);
}

bool D3D12ResourceHeap::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc)
{
    /* Get D3D resource with SRV binding flags */
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
        if ((bufferD3D.GetBindFlags() & BindFlags::Sampled) != 0)
        {
            /* Create shader resource view (SRV) for D3D buffer */
            if (IsBufferViewEnabled(desc.bufferView))
                bufferD3D.CreateShaderResourceView(device, cpuDescHandle, desc.bufferView);
            else
                bufferD3D.CreateShaderResourceView(device, cpuDescHandle);
            return true;
        }
    }
    else if (resource.GetResourceType() == ResourceType::Texture)
    {
        auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
        if ((textureD3D.GetBindFlags() & BindFlags::Sampled) != 0)
        {
            /* Create shader resource view (SRV) for D3D texture */
            if (IsTextureViewEnabled(desc.textureView))
                textureD3D.CreateShaderResourceView(device, cpuDescHandle, desc.textureView);
            else
                textureD3D.CreateShaderResourceView(device, cpuDescHandle);
            return true;
        }
    }
    return false;
}

bool D3D12ResourceHeap::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc)
{
    /* Get D3D resource with UAV binding flags */
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
        if ((bufferD3D.GetBindFlags() & BindFlags::Storage) != 0)
        {
            /* Create unordered access view (UAV) for D3D buffer */
            if (IsBufferViewEnabled(desc.bufferView))
                bufferD3D.CreateUnorderedAccessView(device, cpuDescHandle, desc.bufferView);
            else
                bufferD3D.CreateUnorderedAccessView(device, cpuDescHandle);
            return true;
        }
    }
    else if (resource.GetResourceType() == ResourceType::Texture)
    {
        auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
        if ((textureD3D.GetBindFlags() & BindFlags::Storage) != 0)
        {
            /* Create unordered access view (UAV) for D3D texture */
            if (IsTextureViewEnabled(desc.textureView))
                textureD3D.CreateUnorderedAccessView(device, cpuDescHandle, desc.textureView);
            else
                textureD3D.CreateUnorderedAccessView(device, cpuDescHandle);
            return true;
        }
    }
    return false;
}

bool D3D12ResourceHeap::CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc)
{
    /* Get D3D resource with CBV binding flags */
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
        if ((bufferD3D.GetBindFlags() & BindFlags::ConstantBuffer) != 0)
        {
            /* Create constant buffer view (CBV) for D3D buffer */
            if (IsBufferViewEnabled(desc.bufferView))
                bufferD3D.CreateConstantBufferView(device, cpuDescHandle, desc.bufferView);
            else
                bufferD3D.CreateConstantBufferView(device, cpuDescHandle);
            return true;
        }
    }
    return false;
}

bool D3D12ResourceHeap::CreateSampler(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc)
{
    /* Get D3D sampler resource */
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Sampler)
    {
        /* Create sampler state for D3D sampler */
        auto& samplerD3D = LLGL_CAST(D3D12Sampler&, resource);
        samplerD3D.CreateResourceView(device, cpuDescHandle);
        return true;
    }
    return false;
}

static bool IsUAVResourceBarrierRequired(long bindFlags)
{
    return ((bindFlags & BindFlags::Storage) != 0);
}

void D3D12ResourceHeap::ExchangeUAVResource(
    const D3D12DescriptorHeapLocation&  descriptorLocation,
    std::uint32_t                       descriptorSet,
    Resource&                           resource,
    std::uint32_t                       (&setRange)[2])
{
    if (HasBarriers())
    {
        if (resource.GetResourceType() == ResourceType::Buffer)
        {
            auto& buffer = LLGL_CAST(Buffer&, resource);
            if (IsUAVResourceBarrierRequired(buffer.GetBindFlags()))
            {
                auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, bufferD3D.GetNative(), setRange);
            }
            else
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, nullptr, setRange);
        }
        else if (resource.GetResourceType() == ResourceType::Texture)
        {
            auto& texture = LLGL_CAST(Texture&, resource);
            if (IsUAVResourceBarrierRequired(texture.GetBindFlags()))
            {
                auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, textureD3D.GetNative(), setRange);
            }
            else
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, nullptr, setRange);
        }
    }
}

void D3D12ResourceHeap::EmplaceD3DUAVResource(
    const D3D12DescriptorHeapLocation&  descriptorLocation,
    std::uint32_t                       descriptorSet,
    ID3D12Resource*                     resource,
    std::uint32_t                       (&setRange)[2])
{
    if (descriptorLocation.index >= uavResourceIndexOffset_)
    {
        ID3D12Resource*& cached = uavResourceHeap_[descriptorSet * uavResourceSetStride_ + descriptorLocation.index - uavResourceIndexOffset_];
        if (cached != resource)
        {
            cached      = resource;
            setRange[0] = std::min(setRange[0], descriptorSet);
            setRange[1] = std::max(setRange[1], descriptorSet + 1);
        }
    }
}

void D3D12ResourceHeap::UpdateBarriers(std::uint32_t descriptorSet)
{
    char* barrierHeapStart = &barriers_[descriptorSet * barrierStride_];
    auto* barriers = reinterpret_cast<D3D12_RESOURCE_BARRIER*>(barrierHeapStart + sizeof(UINT));

    /* Write new barriers for entire descriptor set */
    UINT numBarriers = 0;
    for_range(i, uavResourceSetStride_)
    {
        if (ID3D12Resource* resource = uavResourceHeap_[descriptorSet * uavResourceSetStride_ + i])
        {
            D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriers++];
            {
                barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.UAV.pResource   = resource;
            }
        }
    }

    /* Write new number of barriers at start of barrier heap */
    *reinterpret_cast<UINT*>(barrierHeapStart) = numBarriers;
}


} // /namespace LLGL



// ================================================================================
