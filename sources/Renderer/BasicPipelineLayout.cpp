/*
 * BasicPipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "BasicPipelineLayout.h"


namespace LLGL
{


BasicPipelineLayout::BasicPipelineLayout(const PipelineLayoutDescriptor& desc) :
    bindings_ { desc.bindings }
{
}

std::uint32_t BasicPipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(bindings_.size());
}


} // /namespace LLGL



// ================================================================================
