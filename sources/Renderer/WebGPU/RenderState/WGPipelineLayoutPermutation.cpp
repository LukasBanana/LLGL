/*
 * WGPipelineLayoutPermutation.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineLayoutPermutation.h"
#include "../WGCore.h"
#include "../Shader/WGResourceReflectionTable.h"
#include "../../../Core/Assertion.h"
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
    std::uint64_t&                              outGroupIndex,
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
    const char*                                 debugName,
    Report&                                     outReport)
{
    /*
    Temporary container to store group indices for each entry.
    Used to sort and determine how many bind groups must be allocated.
    */
    std::vector<std::uint64_t> bindGroupIndices;
    bindGroupIndices.resize(bindGroupEntries.size());

    /* Update bind group layout entries for this permutation depending on resource reflection tables */
    const char* debugNameExt = (*debugName != '\0' ? debugName : "<unnamed>");

    std::vector<WGPUBindGroupLayoutEntry> bindGroupEntryPermutations{ bindGroupEntries.begin(), bindGroupEntries.end() };
    for_range(i, bindGroupEntries.size())
    {
        UpdateBindGroupLayoutEntry(
            static_cast<std::uint32_t>(i),
            bindGroupEntryPermutations[i],
            bindGroupIndices[i],
            bindGroupEntryNames[i].c_str(),
            resourceTables,
            debugNameExt,
            outReport
        );
    }

    /* Create bind group layout */
    if (!bindGroupEntryPermutations.empty())
    {
        WGPUBindGroupLayoutDescriptor wgpuBindGroupDesc;
        {
            wgpuBindGroupDesc.nextInChain   = nullptr;
            wgpuBindGroupDesc.label         = ToWGStringView(debugName);
            wgpuBindGroupDesc.entryCount    = bindGroupEntryPermutations.size();
            wgpuBindGroupDesc.entries       = bindGroupEntryPermutations.data();
        }
        bindGroupLayout_ = wgpuDeviceCreateBindGroupLayout(device, &wgpuBindGroupDesc);
        LLGL_ASSERT_PTR(bindGroupLayout_);
    }

    /* Create native WebGPU pipeline layout wiht a single bind group */
    WGPUPipelineLayoutDescriptor wgpuLayoutDesc;
    {
        wgpuLayoutDesc.nextInChain  = nullptr;
        wgpuLayoutDesc.label        = ToWGStringView(debugName);
        if (bindGroupLayout_ != nullptr)
        {
            wgpuLayoutDesc.bindGroupLayoutCount = 1;
            wgpuLayoutDesc.bindGroupLayouts     = &bindGroupLayout_;
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
}

WGPipelineLayoutPermutation::~WGPipelineLayoutPermutation()
{
    if (bindGroupLayout_ != nullptr)
        wgpuBindGroupLayoutRelease(bindGroupLayout_);
    wgpuPipelineLayoutRelease(pipelineLayout_);
}


} // /namespace LLGL



// ================================================================================
