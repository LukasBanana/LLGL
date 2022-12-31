/*
 * NullRenderPass.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullRenderPass.h"


namespace LLGL
{


NullRenderPass::NullRenderPass(const RenderPassDescriptor& desc) :
    desc { desc }
{
}

void NullRenderPass::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
