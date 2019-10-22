/*
 * PipelineStateUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "PipelineStateUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


LLGL_EXPORT bool IsStaticStencilRefEnabled(const StencilDescriptor& desc)
{
    return (desc.testEnabled && !desc.referenceDynamic);
}

static bool IsBlendOpUsingBlendFactor(const BlendOp op)
{
    return (op == BlendOp::BlendFactor || op == BlendOp::InvBlendFactor);
}

static bool IsTargetUsingBlendFactor(const BlendTargetDescriptor& desc)
{
    return
    (
        desc.blendEnabled &&
        (
            IsBlendOpUsingBlendFactor(desc.srcColor) ||
            IsBlendOpUsingBlendFactor(desc.dstColor) ||
            IsBlendOpUsingBlendFactor(desc.srcAlpha) ||
            IsBlendOpUsingBlendFactor(desc.dstAlpha)
        )
    );
}

LLGL_EXPORT bool IsStaticBlendFactorEnabled(const BlendDescriptor& desc)
{
    if (!desc.blendFactorDynamic)
    {
        if (desc.independentBlendEnabled)
        {
            for (const auto& target : desc.targets)
            {
                if (IsTargetUsingBlendFactor(target))
                    return true;
            }
        }
        else
        {
            if (IsTargetUsingBlendFactor(desc.targets[0]))
                return true;
        }
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
