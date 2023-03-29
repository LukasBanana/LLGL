/*
 * MTPipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTPipelineLayout.h"


namespace LLGL
{


MTPipelineLayout::MTPipelineLayout(const PipelineLayoutDescriptor& desc) :
    heapBindings_ { desc.heapBindings }
{
}

std::uint32_t MTPipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(heapBindings_.size()); //TODO
}

std::uint32_t MTPipelineLayout::GetNumBindings() const
{
    return 0; //TODO
}

std::uint32_t MTPipelineLayout::GetNumStaticSamplers() const
{
    return 0; //TODO
}

std::uint32_t MTPipelineLayout::GetNumUniforms() const
{
    return 0; //TODO
}


} // /namespace LLGL



// ================================================================================
