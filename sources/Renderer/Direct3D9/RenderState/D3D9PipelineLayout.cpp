/*
 * D3D9PipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineLayout.h"
#include "D3D9StateManager.h"
#include <LLGL/Utils/ForRange.h>
#include "../../../Core/Assertion.h"


namespace LLGL
{


static void ConvertD3DResourceBinding(D3D9ResourceBinding& outBinding, const BindingDescriptor& inBinding)
{
    outBinding.type         = inBinding.type;
    outBinding.combiners    = 0;
    outBinding.stage        = static_cast<DWORD>(inBinding.slot.index);
}

D3D9PipelineLayout::D3D9PipelineLayout(const PipelineLayoutDescriptor& desc) :
    uniformDesc_            { desc.uniforms                                          },
    numUniqueStaticSampler_ { static_cast<std::uint32_t>(desc.staticSamplers.size()) }
{
    /* Build resource binding table */
    resourceBindingTable_.resourceBindings.resize(desc.bindings.size());
    for_range(i, desc.bindings.size())
    {
        ConvertD3DResourceBinding(resourceBindingTable_.resourceBindings[i], desc.bindings[i]);
        BuildCombinedSamplerStages(desc, resourceBindingTable_.resourceBindings[i], desc.bindings[i].name);
    }

    /* Build static sampler states and duplicate for combined texture-samplers */
    staticSamplers_.reserve(desc.staticSamplers.size());
    for (const StaticSamplerDescriptor& staticSamplerDesc : desc.staticSamplers)
        BuildStaticSampler(desc, staticSamplerDesc);
}

void D3D9PipelineLayout::SetDebugName(const char* name)
{
    // dummy
}

std::uint32_t D3D9PipelineLayout::GetNumHeapBindings() const
{
    return 0; //TODO
}

std::uint32_t D3D9PipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(resourceBindingTable_.resourceBindings.size());
}

std::uint32_t D3D9PipelineLayout::GetNumStaticSamplers() const
{
    return numUniqueStaticSampler_;
}

std::uint32_t D3D9PipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniformDesc_.size());
}

void D3D9PipelineLayout::BindStaticSamplers(D3D9StateManager& stateMngr) const
{
    for (const D3DStaticSamplerAndStage& samplerAndStage : staticSamplers_)
        stateMngr.BindSampler(samplerAndStage.stage, samplerAndStage.sampler.get());
}


/*
 * ======= Private: =======
 */

void D3D9PipelineLayout::BuildCombinedSamplerStages(const PipelineLayoutDescriptor& pipelineLayoutDesc, D3D9ResourceBinding& resourceBinding, StringView name)
{
    const ResourceType type = resourceBinding.type;
    if (!pipelineLayoutDesc.combinedTextureSamplers.empty() && (type == ResourceType::Texture || type == ResourceType::Sampler))
    {
        const std::size_t firstStageIndex = combinedSamplerStages_.size();

        /* Find texture or sampler name in list of combined texture-samplers */
        for (const CombinedTextureSamplerDescriptor& desc : pipelineLayoutDesc.combinedTextureSamplers)
        {
            if ((type == ResourceType::Texture && desc.textureName == name) ||
                (type == ResourceType::Sampler && desc.samplerName == name))
            {
                combinedSamplerStages_.push_back(static_cast<DWORD>(desc.slot.index));
            }
        }

        /* Return start index and number of slots if the list has grown */
        const std::size_t numStages = (combinedSamplerStages_.size() - firstStageIndex);
        if (numStages > 0)
        {
            resourceBinding.stage       = static_cast<DWORD>(firstStageIndex);
            resourceBinding.combiners   = static_cast<UINT>(numStages);
        }
    }
}

void D3D9PipelineLayout::BuildStaticSampler(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StaticSamplerDescriptor& staticSamplerDesc)
{
    D3D9EmulatedSamplerSPtr newStaticSampler = std::make_shared<D3D9EmulatedSampler>(staticSamplerDesc.sampler);

    /* Try to add static sampler as combined texture-samplers. Otherwise, use binding slot directly from the primary static sampler descriptor */
    if (!AddCombinedStaticSamplers(pipelineLayoutDesc, staticSamplerDesc, newStaticSampler))
        AddStaticSampler(staticSamplerDesc.slot, newStaticSampler);
}

bool D3D9PipelineLayout::AddCombinedStaticSamplers(const PipelineLayoutDescriptor& pipelineLayoutDesc, const StaticSamplerDescriptor& staticSamplerDesc, const D3D9EmulatedSamplerSPtr& newStaticSampler)
{
    bool isCombinedStaticSampler = false;
    for (const CombinedTextureSamplerDescriptor& combinedSamplerDesc : pipelineLayoutDesc.combinedTextureSamplers)
    {
        if (combinedSamplerDesc.samplerName == staticSamplerDesc.name)
        {
            /* If there is at least one entry in the combined texture-sampler descriptors, mark this static sampler as such a combined sampler */
            AddStaticSampler(combinedSamplerDesc.slot, newStaticSampler);
            isCombinedStaticSampler = true;
        }
    }
    return isCombinedStaticSampler;
}

void D3D9PipelineLayout::AddStaticSampler(const BindingSlot& slot, const D3D9EmulatedSamplerSPtr& newStaticSampler)
{
    staticSamplers_.push_back(D3DStaticSamplerAndStage{ static_cast<DWORD>(slot.index), newStaticSampler });
}


} // /namespace LLGL



// ================================================================================
