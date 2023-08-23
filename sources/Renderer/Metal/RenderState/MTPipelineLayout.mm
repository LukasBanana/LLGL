/*
 * MTPipelineLayout.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTPipelineLayout.h"
#include "../Texture/MTSampler.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


MTPipelineLayout::MTPipelineLayout(id<MTLDevice> device, const PipelineLayoutDescriptor& desc) :
    heapBindings_ { desc.heapBindings },
    uniforms_     { desc.uniforms     }
{
    BuildDynamicBindings(desc.bindings);
    BuildStaticSamplers(device, desc.staticSamplers);
}

MTPipelineLayout::~MTPipelineLayout()
{
    /* Release all static sampler states explicitly */
    for (id<MTLSamplerState> samplerState : staticSamplerStates_)
        [samplerState release];
}

std::uint32_t MTPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size());
}

std::uint32_t MTPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(dynamicBindings_.size());
}

std::uint32_t MTPipelineLayout::GetNumStaticSamplers() const
{
    return numStaticSamplers_;
}

std::uint32_t MTPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniforms_.size());
}

void MTPipelineLayout::SetStaticVertexSamplers(id<MTLRenderCommandEncoder> renderEncoder) const
{
    for_range(i, numStaticSamplerPerStage_[MTShaderStage_Vertex])
    {
        [renderEncoder
            setVertexSamplerState:  staticSamplerStates_[i]
            atIndex:                staticSamplerIndices_[i]
        ];
    }
}

void MTPipelineLayout::SetStaticFragmentSamplers(id<MTLRenderCommandEncoder> renderEncoder) const
{
    const auto offset = numStaticSamplerPerStage_[MTShaderStage_Fragment - 1];
    for_subrange(i, offset, offset + numStaticSamplerPerStage_[MTShaderStage_Fragment])
    {
        [renderEncoder
            setFragmentSamplerState:    staticSamplerStates_[i]
            atIndex:                    staticSamplerIndices_[i]
        ];
    }
}

void MTPipelineLayout::SetStaticKernelSamplers(id<MTLComputeCommandEncoder> computeEncoder) const
{
    const auto offset = numStaticSamplerPerStage_[MTShaderStage_Kernel - 1];
    for_subrange(i, offset, offset + numStaticSamplerPerStage_[MTShaderStage_Kernel])
    {
        [computeEncoder
            setSamplerState:    staticSamplerStates_[i]
            atIndex:            staticSamplerIndices_[i]
        ];
    }
}


/*
 * ======= Private: =======
 */

static void Convert(MTDynamicResourceLayout& dst, const BindingDescriptor& src)
{
    dst.type    = src.type;
    dst.slot    = src.slot.index;
    dst.stages  = src.stageFlags;
}

void MTPipelineLayout::BuildDynamicBindings(const ArrayView<BindingDescriptor>& bindings)
{
    dynamicBindings_.resize(bindings.size());
    for_range(i, bindings.size())
        Convert(dynamicBindings_[i], bindings[i]);
}

void MTPipelineLayout::BuildStaticSamplers(
    id<MTLDevice>                               device,
    const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs)
{
    numStaticSamplers_ = static_cast<std::uint32_t>(staticSamplerDescs.size());
    staticSamplerStates_.reserve(staticSamplerDescs.size());
    staticSamplerIndices_.reserve(staticSamplerDescs.size());
    numStaticSamplerPerStage_[MTShaderStage_Vertex  ] = static_cast<std::uint32_t>(BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::VertexStage));
    numStaticSamplerPerStage_[MTShaderStage_Fragment] = static_cast<std::uint32_t>(BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::FragmentStage));
    numStaticSamplerPerStage_[MTShaderStage_Kernel  ] = static_cast<std::uint32_t>(BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::ComputeStage));
}

std::size_t MTPipelineLayout::BuildStaticSamplersForStage(
    id<MTLDevice>                               device,
    const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs,
    long                                        stageFlags)
{
    std::size_t count = 0;

    for (const auto& desc : staticSamplerDescs)
    {
        if ((desc.stageFlags & stageFlags) != 0)
        {
            staticSamplerStates_.push_back(MTSampler::CreateNative(device, desc.sampler));
            staticSamplerIndices_.push_back(desc.slot.index);
            ++count;
        }
    }

    return count;
}


} // /namespace LLGL



// ================================================================================
