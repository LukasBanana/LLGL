/*
 * WGPipelineLayoutPermutation.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineLayoutPermutation.h"
#include "../WGCore.h"
#include "../Shader/WGResourceReflectionTable.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static const WGResourceReflection* FindResourceReflection(ArrayView<const WGResourceReflectionTable*> resourceTables, const char* name)
{
    for (const WGResourceReflectionTable* resourceTable : resourceTables)
    {
        if (const WGResourceReflection* resourceInfo = resourceTable->FindResource(name))
            return resourceInfo;
    }
    return nullptr;
}

static void UpdateBindGroupLayoutEntry(
    std::uint32_t                               entryIndex,
    WGPUBindGroupLayoutEntry&                   outEntry,
    std::uint32_t&                              outGroupIndex,
    const char*                                 entryName,
    ArrayView<const WGResourceReflectionTable*> resourceTables,
    const char*                                 debugName,
    Report&                                     outReport)
{
    if (const WGResourceReflection* resourceInfo = FindResourceReflection(resourceTables, entryName))
    {
        /* Update binding index and warn on mismatch */
        if (outEntry.binding != resourceInfo->bindingIndex)
        {
            outReport.Printf(
                "warning: resource '%s' mismatch between specified binding index (%u) and shader reflection (%u)\n",
                entryName, outEntry.binding, resourceInfo->bindingIndex
            );
            outEntry.binding = resourceInfo->bindingIndex;
        }

        /* Return group index for each entry */
        outGroupIndex = resourceInfo->groupIndex;

        /* Update resource information */
        outEntry.storageTexture.access          = resourceInfo->storageTextureAccess;
        outEntry.storageTexture.format          = resourceInfo->storageTextureFormat;
        outEntry.storageTexture.viewDimension   = resourceInfo->textureViewDimension;

        outEntry.texture.sampleType             = resourceInfo->textureSampleType;
        outEntry.texture.viewDimension          = resourceInfo->textureViewDimension;
        outEntry.texture.multisampled           = resourceInfo->multisampled;

        outEntry.sampler.type                   = resourceInfo->samplerBindingType;
    }
#if 0
    else
    {
        /* Error: Failed to find resource by name */
        const char* entryNameExt = (*entryName != '\0' ? entryName : "<unnamed>");
        outReport.Errorf(
            "binding '%s' (%u) not found in resource table for pipeline layout '%s'\n",
            entryNameExt, entryIndex, debugName
        );
    }
#endif
}

WGPipelineLayoutPermutation::WGPipelineLayoutPermutation(
    WGPUDevice                                  device,
    ArrayView<WGPUBindGroupLayoutEntry>         bindGroupEntries,
    ArrayView<std::string>                      bindGroupEntryNames,
    ArrayView<const WGResourceReflectionTable*> resourceTables,
    std::uint32_t                               immediateSize,
    const WGCoreLimits&                         coreLimits,
    const char*                                 debugName,
    Report&                                     outReport)
{
    LLGL_ASSERT(bindGroupEntries.size() == bindGroupEntryNames.size());

    /* Update bind group layout entries for this permutation depending on resource reflection tables */
    const char* debugNameExt = (*debugName != '\0' ? debugName : "<unnamed>");

    descriptorMap_.resize(bindGroupEntries.size());
    for_range(i, bindGroupEntries.size())
    {
        WGPUBindGroupLayoutEntry groupEntry = bindGroupEntries[i];

        std::uint32_t groupIndex = 0;
        UpdateBindGroupLayoutEntry(
            static_cast<std::uint32_t>(i),
            groupEntry,
            groupIndex,
            bindGroupEntryNames[i].c_str(),
            resourceTables,
            debugNameExt,
            outReport
        );

        /* Append entry to bind group layout */
        if (!(groupIndex < coreLimits.maxBindGroups))
        {
            outReport.Errorf(
                "descriptor group index (%u) out of bounds; upper bound is %u\n",
                groupIndex, coreLimits.maxBindGroups
            );
        }
        else if (!(groupEntry.binding < coreLimits.maxBindingsPerBindGroup))
        {
            outReport.Errorf(
                "descriptor binding index (%u) out of bounds; upper bound is %u\n",
                groupEntry.binding, coreLimits.maxBindingsPerBindGroup
            );
        }
        else
        {
            if (!(groupIndex < layoutGroups_.size()))
                layoutGroups_.resize(groupIndex + 1);

            const std::size_t entryIndex = layoutGroups_[groupIndex].entries.size();
            layoutGroups_[groupIndex].entries.push_back(groupEntry);

            /* Store descriptor mapping to bind-group entry */
            descriptorMap_[i].groupIndex = groupIndex;
            descriptorMap_[i].entryIndex = static_cast<std::uint32_t>(entryIndex);
        }
    }

    /* Create bind group layouts */
    std::vector<WGPUBindGroupLayout> bindGroupLayouts;
    bindGroupLayouts.resize(layoutGroups_.size());

    for_range(group, layoutGroups_.size())
    {
        if (layoutGroups_[group].entries.empty())
        {
            outReport.Errorf("cannot create WGPUBindGroupLayout without bind-group entries (group=%u)\n", group);
            return;
        }

        WGPUBindGroupLayoutDescriptor wgpuBindGroupDesc;
        {
            wgpuBindGroupDesc.nextInChain   = nullptr;
            wgpuBindGroupDesc.label         = ToWGStringView(debugName);
            wgpuBindGroupDesc.entryCount    = layoutGroups_[group].entries.size();
            wgpuBindGroupDesc.entries       = layoutGroups_[group].entries.data();
        }
        WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(device, &wgpuBindGroupDesc);
        WGThrowIfCreateFailed(bindGroupLayout, "WGPUBindGroupLayout");

        bindGroupLayouts[group] = bindGroupLayout;
        layoutGroups_[group].bindGroupLayout = bindGroupLayout;
    }

    /* Create native WebGPU pipeline layout wiht a single bind group */
    WGPUPipelineLayoutDescriptor wgpuLayoutDesc;
    {
        wgpuLayoutDesc.nextInChain  = nullptr;
        wgpuLayoutDesc.label        = ToWGStringView(debugName);
        if (!bindGroupLayouts.empty())
        {
            wgpuLayoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
            wgpuLayoutDesc.bindGroupLayouts     = bindGroupLayouts.data();
        }
        else
        {
            wgpuLayoutDesc.bindGroupLayoutCount = 0;
            wgpuLayoutDesc.bindGroupLayouts     = nullptr;
        }
        wgpuLayoutDesc.immediateSize = immediateSize;
    }
    pipelineLayout_ = wgpuDeviceCreatePipelineLayout(device, &wgpuLayoutDesc);
    LLGL_ASSERT_PTR(pipelineLayout_);

    /* Allocate bind group cache if this pipeline layout contains any bind groups */
    if (!bindGroupLayouts.empty())
        bindGroupCache_ = MakeUnique<WGBindGroupCache>(this);
}

WGPipelineLayoutPermutation::~WGPipelineLayoutPermutation()
{
    for (WGPipelineLayoutGroup& layoutGroup : layoutGroups_)
        wgpuBindGroupLayoutRelease(layoutGroup.bindGroupLayout);
    wgpuPipelineLayoutRelease(pipelineLayout_);
}


} // /namespace LLGL



// ================================================================================
