/*
 * WGBindGroupCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGBindGroupCache.h"
#include "WGPipelineLayoutPermutation.h"
#include "../Buffer/WGBuffer.h"
#include "../Texture/WGTexture.h"
#include "../Texture/WGSampler.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <limits.h>
#include <algorithm>


namespace LLGL
{


static void InitBindGroupEntry(WGPUBindGroupEntry& outEntry, const WGPUBindGroupLayoutEntry& inLayoutEntry)
{
    outEntry.nextInChain    = nullptr;
    outEntry.binding        = inLayoutEntry.binding;
    outEntry.buffer         = nullptr;
    outEntry.offset         = 0;
    outEntry.size           = 0;
    outEntry.sampler        = nullptr;
    outEntry.textureView    = nullptr;
}

WGBindGroupCache::WGBindGroupCache(const WGPipelineLayoutPermutation* pipelineLayoutPermutation) :
    pipelineLayoutPermutation_ { pipelineLayoutPermutation }
{
    LLGL_ASSERT_PTR(pipelineLayoutPermutation);

    /* Initialize bind group entries */
    bindGroupEntries_.resize(pipelineLayoutPermutation->GetDescriptorMap().size());
    const std::vector<WGPipelineLayoutGroup>& layoutGroups = pipelineLayoutPermutation->GetLayoutGroups();

    bindGroups_.resize(layoutGroups.size());

    std::size_t entryIndex = 0;
    for_range(group, layoutGroups.size())
    {
        bindGroups_[group].dirtyBit = 0;
        bindGroups_[group].firstEntry = entryIndex;
        {
            const WGPipelineLayoutGroup& layoutGroup = layoutGroups[group];
            for (const WGPUBindGroupLayoutEntry& layoutEntry : layoutGroup.entries)
                InitBindGroupEntry(bindGroupEntries_[entryIndex++], layoutEntry);
        }
        bindGroups_[group].numEntries = entryIndex - bindGroups_[group].firstEntry;
    }

    ResetDirtRange();
}

static void WriteBindGroupEntryForBuffer(WGPUBindGroupEntry& wgpuEntry, WGBuffer& bufferWG)
{
    wgpuEntry.buffer    = bufferWG.GetNative();
    wgpuEntry.offset    = 0;
    wgpuEntry.size      = bufferWG.GetSize();
}

static void WriteBindGroupEntryForTexture(WGPUBindGroupEntry& wgpuEntry, WGTexture& textureWG)
{
    wgpuEntry.textureView = textureWG.GetDefaultTextureView();
}

static void WriteBindGroupEntryForSampler(WGPUBindGroupEntry& wgpuEntry, WGSampler& samplerWG)
{
    wgpuEntry.sampler = samplerWG.GetNative();
}

bool WGBindGroupCache::EmplaceResource(Resource& resource, std::uint32_t location)
{
    /* Map descriptor to bind-group entry */
    const std::vector<WGPipelineLayoutDescriptor>& descriptorMap = pipelineLayoutPermutation_->GetDescriptorMap();
    if (location >= descriptorMap.size())
        return false; /*Out of bounds*/

    const WGPipelineLayoutDescriptor& descriptor = descriptorMap[location];
    LLGL_ASSERT(descriptor.groupIndex < bindGroups_.size());

    const BindGroupInfo& groupInfo = bindGroups_[descriptor.groupIndex];
    const std::uint32_t entryIndex = groupInfo.firstEntry + descriptor.entryIndex;
    LLGL_ASSERT(entryIndex < bindGroupEntries_.size());

    /* Write descriptor entry */
    WGPUBindGroupEntry& wgpuEntry = bindGroupEntries_[entryIndex];
    //const WGPipelineLayoutGroup& layoutGroup = pipelineLayoutPermutation_->GetLayoutGroups()[descriptor.groupIndex];
    //const WGPUBindGroupLayoutEntry& layoutEntry = layoutGroup.entries[descriptor.entryIndex];

    switch (resource.GetResourceType())
    {
        case ResourceType::Buffer:
            WriteBindGroupEntryForBuffer(wgpuEntry, LLGL_CAST(WGBuffer&, resource));
            break;

        case ResourceType::Texture:
            WriteBindGroupEntryForTexture(wgpuEntry, LLGL_CAST(WGTexture&, resource));
            break;

        case ResourceType::Sampler:
            WriteBindGroupEntryForSampler(wgpuEntry, LLGL_CAST(WGSampler&, resource));
            break;

        default:
            return false; /*Invalid argument*/
    }

    /* Invalidate cache entry */
    InvalidateBindGroup(descriptor.groupIndex);

    return true;
}

void WGBindGroupCache::FlushRenderPassBindGroups(WGPUDevice device, WGPURenderPassEncoder renderPassEncoder)
{
    if (bindGroupDirtyRange_[0] < bindGroupDirtyRange_[1])
    {
        for_subrange(group, bindGroupDirtyRange_[0], bindGroupDirtyRange_[1])
        {
            if (bindGroups_[group].dirtyBit != 0)
            {
                FlushRenderPassBindGroup(device, renderPassEncoder, group);
                bindGroups_[group].dirtyBit = 0;
            }
        }
        ResetDirtRange();
    }
}

void WGBindGroupCache::FlushComputePassBindGroups(WGPUDevice device, WGPUComputePassEncoder computePassEncoder)
{
    //TODO
}

void WGBindGroupCache::Invalidate(std::uint16_t first, std::uint16_t last)
{
    bindGroupDirtyRange_[0] = std::min<std::uint16_t>(bindGroupDirtyRange_[0], first);
    bindGroupDirtyRange_[1] = std::min<std::uint16_t>(static_cast<std::uint16_t>(bindGroups_.size()), std::max<std::uint16_t>(bindGroupDirtyRange_[1], last));
}


/*
 * ======= Private: =======
 */

void WGBindGroupCache::ResetDirtRange()
{
    bindGroupDirtyRange_[0] = UINT16_MAX;
    bindGroupDirtyRange_[1] = 0;
}

void WGBindGroupCache::FlushRenderPassBindGroup(WGPUDevice device, WGPURenderPassEncoder renderPassEncoder, std::uint32_t groupIndex)
{
    const BindGroupInfo& groupInfo = bindGroups_[groupIndex];

    /* Hash bind group entries memory as 64-bit integers (equal to alignment of WGPUBindGroupEntry) */
    static_assert(alignof(WGPUBindGroupEntry) == sizeof(std::uint64_t), "alignment of WGPUBindGroupEntry must be equal to size of uint64_t");
    const WGPUBindGroupEntry* firstEntry = &(bindGroupEntries_[groupInfo.firstEntry]);
    BindGroupKey key = HashRange(
        reinterpret_cast<const std::uint64_t*>(firstEntry),
        reinterpret_cast<const std::uint64_t*>(firstEntry + groupInfo.numEntries)
    );

    auto it = cachedBindGroups_.find(key);
    if (it == cachedBindGroups_.end())
    {
        /* Allocate a new WebGPU bind group */
        const WGPipelineLayoutGroup& layoutGroup = pipelineLayoutPermutation_->GetLayoutGroups()[groupIndex];
        WGBindGroupPtr bindGroup = MakeUnique<WGBindGroup>(
            device,
            layoutGroup.bindGroupLayout,
            ArrayView<WGPUBindGroupEntry>{ &(bindGroupEntries_[groupInfo.firstEntry]), groupInfo.numEntries }
        );
        SetRenderPassBindGroup(renderPassEncoder, groupIndex, bindGroup.get());
        (void)cachedBindGroups_.insert(std::pair<BindGroupKey, WGBindGroupPtr>{ key, std::move(bindGroup) });
    }
    else
    {
        /* Set existing bind group */
        SetRenderPassBindGroup(renderPassEncoder, groupIndex, it->second.get());
    }
}

void WGBindGroupCache::SetRenderPassBindGroup(WGPURenderPassEncoder renderPassEncoder, std::uint32_t groupIndex, const WGBindGroup* bindGroup)
{
    wgpuRenderPassEncoderSetBindGroup(renderPassEncoder, groupIndex, bindGroup->GetNative(), 0, nullptr);
}

void WGBindGroupCache::InvalidateBindGroup(std::uint32_t groupIndex)
{
    if (bindGroups_[groupIndex].dirtyBit == 0)
    {
        bindGroups_[groupIndex].dirtyBit = 1;
        Invalidate(static_cast<std::uint16_t>(groupIndex), static_cast<std::uint16_t>(groupIndex + 1));
    }
}


} // /namespace LLGL



// ================================================================================
