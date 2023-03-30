/*
 * MTPipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTPipelineLayout.h"
#include "../Texture/MTSampler.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


MTPipelineLayout::MTPipelineLayout(id<MTLDevice> device, const PipelineLayoutDescriptor& desc) :
    heapBindings_ { desc.heapBindings }
{
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
    return 0; //TODO
}

std::uint32_t MTPipelineLayout::GetNumStaticSamplers() const
{
    return numStaticSamplers_;
}

std::uint32_t MTPipelineLayout::GetNumUniforms() const
{
    return 0; //TODO
}

void MTPipelineLayout::SetStaticVertexSamplers(id<MTLRenderCommandEncoder> renderEncoder) const
{
    for_range(i, numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_VERTEX])
    {
        [renderEncoder
            setVertexSamplerState:  staticSamplerStates_[i]
            atIndex:                staticSamplerIndices_[i]
        ];
    }
}

void MTPipelineLayout::SetStaticFragmentSamplers(id<MTLRenderCommandEncoder> renderEncoder) const
{
    const auto offset = numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_FRAGMENT - 1];
    for_subrange(i, offset, offset + numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_FRAGMENT])
    {
        [renderEncoder
            setFragmentSamplerState:    staticSamplerStates_[i]
            atIndex:                    staticSamplerIndices_[i]
        ];
    }
}

void MTPipelineLayout::SetStaticKernelSamplers(id<MTLComputeCommandEncoder> computeEncoder) const
{
    const auto offset = numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_KERNEL - 1];
    for_subrange(i, offset, offset + numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_KERNEL])
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

void MTPipelineLayout::BuildStaticSamplers(
    id<MTLDevice>                               device,
    const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs)
{
    numStaticSamplers_ = static_cast<std::uint32_t>(staticSamplerDescs.size());
    staticSamplerStates_.reserve(staticSamplerDescs.size());
    staticSamplerIndices_.reserve(staticSamplerDescs.size());
    numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_VERTEX  ] = BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::VertexStage);
    numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_FRAGMENT] = BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::FragmentStage);
    numStaticSamplerPerStage_[LLGL_MT_SHADER_STAGE_KERNEL  ] = BuildStaticSamplersForStage(device, staticSamplerDescs, StageFlags::ComputeStage);
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
