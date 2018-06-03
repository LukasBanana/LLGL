/*
 * GLPipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLPipelineLayout.h"


namespace LLGL
{


GLPipelineLayout::GLPipelineLayout(const PipelineLayoutDescriptor& desc) :
    bindings_ { desc.bindings }
{
}


} // /namespace LLGL



// ================================================================================
