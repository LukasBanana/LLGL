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
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../ResourceUtils.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../CheckedCast.h"
#include "../../../Core/Exception.h"
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
    auto pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        LLGL_TRAP("failed to create resource heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    const std::uint32_t numBindings         = pipelineLayoutD3D->GetNumHeapBindings();
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    numDescriptorSets_ = numResourceViews / numBindings;

    /* Store meta data which pipelines will be used by this resource heap */
    long convolutedStageFlags = pipelineLayoutD3D->GetConvolutedStageFlags();

    /* Store descriptor handle strides per descriptor set */
    const auto& descHeapLayout  = pipelineLayoutD3D->GetDescriptorHeapLayout();
    numDescriptorsPerSet_[0]    = descHeapLayout.SumResourceViews();
    numDescriptorsPerSet_[1]    = descHeapLayout.SumSamplers();

    /* Keep copy of root parameter map */
    descriptorMap_ = pipelineLayoutD3D->GetDescriptorHeapMap();
    resources_.resize(numDescriptorSets_ * numDescriptorsPerSet_[0], nullptr);

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
    }

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        CreateResourceViewHandles(device, 0, initialResourceViews);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

static D3D12Resource* GetD3DResourceRef(Resource& resource)
{
    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            return &(bufferD3D.GetResource());
        }
        break;

        case ResourceType::Texture:
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            return &(textureD3D.GetResource());
        }
        break;

        default:
        break;
    }
    return nullptr;
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

    for (const ResourceViewDescriptor& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get CPU descriptor handle address for current root parameter */
        const std::uint32_t                 descriptorSet       = firstDescriptor / numBindings;
        const D3D12DescriptorHeapLocation&  descriptorLocation  = descriptorMap_[firstDescriptor % numBindings];
        const UINT                          handleOffset        = descriptorHandleStrides_[descriptorLocation.heap] * descriptorLocation.descriptorIndex;
        const UINT                          setOffset           = descriptorSetStrides_[descriptorLocation.heap] * (firstDescriptor / numBindings);

        cpuDescHandle.ptr = cpuDescHandles[descriptorLocation.heap].ptr + handleOffset + setOffset;

        /* Store reference to resource state */
        if (descriptorLocation.heap == 0)
        {
            const UINT descriptorHeap0Index = descriptorSet * numDescriptorsPerSet_[0] + descriptorLocation.descriptorIndex;
            LLGL_ASSERT(descriptorHeap0Index < resources_.size());
            resources_[descriptorHeap0Index] = GetD3DResourceRef(*(desc.resource));
        }

        /* Write current resource view to descriptor heap */
        switch (descriptorLocation.type)
        {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                /* Replace UAV resource to pre-compute barriers */
                if (CreateShaderResourceView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource));
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                if (CreateUnorderedAccessView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource));
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                if (CreateConstantBufferView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorLocation, descriptorSet, *(desc.resource));
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                if (CreateSampler(device, cpuDescHandle, desc))
                    ++numWritten;
                break;
        }

        ++firstDescriptor;
    }

    return numWritten;
}

void D3D12ResourceHeap::TransitionResources(D3D12CommandContext& context, std::uint32_t descriptorSet)
{
    /* Transition all D3D resources in the specified descriptor set; Only heap 0 for CBV/SRV/UAV */
    for_range(i, numDescriptorsPerSet_[0])
    {
        const D3D12DescriptorHeapLocation& descriptorLocation = descriptorMap_[i];
        const UINT descriptorHeap0Index = descriptorSet * numDescriptorsPerSet_[0] + descriptorLocation.descriptorIndex;
        LLGL_ASSERT(descriptorHeap0Index < resources_.size());
        if (D3D12Resource* resource = resources_[descriptorHeap0Index])
            context.TransitionResource(*resource, descriptorMap_[i].state);
    }
}

void D3D12ResourceHeap::InsertUAVBarriers(D3D12CommandContext& context, std::uint32_t descriptorSet)
{
    if (descriptorSet < numDescriptorSets_ && HasUAVBarriers())
    {
        for_range(index, uavResourceSetStride_)
        {
            ID3D12Resource* resource = uavResourceHeap_[descriptorSet * uavResourceSetStride_ + index];
            context.SetResourceUAVBarrier(resource, index);
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

std::uint32_t D3D12ResourceHeap::GetNumDescriptorSets() const
{
    return numDescriptorSets_;
}


/*
 * ======= Private: =======
 */

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
    auto& resource = *(desc.resource);
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
    auto& resource = *(desc.resource);
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
    auto& resource = *(desc.resource);
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
    auto& resource = *(desc.resource);
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
    Resource&                           resource)
{
    if (HasUAVBarriers())
    {
        if (resource.GetResourceType() == ResourceType::Buffer)
        {
            auto& buffer = LLGL_CAST(Buffer&, resource);
            if (IsUAVResourceBarrierRequired(buffer.GetBindFlags()))
            {
                auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, bufferD3D.GetNative());
            }
            else
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, nullptr);
        }
        else if (resource.GetResourceType() == ResourceType::Texture)
        {
            auto& texture = LLGL_CAST(Texture&, resource);
            if (IsUAVResourceBarrierRequired(texture.GetBindFlags()))
            {
                auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, textureD3D.GetNative());
            }
            else
                EmplaceD3DUAVResource(descriptorLocation, descriptorSet, nullptr);
        }
    }
}

void D3D12ResourceHeap::EmplaceD3DUAVResource(
    const D3D12DescriptorHeapLocation&  descriptorLocation,
    std::uint32_t                       descriptorSet,
    ID3D12Resource*                     resource)
{
    if (descriptorLocation.descriptorIndex >= uavResourceIndexOffset_)
    {
        const UINT uavResourceIndex = descriptorSet * uavResourceSetStride_ + descriptorLocation.descriptorIndex - uavResourceIndexOffset_;
        uavResourceHeap_[uavResourceIndex] = resource;
    }
}


} // /namespace LLGL



// ================================================================================
