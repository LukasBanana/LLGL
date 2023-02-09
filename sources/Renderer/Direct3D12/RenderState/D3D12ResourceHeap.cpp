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
#include "../../ResourceUtils.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Resource.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Misc/ForRange.h>
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
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    const auto numBindings      = pipelineLayoutD3D->GetNumBindings();
    const auto numResourceViews = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    numDescriptorSets_ = numResourceViews / numBindings;

    /* Store meta data which pipelines will be used by this resource heap */
    auto convolutedStageFlags = pipelineLayoutD3D->GetConvolutedStageFlags();

    hasGraphicsDescriptors_ = ((convolutedStageFlags & StageFlags::AllGraphicsStages) != 0);
    hasComputeDescriptors_  = ((convolutedStageFlags & StageFlags::ComputeStage     ) != 0);

    /* Store descriptor handle strides per descriptor set */
    const auto descHandleStrideCbvSrvUav = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const auto descHandleStrideSampler = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    const auto& descHeapLayout          = pipelineLayoutD3D->GetDescriptorHeapLayout();
    const auto numResourceViewHandles   = descHeapLayout.SumResourceViews();
    const auto numSamplerHandles        = descHeapLayout.SumSamplers();

    descriptorSetStrides_[0] = descHandleStrideCbvSrvUav * numResourceViewHandles;
    descriptorSetStrides_[1] = descHandleStrideSampler * numSamplerHandles;

    /* Keep copy of root parameter map */
    descriptorHandleMap_ = pipelineLayoutD3D->GetDescriptorHandleMap();

    /* Create descriptor heaps */
    if (numResourceViewHandles > 0)
        CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numResourceViewHandles * numDescriptorSets_, descriptorHeapResourceViews_);
    if (numSamplerHandles > 0)
        CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, numSamplerHandles * numDescriptorSets_, descriptorHeapSamplers_);

    /* Allocate barrier resources if any UAV barriers are specified */
    if ((desc.barrierFlags & BarrierFlags::Storage) != 0)
    {
        /* Allocate empty heap for ID3D12Resource object that require a UAV barrier */
        uavResourceSetStride_   = descHeapLayout.SumUAVs();
        uavResourceIndexOffset_ = descHeapLayout.SumNonUAVs();
        uavResourceHeap_.resize(uavResourceSetStride_ * numDescriptorSets_);

        /* Allocate packed buffer for resource barriers with UINT for number of active barriers and an array of D3D12_RESOURCE_BARRIER[N] */
        barrierStride_ = sizeof(UINT) + sizeof(D3D12_RESOURCE_BARRIER) * uavResourceSetStride_;
        barriers_.resize(barrierStride_ * numDescriptorSets_);
    }

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        CreateResourceViewHandles(device, 0, initialResourceViews);
}

std::uint32_t D3D12ResourceHeap::CreateResourceViewHandles(
    ID3D12Device*                               device,
    std::uint32_t                               firstDescriptor,
    const ArrayView<ResourceViewDescriptor>&    resourceViews)
{
    /* Quit if there's nothing to do */
    if (resourceViews.empty())
        return 0;

    const auto numBindings      = static_cast<std::uint32_t>(descriptorHandleMap_.size());
    const auto numDescriptors   = numDescriptorSets_ * numBindings;

    /* Silently quit on out of bounds; debug layer must report these errors */
    if (firstDescriptor >= numDescriptors)
        return 0;
    if (firstDescriptor + resourceViews.size() > numDescriptors)
        return 0;

    /* Get CPU descriptor heap starts */
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = {};
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandles[2] = {};

    if (descriptorHeaps_[0] != 0)
        cpuDescHandles[0] = descriptorHeaps_[0]->GetCPUDescriptorHandleForHeapStart();
    if (descriptorHeaps_[1] != 0)
        cpuDescHandles[1] = descriptorHeaps_[1]->GetCPUDescriptorHandleForHeapStart();

    /* Write each resource view into respective descriptor heap */
    std::uint32_t numWritten = 0;
    std::uint32_t uavChangeSetRange[2] = {};

    for (const auto& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get CPU descriptor handle address for current root parameter */
        const auto  descriptorSet       = firstDescriptor / numBindings;
        const auto& descriptorHandle    = descriptorHandleMap_[firstDescriptor % numBindings];
        const auto  handleOffset        = descriptorHandleStrides_[descriptorHandle.heap] * descriptorHandle.index;
        const auto  setOffset           = descriptorSetStrides_[descriptorHandle.heap] * (firstDescriptor / numBindings);

        cpuDescHandle.ptr = cpuDescHandles[descriptorHandle.heap].ptr + handleOffset + setOffset;

        /* Write current resource view to descriptor heap */
        switch (descriptorHandle.type)
        {
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                /* Replace UAV resource to pre-compute barriers */
                if (CreateShaderResourceView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorHandle, descriptorSet, *(desc.resource), uavChangeSetRange);
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                if (CreateUnorderedAccessView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorHandle, descriptorSet, *(desc.resource), uavChangeSetRange);
                }
                break;

            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                if (CreateConstantBufferView(device, cpuDescHandle, desc))
                {
                    ++numWritten;
                    ExchangeUAVResource(descriptorHandle, descriptorSet, *(desc.resource), uavChangeSetRange);
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

void D3D12ResourceHeap::SetGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet)
{
    if (hasGraphicsDescriptors_)
    {
        /* Bind root descriptor tables to graphics pipeline */
        for_range(i, numDescriptorHeaps_)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorHeaps_[i]->GetGPUDescriptorHandleForHeapStart();
            gpuDescHandle.ptr += descriptorSetStrides_[i] * descriptorSet;
            commandList->SetGraphicsRootDescriptorTable(i, gpuDescHandle);
        }
    }
}

void D3D12ResourceHeap::SetComputeRootDescriptorTables(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet)
{
    if (hasComputeDescriptors_)
    {
        /* Bind root descriptor tables to compute pipeline */
        for_range(i, numDescriptorHeaps_)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorHeaps_[i]->GetGPUDescriptorHandleForHeapStart();
            gpuDescHandle.ptr += descriptorSetStrides_[i] * descriptorSet;
            commandList->SetComputeRootDescriptorTable(i, gpuDescHandle);
        }
    }
}

void D3D12ResourceHeap::InsertResourceBarriers(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet)
{
    if (descriptorSet < numDescriptorSets_ && HasBarriers())
    {
        const auto barrierHeapStart = &barriers_[descriptorSet * barrierStride_];
        const auto numBarriers = *reinterpret_cast<const UINT*>(barrierHeapStart);
        if (numBarriers > 0)
        {
            const auto barriers = reinterpret_cast<const D3D12_RESOURCE_BARRIER*>(barrierHeapStart + sizeof(UINT));
            commandList->ResourceBarrier(numBarriers, barriers);
        }
    }
}

void D3D12ResourceHeap::SetName(const char* name)
{
    D3D12SetObjectNameSubscript(descriptorHeapResourceViews_.Get(), name, ".ResourceViews");
    D3D12SetObjectNameSubscript(descriptorHeapSamplers_.Get(), name, ".Samplers");
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

static const char* GetContextInfoForFailedDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    switch (heapType)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:    return "for heap type D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV";
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:        return "for heap type D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER";
        default:                                        return nullptr;
    }
}

void D3D12ResourceHeap::CreateDescriptorHeap(
    ID3D12Device*                   device,
    D3D12_DESCRIPTOR_HEAP_TYPE      heapType,
    UINT                            numDescriptors,
    ComPtr<ID3D12DescriptorHeap>&   outDescritporHeap)
{
    /* Create descriptor heap for resource views or samplers */
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type           = heapType;
        heapDesc.NumDescriptors = numDescriptors;
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask       = 0;
    }
    auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(outDescritporHeap.ReleaseAndGetAddressOf()));
    DXThrowIfCreateFailed(hr, GetContextInfoForFailedDescriptorHeap(heapType));

    /* Store in array for quick access */
    AppendDescriptorHeapToArray(outDescritporHeap.Get());
}

void D3D12ResourceHeap::AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap)
{
    /* Get device and type from descriptor heap */
    ComPtr<ID3D12Device> device;
    descriptorHeap->GetDevice(IID_PPV_ARGS(&device));
    D3D12_DESCRIPTOR_HEAP_TYPE heapType = descriptorHeap->GetDesc().Type;

    /* Append descriptor heap and handle stride to arrays */
    LLGL_ASSERT(numDescriptorHeaps_ < 2);
    descriptorHeaps_[numDescriptorHeaps_] = descriptorHeap;
    descriptorHandleStrides_[numDescriptorHeaps_] = device->GetDescriptorHandleIncrementSize(heapType);
    ++numDescriptorHeaps_;
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
    const D3D12DescriptorHandleLocation&    descriptorHandle,
    std::uint32_t                           descriptorSet,
    Resource&                               resource,
    std::uint32_t                           (&setRange)[2])
{
    if (HasBarriers())
    {
        if (resource.GetResourceType() == ResourceType::Buffer)
        {
            auto& buffer = LLGL_CAST(Buffer&, resource);
            if (IsUAVResourceBarrierRequired(buffer.GetBindFlags()))
            {
                auto& bufferD3D = LLGL_CAST(D3D12Buffer&, buffer);
                EmplaceD3DUAVResource(descriptorHandle, descriptorSet, bufferD3D.GetNative(), setRange);
            }
            else
                EmplaceD3DUAVResource(descriptorHandle, descriptorSet, nullptr, setRange);
        }
        else if (resource.GetResourceType() == ResourceType::Texture)
        {
            auto& texture = LLGL_CAST(Texture&, resource);
            if (IsUAVResourceBarrierRequired(texture.GetBindFlags()))
            {
                auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);
                EmplaceD3DUAVResource(descriptorHandle, descriptorSet, textureD3D.GetNative(), setRange);
            }
            else
                EmplaceD3DUAVResource(descriptorHandle, descriptorSet, nullptr, setRange);
        }
    }
}

void D3D12ResourceHeap::EmplaceD3DUAVResource(
    const D3D12DescriptorHandleLocation&    descriptorHandle,
    std::uint32_t                           descriptorSet,
    ID3D12Resource*                         resource,
    std::uint32_t                           (&setRange)[2])
{
    if (descriptorHandle.index >= uavResourceIndexOffset_)
    {
        auto& cached = uavResourceHeap_[descriptorSet * uavResourceSetStride_ + descriptorHandle.index - uavResourceIndexOffset_];
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
    auto barrierHeapStart = &barriers_[descriptorSet * barrierStride_];
    auto barriers = reinterpret_cast<D3D12_RESOURCE_BARRIER*>(barrierHeapStart + sizeof(UINT));

    /* Write new barriers for entire descriptor set */
    UINT numBarriers = 0;
    for_range(i, uavResourceSetStride_)
    {
        if (auto resource = uavResourceHeap_[descriptorSet * uavResourceSetStride_ + i])
        {
            auto& barrier = barriers[numBarriers++];
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
