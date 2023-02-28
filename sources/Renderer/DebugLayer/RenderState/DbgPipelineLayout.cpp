/*
 * DbgPipelineLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgPipelineLayout.h"
#include "../DbgCore.h"


namespace LLGL
{


DbgPipelineLayout::DbgPipelineLayout(PipelineLayout& instance, const PipelineLayoutDescriptor& desc) :
    instance { instance },
    desc     { desc     }
{
}

void DbgPipelineLayout::SetName(const char* name)
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
