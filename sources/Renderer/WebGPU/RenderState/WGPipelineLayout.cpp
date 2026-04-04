/*
 * WGPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineLayout.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


/* WGPipelineLayout::WGPipelineLayout(const PipelineLayoutDescriptor& desc) { ... } */

std::uint32_t WGPipelineLayout::GetNumHeapBindings() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGPipelineLayout::GetNumBindings() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGPipelineLayout::GetNumStaticSamplers() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

std::uint32_t WGPipelineLayout::GetNumUniforms() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
