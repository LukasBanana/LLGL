/*
 * D3D9RenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9RenderPass.h"


namespace LLGL
{


D3D9RenderPass::D3D9RenderPass(const RenderPassDescriptor& desc) :
    desc { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9RenderPass::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
