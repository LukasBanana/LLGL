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
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include <LLGL/Resource.h>
#include <LLGL/ResourceHeapFlags.h>
#include <functional>


namespace LLGL
{


static const D3D12PipelineLayout* GetD3DPipelineLayout(const ResourceHeapDescriptor& desc)
{
    if (!desc.pipelineLayout)
        throw std::invalid_argument("cannot create resource heap without pipeline layout");
    return LLGL_CAST(const D3D12PipelineLayout*, desc.pipelineLayout);
}

static bool RequiresResourceBarriers(long bindFlags)
{
    return ((bindFlags & BindFlags::Storage) != 0);
}

// Returns true if the specified list of resource views contains at least one resource that requires a UAV resource barrier
static bool RequiresResourceBarriers(const std::vector<ResourceViewDescriptor>& resourceViews)
{
    for (const auto& rvDesc : resourceViews)
    {
        if (const auto* resource = rvDesc.resource)
        {
            switch (resource->GetResourceType())
            {
                case ResourceType::Buffer:
                {
                    if (RequiresResourceBarriers(LLGL_CAST(const Buffer*, resource)->GetBindFlags()))
                        return true;
                }
                break;

                case ResourceType::Texture:
                {
                    if (RequiresResourceBarriers(LLGL_CAST(const Texture*, resource)->GetBindFlags()))
                        return true;
                }
                break;

                default:
                break;
            }
        }
    }
    return false;
}

// Returns the native D3D resource from the descriptor if the resource contains a UAV
static ID3D12Resource* GetD3DResourceWithUAV(const ResourceViewDescriptor& rvDesc)
{
    if (auto resource = rvDesc.resource)
    {
        switch (resource->GetResourceType())
        {
            case ResourceType::Buffer:
            {
                auto bufferD3D = LLGL_CAST(const D3D12Buffer*, resource);
                if (RequiresResourceBarriers(bufferD3D->GetBindFlags()))
                    return bufferD3D->GetNative();
            }
            break;

            case ResourceType::Texture:
            {
                auto textureD3D = LLGL_CAST(const D3D12Texture*, resource);
                if (RequiresResourceBarriers(textureD3D->GetBindFlags()))
                    return textureD3D->GetNative();
            }
            break;

            default:
            break;
        }
    }
    return nullptr;
}

D3D12ResourceHeap::D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc)
{
    /* Create descriptor heaps */
    auto cpuDescHandleCbvSrvUav = CreateHeapTypeCbvSrvUav(device, desc);
    auto cpuDescHandleSampler   = CreateHeapTypeSampler(device, desc);

    /* Store meta data which pipelines will be used by this resource heap */
    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);
    auto combinedStageFlags = pipelineLayoutD3D->GetCombinedStageFlags();

    hasGraphicsDescriptors_ = ((combinedStageFlags & StageFlags::AllGraphicsStages) != 0);
    hasComputeDescriptors_  = ((combinedStageFlags & StageFlags::ComputeStage     ) != 0);

    /* Store descriptor handle strides for the respective number of resources per set */
    const auto descHandleStrideCbvSrvUav = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    const auto descHandleStrideSampler = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    const auto& rootParameterLayout = pipelineLayoutD3D->GetRootParameterLayout();

    descriptorHandleStrides_[0] =
    (
        rootParameterLayout.numBufferCBV  +
        rootParameterLayout.numBufferSRV  +
        rootParameterLayout.numTextureSRV +
        rootParameterLayout.numBufferUAV  +
        rootParameterLayout.numTextureUAV
    ) * descHandleStrideCbvSrvUav;

    descriptorHandleStrides_[1] =
    (
        rootParameterLayout.numSamplers
    ) * descHandleStrideSampler;

    /* Create descriptors */
    std::size_t bindingIndex = 0, firstResourceIndex = 0;
    do
    {
        firstResourceIndex = bindingIndex;
        {
            CreateConstantBufferViews (device, desc, cpuDescHandleCbvSrvUav, bindingIndex, firstResourceIndex, rootParameterLayout);
            CreateShaderResourceViews (device, desc, cpuDescHandleCbvSrvUav, bindingIndex, firstResourceIndex, rootParameterLayout);
            CreateUnorderedAccessViews(device, desc, cpuDescHandleCbvSrvUav, bindingIndex, firstResourceIndex, rootParameterLayout);
            CreateSamplers            (device, desc, cpuDescHandleSampler,   bindingIndex, firstResourceIndex, rootParameterLayout);
        }
        ++numDescriptorSets_;
    }
    while (bindingIndex < desc.resourceViews.size() && firstResourceIndex < bindingIndex);

    /* Check if any ressource barriers are required */
    if (RequiresResourceBarriers(desc.resourceViews))
    {
        /* Append UAV resource barrier for each resource that has the 'BindFlags::Storage' bit */
        const auto numResourceViews = desc.resourceViews.size();
        const auto numBindings      = numResourceViews / numDescriptorSets_;

        for (std::size_t i = 0; i < numResourceViews; i += numBindings)
        {
            const UINT barrierOffset = static_cast<UINT>(barriers_.size());
            {
                for (std::size_t j = 0; j < numBindings; ++j)
                {
                    if (auto resource = GetD3DResourceWithUAV(desc.resourceViews[i + j]))
                        AppendUAVBarrier(resource);
                }
            }
            barrierOffsets_.push_back(barrierOffset);
        }

        /* Append end-offset for resource barriers (if they are used) */
        barrierOffsets_.push_back(static_cast<UINT>(barriers_.size()));
    }
}

void D3D12ResourceHeap::InsertResourceBarriers(ID3D12GraphicsCommandList* commandList, UINT firstSet)
{
    if (!barriers_.empty())
    {
        /* Insert all resource barriers that are required for the specified descriptor set */
        const UINT numBarriers = GetBarrierCount(firstSet);
        if (numBarriers > 0)
            commandList->ResourceBarrier(numBarriers, &(barriers_[GetBarrierOffset(firstSet)]));
    }
}

void D3D12ResourceHeap::SetName(const char* name)
{
    D3D12SetObjectNameSubscript(heapTypeCbvSrvUav_.Get(), name, ".CbvSrvUav");
    D3D12SetObjectNameSubscript(heapTypeSampler_.Get(), name, ".Sampler");
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

using D3DResourceViewFunc = std::function<bool(Resource& resource, const ResourceViewDescriptor& rvDesc)>;

static void ForEachResourceViewOfType(
    const ResourceHeapDescriptor&   desc,
    const ResourceType              resourceType,
    std::size_t                     firstIndex,
    UINT                            numResourceViewsInLayout,
    const D3DResourceViewFunc&      callback)
{
    for (auto i = firstIndex; i < desc.resourceViews.size() && numResourceViewsInLayout > 0; ++i)
    {
        const auto& resourceView = desc.resourceViews[i];
        if (auto resource = resourceView.resource)
        {
            if (resource->GetResourceType() == resourceType)
            {
                if (callback(*resource, resourceView))
                    --numResourceViewsInLayout;
            }
        }
    }
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
    std::size_t&                    bindingIndex,
    std::size_t                     firstResourceIndex,
    const D3D12RootParameterLayout& rootParameterLayout)
{
    UINT descHandleStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        firstResourceIndex,
        rootParameterLayout.numBufferCBV,
        [&](Resource& resource, const ResourceViewDescriptor& rvDesc) -> bool
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::ConstantBuffer, bindingIndex))
            {
                if (IsBufferViewEnabled(rvDesc.bufferView))
                    bufferD3D.CreateConstantBufferView(device, cpuDescHandle, rvDesc.bufferView);
                else
                    bufferD3D.CreateConstantBufferView(device, cpuDescHandle);
                cpuDescHandle.ptr += descHandleStride;
                return true;
            }
            return false;
        }
    );
}

void D3D12ResourceHeap::CreateShaderResourceViews(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex,
    std::size_t                     firstResourceIndex,
    const D3D12RootParameterLayout& rootParameterLayout)
{
    const UINT descHandleStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    /* First create SRVs for all sampled-buffers; it needs to be in the same order as the root parameters are build in <D3D12PipelineLayout> */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        firstResourceIndex,
        rootParameterLayout.numBufferSRV,
        [&](Resource& resource, const ResourceViewDescriptor& rvDesc) -> bool
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::Sampled, bindingIndex))
            {
                if (IsBufferViewEnabled(rvDesc.bufferView))
                    bufferD3D.CreateShaderResourceView(device, cpuDescHandle, rvDesc.bufferView);
                else
                    bufferD3D.CreateShaderResourceView(device, cpuDescHandle);
                cpuDescHandle.ptr += descHandleStride;
                return true;
            }
            return false;
        }
    );

    /* Now create SRVs for all sampled-textures */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Texture,
        firstResourceIndex,
        rootParameterLayout.numTextureSRV,
        [&](Resource& resource, const ResourceViewDescriptor& rvDesc) -> bool
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, textureD3D.GetBindFlags(), BindFlags::Sampled, bindingIndex))
            {
                if (IsTextureViewEnabled(rvDesc.textureView))
                    textureD3D.CreateShaderResourceView(device, cpuDescHandle, rvDesc.textureView);
                else
                    textureD3D.CreateShaderResourceView(device, cpuDescHandle);
                cpuDescHandle.ptr += descHandleStride;
                return true;
            }
            return false;
        }
    );
}

void D3D12ResourceHeap::CreateUnorderedAccessViews(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex,
    std::size_t                     firstResourceIndex,
    const D3D12RootParameterLayout& rootParameterLayout)
{
    const UINT descHandleStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    auto pipelineLayoutD3D = GetD3DPipelineLayout(desc);

    /* First create UAVs for all RW-buffers; it needs to be in the same order as the root parameters are build in <D3D12PipelineLayout> */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Buffer,
        firstResourceIndex,
        rootParameterLayout.numBufferUAV,
        [&](Resource& resource, const ResourceViewDescriptor& rvDesc) -> bool
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, bufferD3D.GetBindFlags(), BindFlags::Storage, bindingIndex))
            {
                if (IsBufferViewEnabled(rvDesc.bufferView))
                    bufferD3D.CreateUnorderedAccessView(device, cpuDescHandle, rvDesc.bufferView);
                else
                    bufferD3D.CreateUnorderedAccessView(device, cpuDescHandle);
                cpuDescHandle.ptr += descHandleStride;
                return true;
            }
            return false;
        }
    );

    /* Now create UAVs for all RW-textures */
    ForEachResourceViewOfType(
        desc,
        ResourceType::Texture,
        firstResourceIndex,
        rootParameterLayout.numTextureUAV,
        [&](Resource& resource, const ResourceViewDescriptor& rvDesc) -> bool
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            if (MatchBindFlags(*pipelineLayoutD3D, textureD3D.GetBindFlags(), BindFlags::Storage, bindingIndex))
            {
                if (IsTextureViewEnabled(rvDesc.textureView))
                    textureD3D.CreateUnorderedAccessView(device, cpuDescHandle, rvDesc.textureView);
                else
                    textureD3D.CreateUnorderedAccessView(device, cpuDescHandle);
                cpuDescHandle.ptr += descHandleStride;
                return true;
            }
            return false;
        }
    );
}

void D3D12ResourceHeap::CreateSamplers(
    ID3D12Device*                   device,
    const ResourceHeapDescriptor&   desc,
    D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
    std::size_t&                    bindingIndex,
    std::size_t                     firstResourceIndex,
    const D3D12RootParameterLayout& rootParameterLayout)
{
    const UINT descHandleStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    ForEachResourceViewOfType(
        desc,
        ResourceType::Sampler,
        firstResourceIndex,
        rootParameterLayout.numSamplers,
        [&](Resource& resource, const ResourceViewDescriptor& /*rvDesc*/) -> bool
        {
            auto& samplerD3D = LLGL_CAST(D3D12Sampler&, resource);
            samplerD3D.CreateResourceView(device, cpuDescHandle);
            cpuDescHandle.ptr += descHandleStride;
            ++bindingIndex;
            return true;
        }
    );
}

void D3D12ResourceHeap::AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap)
{
    descriptorHeaps_[numDescriptorHeaps_++] = descriptorHeap;
}

void D3D12ResourceHeap::AppendUAVBarrier(ID3D12Resource* resource)
{
    D3D12_RESOURCE_BARRIER barrier;
    {
        barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.UAV.pResource   = resource;
    }
    barriers_.push_back(barrier);
}

UINT D3D12ResourceHeap::GetBarrierOffset(UINT firstSet) const
{
    return barrierOffsets_[firstSet];
}

UINT D3D12ResourceHeap::GetBarrierCount(UINT firstSet) const
{
    return (barrierOffsets_[firstSet + 1] - barrierOffsets_[firstSet]);
}


} // /namespace LLGL



// ================================================================================
