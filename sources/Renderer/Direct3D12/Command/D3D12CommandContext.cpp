/*
 * D3D12CommandContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12CommandContext.h"
#include "D3D12CommandQueue.h"
#include "../D3D12Device.h"
#include "../D3D12Resource.h"
#include "../Buffer/D3D12Buffer.h"
#include "../Texture/D3D12Texture.h"
#include "../RenderState/D3D12Fence.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <limits.h>


// Validates resource descriptors for each transition barrier. Potentially slow, use with caution!
#define LLGL_DEBUG_D3D12_RESOURCE_BARRIERS 0


namespace LLGL
{


static constexpr D3D12_DESCRIPTOR_HEAP_TYPE g_descriptorHeapTypes[] =
{
    D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
};

constexpr UINT D3D12CommandContext::maxNumAllocators;
constexpr UINT D3D12CommandContext::maxNumResourceBarrieres;
constexpr UINT D3D12CommandContext::maxNumDescriptorHeaps;

D3D12CommandContext::D3D12CommandContext()
{
    ClearCache();
}

D3D12CommandContext::D3D12CommandContext(D3D12Device& device)
{
    Create(device);
}

void D3D12CommandContext::Create(
    D3D12Device&            device,
    D3D12_COMMAND_LIST_TYPE commandListType,
    UINT                    numAllocators,
    UINT64                  initialStagingChunkSize,
    bool                    initialClose,
    bool                    cacheResourceStates)
{
    doCacheResourceStates_ = cacheResourceStates;

    /* Store reference to device and command queue */
    device_ = device.GetNative();

    /* Create fence for command allocators */
    allocatorFence_.Create(device.GetNative());

    /* Determine number of command allocators */
    numAllocators_ = Clamp<UINT>(numAllocators, 1u, D3D12CommandContext::maxNumAllocators);

    /* Create command allocators and descriptor heap pools */
    constexpr UINT64 minStagingChunkSize = 256;
    initialStagingChunkSize = std::max(minStagingChunkSize, initialStagingChunkSize);

    for_range(i, numAllocators_)
    {
        commandAllocators_[i] = device.CreateDXCommandAllocator(commandListType);
        for_range(j, D3D12CommandContext::maxNumDescriptorHeaps)
            stagingDescriptorPools_[i][j].InitializeDevice(device.GetNative(), g_descriptorHeapTypes[j]);
        descriptorCaches_[i].Create(device.GetNative());
        stagingBufferPools_[i].InitializeDevice(device.GetNative(), initialStagingChunkSize);
        intermediateBufferPools_[i].InitializeDevice(device.GetNative());
    }

    /* Create graphics command list and close it (they are created in recording mode) */
    commandList_ = device.CreateDXCommandList(commandListType, GetCommandAllocator());

    if (initialClose)
        commandList_->Close();

    /* Clear cache alongside device object initialization */
    ClearCache();
}

void D3D12CommandContext::Close()
{
    /* Flush pending resource barriers */
    FlushResourceBarriers();

    /* Close native command list */
    HRESULT hr = commandList_->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");

    /* Reset resource states if this context had to cache them */
    if (doCacheResourceStates_)
    {
        for (const D3D12ResourceTransitionExt& resourceState : cachedResourceStates_)
            resourceState.resource->currentState = resourceState.initialState;
    }
}

void D3D12CommandContext::Signal(D3D12CommandQueue& commandQueue)
{
    const UINT64 currentFenceValue = allocatorFence_.GetCompletedValue();
    const UINT64 nextFenceValue = allocatorFenceValues_[currentAllocatorIndex_];
    if (currentFenceValue < nextFenceValue)
        commandQueue.SignalFence(allocatorFence_.Get(), nextFenceValue);
    allocatorFenceValueDirty_[currentAllocatorIndex_] = false;
}

void D3D12CommandContext::Reset(D3D12CommandQueue& commandQueue)
{
    /* Switch to next command allocator */
    NextCommandAllocator(commandQueue);

    /* Reset graphics command list */
    HRESULT hr = commandList_->Reset(GetCommandAllocator(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");

    /* Invalidate state cache */
    ClearCache();
}

void D3D12CommandContext::ExecuteBundle(D3D12CommandContext& otherContext)
{
    /*
    TODO:
    D3D12 bundles can bind descriptor heaps but they must match the primary command buffer's descriptor heaps.
    As a workaround, always bind the descriptor heaps that were cached in the secondary command buffer,
    since those that are shader visible will be the same throughout the command encoding (see D3D12StagingDescriptorHeapPool).
    Some kind of descriptor heap sharing/pooling should be implemented next.
    */
    SetDescriptorHeapsOfOtherContext(otherContext);

    /* Encode command to execute command list of other context as bundle */
    commandList_->ExecuteBundle(otherContext.GetCommandList());
}

void D3D12CommandContext::ExecuteResourceTransitions(const D3D12CommandContext& otherContext)
{
    for (const D3D12ResourceTransitionExt& resourceState : otherContext.cachedResourceStates_)
    {
        /* Transition resource into the state that's expected at the beginning of the command list */
        TransitionResource(*resourceState.resource, resourceState.beginState);

        /* Store state the resource will be at the end of the command list now, since this won't effect the command list execution */
        resourceState.resource->currentState = resourceState.endState;
    }
}

void D3D12CommandContext::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_STATES oldState, UINT subresource, bool flushImmediate)
{
    #if LLGL_DEBUG_D3D12_RESOURCE_BARRIERS
    if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        D3D12_RESOURCE_DESC descD3D = resource->GetDesc();
        LLGL_ASSERT((descD3D.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0);
    }
    #endif // /LLGL_DEBUG_D3D12_RESOURCE_BARRIERS

    /* Check if there's already a transition barrier for this subresource in the queue */
    if (D3D12_RESOURCE_BARRIER* cachedBarrier = FindSubresourceTransitionBarrier(resource, subresource))
    {
        if (cachedBarrier->Transition.StateBefore == newState)
        {
            /* Drop this barrier if before and after state are the same. Otherwise, ID3D12CommandList will fail! */
            --numResourceBarriers_;
        }
        else
        {
            /* Update after state */
            cachedBarrier->Transition.StateAfter = newState;
        }
    }
    else
    {
        /* Initialize resource barrier for resource transition */
        D3D12_RESOURCE_BARRIER& barrier = NextResourceBarrier();

        barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource    = resource;
        barrier.Transition.Subresource  = subresource;
        barrier.Transition.StateBefore  = oldState;
        barrier.Transition.StateAfter   = newState;
    }

    /* Flush resource barrieres if required */
    if (flushImmediate)
        FlushResourceBarriers();
}

void D3D12CommandContext::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_STATES oldState, bool flushImmediate)
{
    TransitionBarrier(resource, newState, oldState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, flushImmediate);
}

void D3D12CommandContext::TransitionResource(D3D12Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
{
    if (doCacheResourceStates_)
    {
        /* Cache resource state at beginning and end of command list */
        bool isBeginState = false;
        CacheResourceState(&resource, newState, isBeginState);

        if (isBeginState)
        {
            /* Store new resource state */
            resource.currentState = newState;
        }
        else
        {
            /*
            Only transition resource now if it was not the initial cache entry.
            Otherwise, the command list expects the resource to be in this state at the beginning.
            */
            TransitionResourceInternal(resource, newState);
        }
    }
    else
    {
        /* Transition resource to new state if it has changed */
        TransitionResourceInternal(resource, newState);
    }

    /* Flush resource barrieres if required */
    if (flushImmediate)
        FlushResourceBarriers();
}

//private
void D3D12CommandContext::TransitionResourceInternal(D3D12Resource& resource, D3D12_RESOURCE_STATES newState)
{
    if (resource.currentState != newState)
    {
        #if LLGL_DEBUG_D3D12_RESOURCE_BARRIERS
        if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            D3D12_RESOURCE_DESC descD3D = resource.native->GetDesc();
            LLGL_ASSERT((descD3D.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0);
        }
        #endif // /LLGL_DEBUG_D3D12_RESOURCE_BARRIERS

        /* Check if there's already a transition barrier for this subresource in the queue */
        if (D3D12_RESOURCE_BARRIER* cachedBarrier = FindSubresourceTransitionBarrier(resource.Get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES))
        {
            if (cachedBarrier->Transition.StateBefore == newState)
            {
                /* Drop this barrier if before and after state are the same. Otherwise, ID3D12CommandList will fail! */
                --numResourceBarriers_;
            }
            else
            {
                /* Update after state */
                cachedBarrier->Transition.StateAfter = newState;
            }
        }
        else
        {
            D3D12_RESOURCE_BARRIER& barrier = NextResourceBarrier();

            /* Initialize resource barrier for resource transition */
            barrier.Type                    = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags                   = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource    = resource.Get();
            barrier.Transition.Subresource  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore  = resource.currentState;
            barrier.Transition.StateAfter   = newState;
        }

        /* Store new resource state */
        resource.currentState = newState;
    }
}

void D3D12CommandContext::UAVBarrier(ID3D12Resource* resource, bool flushImmediate)
{
    D3D12_RESOURCE_BARRIER& barrier = NextResourceBarrier();

    barrier.Type            = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    barrier.Flags           = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.UAV.pResource   = resource;

    if (flushImmediate)
        FlushResourceBarriers();
}

void D3D12CommandContext::FlushResourceBarriers()
{
    if (numResourceBarriers_ > 0)
    {
        if (numUAVBarriers_ > 0)
        {
            /*
            Merge UAV and transition barriers if possible since ResourceBarrier() command is expensive; From the D3D12 API docu:
            "Transitions should be batched together into a single API call when possible, as a performance optimization."
            */
            if (numResourceBarriers_ + numUAVBarriers_ <= maxNumResourceBarrieres)
            {
                ::memcpy(&(resourceBarriers_[numResourceBarriers_]), uavBarriers_.data(), numUAVBarriers_ * sizeof(D3D12_RESOURCE_BARRIER));
                commandList_->ResourceBarrier(numResourceBarriers_ + numUAVBarriers_, resourceBarriers_);
            }
            else
            {
                uavBarriers_.resize(numResourceBarriers_ + numUAVBarriers_);
                ::memcpy(&(uavBarriers_[numUAVBarriers_]), resourceBarriers_, numResourceBarriers_ * sizeof(D3D12_RESOURCE_BARRIER));
                commandList_->ResourceBarrier(numResourceBarriers_ + numUAVBarriers_, uavBarriers_.data());
            }
        }
        else
            commandList_->ResourceBarrier(numResourceBarriers_, resourceBarriers_);

        /* Reset intermediate barriers */
        numResourceBarriers_ = 0;
    }
    else
    {
        /* Submit UAV barriers */
        if (numUAVBarriers_ > 0)
            commandList_->ResourceBarrier(numUAVBarriers_, uavBarriers_.data());
    }
}

void D3D12CommandContext::ResolveSubresource(
    D3D12Resource&  dstResource,
    UINT            dstSubresource,
    D3D12Resource&  srcResource,
    UINT            srcSubresource,
    DXGI_FORMAT     format)
{
    /* Transition both resources */
    const D3D12_RESOURCE_STATES dstResourceOldState = dstResource.currentState;
    const D3D12_RESOURCE_STATES srcResourceOldState = srcResource.currentState;

    TransitionResource(dstResource, D3D12_RESOURCE_STATE_RESOLVE_DEST);
    TransitionResource(srcResource, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, true);

    /* Resolve multi-sampled render targets */
    commandList_->ResolveSubresource(
        dstResource.native.Get(),
        dstSubresource,
        srcResource.native.Get(),
        srcSubresource,
        format
    );

    /* Transition both resources */
    TransitionResource(dstResource, dstResourceOldState);
    TransitionResource(srcResource, srcResourceOldState);
}

void D3D12CommandContext::CopyTextureRegion(
    D3D12Resource&      dstResource,
    UINT                dstSubresource,
    UINT                dstX,
    UINT                dstY,
    UINT                dstZ,
    D3D12Resource&      srcResource,
    UINT                srcSubresource,
    const D3D12_BOX*    srcBox)
{
    /* Transition both resources */
    const D3D12_RESOURCE_STATES dstResourceOldState = dstResource.currentState;
    const D3D12_RESOURCE_STATES srcResourceOldState = srcResource.currentState;

    TransitionResource(dstResource, D3D12_RESOURCE_STATE_COPY_DEST);
    TransitionResource(srcResource, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

    /* Copy texture region subresources */
    D3D12_TEXTURE_COPY_LOCATION dstLocation;
    {
        dstLocation.pResource           = dstResource.Get();
        dstLocation.Type                = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex    = dstSubresource;
    }
    D3D12_TEXTURE_COPY_LOCATION srcLocation;
    {
        srcLocation.pResource           = srcResource.Get();
        srcLocation.Type                = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex    = srcSubresource;
    }
    commandList_->CopyTextureRegion(&dstLocation, dstX, dstY, dstZ, &srcLocation, srcBox);

    /* Transition both resources */
    TransitionResource(dstResource, dstResourceOldState);
    TransitionResource(srcResource, srcResourceOldState);
}

void D3D12CommandContext::UpdateSubresource(
    D3D12Resource&  dstResource,
    UINT64          dstOffset,
    const void*     data,
    UINT64          dataSize)
{
    stagingBufferPools_[currentAllocatorIndex_].WriteStaged(*this, dstResource, dstOffset, data, dataSize);
}

ID3D12Resource* D3D12CommandContext::AllocIntermediateBuffer(UINT64 size, UINT alignment)
{
    return intermediateBufferPools_[currentAllocatorIndex_].AllocBuffer(size, alignment);
}

void D3D12CommandContext::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature)
{
    if (stateCache_.dirtyBits.graphicsRootSignature != 0 || stateCache_.graphicsRootSignature != rootSignature)
    {
        /* Bind graphics root signature and cache state */
        commandList_->SetGraphicsRootSignature(rootSignature);
        stateCache_.graphicsRootSignature           = rootSignature;
        stateCache_.dirtyBits.graphicsRootSignature = 0;
    }
}

void D3D12CommandContext::SetComputeRootSignature(ID3D12RootSignature* rootSignature)
{
    if (stateCache_.dirtyBits.computeRootSignature != 0 || stateCache_.computeRootSignature != rootSignature)
    {
        /* Bind graphics root signature and cache state */
        commandList_->SetComputeRootSignature(rootSignature);
        stateCache_.computeRootSignature            = rootSignature;
        stateCache_.dirtyBits.computeRootSignature  = 0;
    }
}

void D3D12CommandContext::SetPipelineState(ID3D12PipelineState* pipelineState)
{
    stateCache_.stateBits.isDeferredPSO = 0;
    SetPipelineStateCached(pipelineState);
}

void D3D12CommandContext::SetDeferredPipelineState(ID3D12PipelineState* pipelineStateUI16, ID3D12PipelineState* pipelineStateUI32)
{
    stateCache_.stateBits.isDeferredPSO = 1;
    stateCache_.deferredPipelineStates[0] = pipelineStateUI16;
    stateCache_.deferredPipelineStates[1] = pipelineStateUI32;
}

static bool CompareDescriptorHeapRefs(
    UINT                            lhsNumDescriptorHeaps,
    ID3D12DescriptorHeap* const*    lhsDescriptorHeaps,
    UINT                            rhsNumDescriptorHeaps,
    ID3D12DescriptorHeap* const*    rhsDescriptorHeaps)
{
    /* Compare all descriptor heaps in the arrays */
    return
    (
        lhsNumDescriptorHeaps == rhsNumDescriptorHeaps &&
        (::memcmp(lhsDescriptorHeaps, rhsDescriptorHeaps, sizeof(ID3D12DescriptorHeap* const) * lhsNumDescriptorHeaps) == 0)
    );
}

void D3D12CommandContext::SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* const* descriptorHeaps)
{
    if (numDescriptorHeaps <= D3D12CommandContext::maxNumDescriptorHeaps)
    {
        /* Check if the descriptor heaps are cached */
        if (stateCache_.dirtyBits.descriptorHeaps != 0 ||
            !CompareDescriptorHeapRefs(numDescriptorHeaps, descriptorHeaps, stateCache_.numDescriptorHeaps, stateCache_.descriptorHeaps))
        {
            /* Set new descriptor heaps in D3D command list */
            commandList_->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);

            /* Store new descriptor heaps in state cache */
            stateCache_.numDescriptorHeaps = numDescriptorHeaps;
            ::memcpy(stateCache_.descriptorHeaps, descriptorHeaps, sizeof(ID3D12DescriptorHeap* const) * numDescriptorHeaps);
            stateCache_.dirtyBits.descriptorHeaps = 0;
        }
    }
}

void D3D12CommandContext::SetDescriptorHeapsOfOtherContext(const D3D12CommandContext& other)
{
    if (other.stateCache_.numDescriptorHeaps > 0)
        SetDescriptorHeaps(other.stateCache_.numDescriptorHeaps, other.stateCache_.descriptorHeaps);
}

void D3D12CommandContext::SetStagingDescriptorHeaps(const D3D12DescriptorHeapSetLayout& layout, const D3D12RootParameterIndices& indices)
{
    stagingDescriptorSetLayout_ = layout;
    stagingDescriptorIndices_   = indices;

    if (stagingDescriptorSetLayout_.numHeapResourceViews > 0 ||
        stagingDescriptorSetLayout_.numHeapSamplers      > 0 ||
        stagingDescriptorSetLayout_.numResourceViews     > 0 ||
        stagingDescriptorSetLayout_.numSamplers          > 0)
    {
        /* Bind shader-visible descriptor heaps */
        ID3D12DescriptorHeap* const stagingDescriptorHeaps[2] =
        {
            stagingDescriptorPools_[currentAllocatorIndex_][0].GetDescriptorHeap(),
            stagingDescriptorPools_[currentAllocatorIndex_][1].GetDescriptorHeap()
        };
        SetDescriptorHeaps(2, stagingDescriptorHeaps);

        /* Reset descriptor cache for dynamic descriptors */
        descriptorCaches_[currentAllocatorIndex_].Reset(
            stagingDescriptorSetLayout_.numResourceViews,
            stagingDescriptorSetLayout_.numSamplers
        );
    }
}

void D3D12CommandContext::GetStagingDescriptorHeaps(D3D12DescriptorHeapSetLayout& outLayout, D3D12RootParameterIndices& outIndices)
{
    outLayout   = stagingDescriptorSetLayout_;
    outIndices  = stagingDescriptorIndices_;
}

void D3D12CommandContext::SetGraphicsConstant(UINT parameterIndex, D3D12Constant value, UINT offset)
{
    commandList_->SetGraphicsRoot32BitConstant(parameterIndex, value.bits32, offset);
}

void D3D12CommandContext::SetComputeConstant(UINT parameterIndex, D3D12Constant value, UINT offset)
{
    commandList_->SetComputeRoot32BitConstant(parameterIndex, value.bits32, offset);
}

void D3D12CommandContext::SetGraphicsRootParameter(UINT parameterIndex, D3D12_ROOT_PARAMETER_TYPE parameterType, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddr)
{
    switch (parameterType)
    {
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            commandList_->SetGraphicsRootConstantBufferView(parameterIndex, gpuVirtualAddr);
            break;

        case D3D12_ROOT_PARAMETER_TYPE_SRV:
            commandList_->SetGraphicsRootShaderResourceView(parameterIndex, gpuVirtualAddr);
            break;

        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            commandList_->SetGraphicsRootUnorderedAccessView(parameterIndex, gpuVirtualAddr);
            break;
    }
}

void D3D12CommandContext::SetComputeRootParameter(UINT parameterIndex, D3D12_ROOT_PARAMETER_TYPE parameterType, D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddr)
{
    switch (parameterType)
    {
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            commandList_->SetComputeRootConstantBufferView(parameterIndex, gpuVirtualAddr);
            break;

        case D3D12_ROOT_PARAMETER_TYPE_SRV:
            commandList_->SetComputeRootShaderResourceView(parameterIndex, gpuVirtualAddr);
            break;

        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            commandList_->SetComputeRootUnorderedAccessView(parameterIndex, gpuVirtualAddr);
            break;
    }
}

void D3D12CommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& indexBufferView)
{
    commandList_->IASetIndexBuffer(&indexBufferView);
    stateCache_.stateBits.is16BitIndexFormat = (indexBufferView.Format == DXGI_FORMAT_R16_UINT ? 1 : 0);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12CommandContext::GetCPUDescriptorHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT descriptor) const
{
    /* Get current descriptor heap pool via allocator- and type index */
    const UINT typeIndex = static_cast<UINT>(type);
    LLGL_ASSERT(typeIndex < D3D12CommandContext::maxNumDescriptorHeaps);
    const D3D12StagingDescriptorHeapPool& descriptorHeapPool = stagingDescriptorPools_[currentAllocatorIndex_][typeIndex];

    /* Return CPU descriptor handle for the specified descriptor in the pool */
    return descriptorHeapPool.GetCpuHandleWithOffset(descriptor);
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12CommandContext::CopyDescriptorsForStaging(
    D3D12_DESCRIPTOR_HEAP_TYPE  type,
    D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
    UINT                        firstDescriptor,
    UINT                        numDescriptors)
{
    /* Get current descriptor heap pool via allocator- and type index */
    const UINT typeIndex = static_cast<UINT>(type);
    LLGL_ASSERT(typeIndex < D3D12CommandContext::maxNumDescriptorHeaps);
    D3D12StagingDescriptorHeapPool& descriptorHeapPool = stagingDescriptorPools_[currentAllocatorIndex_][typeIndex];

    /* Copy descriptors into shader-visible descriptor heap */
    return descriptorHeapPool.CopyDescriptors(srcDescHandle, firstDescriptor, numDescriptors);
}

void D3D12CommandContext::EmplaceDescriptorForStaging(Resource& resource, const D3D12DescriptorHeapLocation& descriptorLocation)
{
    descriptorCaches_[currentAllocatorIndex_].EmplaceDescriptor(resource, descriptorLocation.descriptorIndex, descriptorLocation.type);
}

void D3D12CommandContext::ResetUAVBarriers(UINT numUAVBarriers)
{
    if (uavBarriers_.size() < numUAVBarriers)
    {
        D3D12_RESOURCE_BARRIER initialBarrier;
        {
            initialBarrier.Type             = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            initialBarrier.Flags            = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            initialBarrier.UAV.pResource    = nullptr;
        }
        uavBarriers_.resize(numUAVBarriers, initialBarrier);
    }
    numUAVBarriers_ = numUAVBarriers;
}

void D3D12CommandContext::SetResourceUAVBarrier(ID3D12Resource* resource, UINT uavBarrierSlot)
{
    uavBarriers_[uavBarrierSlot].UAV.pResource = resource;
}

void D3D12CommandContext::SetResourceUAVBarrier(Resource& resource, const D3D12DescriptorHeapLocation& descriptorLocation)
{
    if (descriptorLocation.uavBarrierIndex < numUAVBarriers_)
    {
        const ResourceType resourceType = resource.GetResourceType();
        if (resourceType == ResourceType::Buffer)
        {
            auto& bufferD3D = LLGL_CAST(D3D12Buffer&, resource);
            SetResourceUAVBarrier(bufferD3D.GetNative(), descriptorLocation.uavBarrierIndex);
        }
        else if (resourceType == ResourceType::Texture)
        {
            auto& textureD3D = LLGL_CAST(D3D12Texture&, resource);
            SetResourceUAVBarrier(textureD3D.GetNative(), descriptorLocation.uavBarrierIndex);
        }
    }
}

void D3D12CommandContext::DrawInstanced(
    UINT vertexCountPerInstance,
    UINT instanceCount,
    UINT startVertexLocation,
    UINT startInstanceLocation)
{
    FlushResourceBarriers();
    FlushDeferredPipelineState();
    FlushGraphicsStagingDescriptorTables();
    commandList_->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void D3D12CommandContext::DrawIndexedInstanced(
    UINT    indexCountPerInstance,
    UINT    instanceCount,
    UINT    startIndexLocation,
    INT     baseVertexLocation,
    UINT    startInstanceLocation)
{
    FlushResourceBarriers();
    FlushDeferredPipelineState();
    FlushGraphicsStagingDescriptorTables();
    commandList_->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void D3D12CommandContext::DrawIndirect(
    ID3D12CommandSignature* commandSignature,
    UINT                    maxCommandCount,
    ID3D12Resource*         argumentBuffer,
    UINT64                  argumentBufferOffset,
    ID3D12Resource*         countBuffer,
    UINT64                  countBufferOffset)
{
    FlushResourceBarriers();
    FlushDeferredPipelineState();
    FlushGraphicsStagingDescriptorTables();
    commandList_->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

void D3D12CommandContext::Dispatch(
    UINT threadGroupCountX,
    UINT threadGroupCountY,
    UINT threadGroupCountZ)
{
    FlushResourceBarriers();
    FlushComputeStagingDescriptorTables();
    commandList_->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void D3D12CommandContext::DispatchIndirect(
    ID3D12CommandSignature* commandSignature,
    UINT                    maxCommandCount,
    ID3D12Resource*         argumentBuffer,
    UINT64                  argumentBufferOffset,
    ID3D12Resource*         countBuffer,
    UINT64                  countBufferOffset)
{
    FlushResourceBarriers();
    FlushComputeStagingDescriptorTables();
    commandList_->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}


/*
 * ======= Private: =======
 */

void D3D12CommandContext::ClearCache()
{
    /* Invalidate dirty bits */
    stateCache_.dirtyBits.pipelineState         = 1;
    stateCache_.dirtyBits.graphicsRootSignature = 1;
    stateCache_.dirtyBits.computeRootSignature  = 1;
    stateCache_.dirtyBits.descriptorHeaps       = 1;

    /* Clear state bits */
    stateCache_.stateBits.isDeferredPSO         = 0;
    stateCache_.stateBits.is16BitIndexFormat    = 0;

    /* Clear cached resource states */
    cachedResourceStates_.clear();

    numUAVBarriers_ = 0;
}

D3D12_RESOURCE_BARRIER& D3D12CommandContext::NextResourceBarrier()
{
    if (numResourceBarriers_ == D3D12CommandContext::maxNumResourceBarrieres)
        FlushResourceBarriers();
    return resourceBarriers_[numResourceBarriers_++];
}

D3D12_RESOURCE_BARRIER* D3D12CommandContext::FindSubresourceTransitionBarrier(ID3D12Resource* resource, UINT subresource)
{
    /* Check if last barrier refers to the same subresource */
    if (numResourceBarriers_ > 0)
    {
        D3D12_RESOURCE_BARRIER& barrier = resourceBarriers_[numResourceBarriers_ - 1];
        if (barrier.Type                   == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
            barrier.Transition.pResource   == resource                               &&
            barrier.Transition.Subresource == subresource)
        {
            return &barrier;
        }
    }
    return nullptr;
}

void D3D12CommandContext::CacheResourceState(D3D12Resource* resource, D3D12_RESOURCE_STATES state, bool& outIsBeginState)
{
    if (resource->cacheIndex < cachedResourceStates_.size() && cachedResourceStates_[resource->cacheIndex].resource == resource)
    {
        /* Update previous cache entry */
        cachedResourceStates_[resource->cacheIndex].endState = state;
    }
    else
    {
        /* Append new cache entry and initialize both begin and end states */
        resource->cacheIndex = static_cast<UINT>(cachedResourceStates_.size());
        cachedResourceStates_.push_back({ resource, resource->currentState, state, state });
        outIsBeginState = true;
    }
}

void D3D12CommandContext::NextCommandAllocator(D3D12CommandQueue& commandQueue)
{
    /* Get next command allocator */
    const UINT64 currentFenceValue = allocatorFenceValues_[currentAllocatorIndex_];
    currentAllocatorIndex_ = ((currentAllocatorIndex_ + 1) % numAllocators_);

    /* If fence was not signaled since last encoding, we must signal it now and wait for a full queue flush */
    if (allocatorFenceValueDirty_[currentAllocatorIndex_])
        Signal(commandQueue);

    /* Wait until fence value of next allocator has been signaled */
    allocatorFence_.WaitForHigherSignal(allocatorFenceValues_[currentAllocatorIndex_]);
    allocatorFenceValues_[currentAllocatorIndex_] = currentFenceValue + 1;
    allocatorFenceValueDirty_[currentAllocatorIndex_] = true;

    /* Reclaim memory allocated by command allocator using <ID3D12CommandAllocator::Reset> */
    HRESULT hr = GetCommandAllocator()->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    /* Reset descriptor heap pools before they are re-used */
    for_range(i, D3D12CommandContext::maxNumDescriptorHeaps)
        stagingDescriptorPools_[currentAllocatorIndex_][i].Reset();

    /* Clear descriptor cache and reset staging buffer pool */
    descriptorCaches_[currentAllocatorIndex_].Clear();
    stagingBufferPools_[currentAllocatorIndex_].Reset();
    intermediateBufferPools_[currentAllocatorIndex_].Reset();
}

void D3D12CommandContext::SetPipelineStateCached(ID3D12PipelineState* pipelineState)
{
    if (stateCache_.dirtyBits.pipelineState != 0 || stateCache_.pipelineState != pipelineState)
    {
        /* Bind pipeline state to command list and cache state */
        commandList_->SetPipelineState(pipelineState);
        stateCache_.pipelineState           = pipelineState;
        stateCache_.dirtyBits.pipelineState = 0;
    }
}

void D3D12CommandContext::FlushDeferredPipelineState()
{
    if (stateCache_.stateBits.isDeferredPSO != 0)
        SetPipelineStateCached(stateCache_.deferredPipelineStates[stateCache_.stateBits.is16BitIndexFormat]);
}

void D3D12CommandContext::FlushGraphicsStagingDescriptorTables()
{
    D3D12DescriptorCache& descriptorCache = descriptorCaches_[currentAllocatorIndex_];
    if (descriptorCache.IsInvalidated())
    {
        auto& currentPoolSet = stagingDescriptorPools_[currentAllocatorIndex_];
        if (stagingDescriptorSetLayout_.numResourceViews > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorCache.FlushCbvSrvUavDescriptors(currentPoolSet[0]);
            if (gpuDescHandle.ptr != 0)
                commandList_->SetGraphicsRootDescriptorTable(stagingDescriptorIndices_.rootParamDescriptors[0], gpuDescHandle);
        }
        if (stagingDescriptorSetLayout_.numSamplers > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorCache.FlushSamplerDescriptors(currentPoolSet[1]);
            if (gpuDescHandle.ptr != 0)
                commandList_->SetGraphicsRootDescriptorTable(stagingDescriptorIndices_.rootParamDescriptors[1], gpuDescHandle);
        }
    }
}

void D3D12CommandContext::FlushComputeStagingDescriptorTables()
{
    D3D12DescriptorCache& descriptorCache = descriptorCaches_[currentAllocatorIndex_];
    if (descriptorCache.IsInvalidated())
    {
        auto& currentPoolSet = stagingDescriptorPools_[currentAllocatorIndex_];
        if (stagingDescriptorSetLayout_.numResourceViews > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorCache.FlushCbvSrvUavDescriptors(currentPoolSet[0]);
            if (gpuDescHandle.ptr != 0)
                commandList_->SetComputeRootDescriptorTable(stagingDescriptorIndices_.rootParamDescriptors[0], gpuDescHandle);
        }
        if (stagingDescriptorSetLayout_.numSamplers > 0)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle = descriptorCache.FlushSamplerDescriptors(currentPoolSet[1]);
            if (gpuDescHandle.ptr != 0)
                commandList_->SetComputeRootDescriptorTable(stagingDescriptorIndices_.rootParamDescriptors[1], gpuDescHandle);
        }
    }
}


} // /namespace LLGL



// ================================================================================
