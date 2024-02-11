/*
 * DbgPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgPipelineLayout.h"
#include "../DbgCore.h"


namespace LLGL
{


DbgPipelineLayout::DbgPipelineLayout(PipelineLayout& instance, const PipelineLayoutDescriptor& desc) :
    instance { instance             },
    desc     { desc                 },
    label    { LLGL_DBG_LABEL(desc) }
{
}

void DbgPipelineLayout::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

std::uint32_t DbgPipelineLayout::GetNumHeapBindings() const
{
    return instance.GetNumHeapBindings();
}

std::uint32_t DbgPipelineLayout::GetNumBindings() const
{
    return instance.GetNumBindings();
}

std::uint32_t DbgPipelineLayout::GetNumStaticSamplers() const
{
    return instance.GetNumStaticSamplers();
}

std::uint32_t DbgPipelineLayout::GetNumUniforms() const
{
    return instance.GetNumUniforms();
}


} // /namespace LLGL



// ================================================================================
