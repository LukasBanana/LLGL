/*
 * D3D9PipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineLayout.h"


namespace LLGL
{


D3D9PipelineLayout::D3D9PipelineLayout(const PipelineLayoutDescriptor& desc) :
    numHeapBindings_   { static_cast<std::uint32_t>(desc.heapBindings.size())   },
    numBindings_       { static_cast<std::uint32_t>(desc.bindings.size())       },
    numStaticSamplers_ { static_cast<std::uint32_t>(desc.staticSamplers.size()) },
    uniformDesc_       { desc.uniforms                                          }
{
}

void D3D9PipelineLayout::SetDebugName(const char* name)
{
    // dummy
}

std::uint32_t D3D9PipelineLayout::GetNumHeapBindings() const
{
    return numHeapBindings_;
}

std::uint32_t D3D9PipelineLayout::GetNumBindings() const
{
    return numBindings_;
}

std::uint32_t D3D9PipelineLayout::GetNumStaticSamplers() const
{
    return numStaticSamplers_;
}

std::uint32_t D3D9PipelineLayout::GetNumUniforms() const
{
    return static_cast<std::uint32_t>(uniformDesc_.size());
}


} // /namespace LLGL



// ================================================================================
