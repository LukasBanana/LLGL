/*
 * DbgRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgRenderPass.h"
#include "../DbgCore.h"
#include "../../RenderPassUtils.h"


namespace LLGL
{


DbgRenderPass::DbgRenderPass(RenderPass& instance, const RenderPassDescriptor& desc) :
    instance        { instance  },
    mutableInstance { &instance },
    desc            { desc      }
{
}

DbgRenderPass::DbgRenderPass(const RenderPass& instance, const RenderPassDescriptor& desc) :
    instance        { instance },
    mutableInstance { nullptr  },
    desc            { desc     }
{
}

void DbgRenderPass::SetName(const char* name)
{
    /* Render passes have to be named manually with an explicitly multable instance, because they can be queried from RenderTarget::GetRenderPass() */
    if (mutableInstance != nullptr)
    {
        /* Set or clear label */
        if (name != nullptr)
            label = name;
        else
            label.clear();

        /* Forward call to instance */
        mutableInstance->SetName(name);
    }
}

std::uint32_t DbgRenderPass::NumEnabledColorAttachments() const
{
    return LLGL::NumEnabledColorAttachments(desc);
}


} // /namespace LLGL



// ================================================================================
