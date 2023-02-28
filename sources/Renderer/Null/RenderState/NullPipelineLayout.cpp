/*
 * NullPipelineLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullPipelineLayout.h"


namespace LLGL
{


NullPipelineLayout::NullPipelineLayout(const PipelineLayoutDescriptor& desc) :
    desc { desc }
{
}

void NullPipelineLayout::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

std::uint32_t NullPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(desc.heapBindings.size());
}

std::uint32_t NullPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(desc.bindings.size());
}

std::uint32_t NullPipelineLayout::GetNumStaticSamplers() const
{
    return static_cast<std::uint32_t>(desc.staticSamplers.size());
}

std::uint32_t NullPipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(desc.uniforms.size());
}


} // /namespace LLGL



// ================================================================================
