/*
 * NullPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullPipelineLayout.h"


namespace LLGL
{


NullPipelineLayout::NullPipelineLayout(const PipelineLayoutDescriptor& desc) :
    desc { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullPipelineLayout::SetDebugName(const char* name)
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
