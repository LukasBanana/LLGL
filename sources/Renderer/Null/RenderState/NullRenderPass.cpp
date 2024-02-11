/*
 * NullRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullRenderPass.h"


namespace LLGL
{


NullRenderPass::NullRenderPass(const RenderPassDescriptor& desc) :
    desc { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullRenderPass::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
