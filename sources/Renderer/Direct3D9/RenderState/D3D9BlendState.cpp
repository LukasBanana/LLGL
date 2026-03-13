/*
 * D3D9BlendState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9BlendState.h"
#include "D3D9StateManager.h"
#include "../D3D9Types.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/MacroUtils.h"


namespace LLGL
{


static bool IsColorAndAlphaBlendEqual(const BlendTargetDescriptor& targetDesc)
{
    return
    (
        targetDesc.colorArithmetic == targetDesc.alphaArithmetic &&
        targetDesc.srcColor        == targetDesc.srcAlpha        &&
        targetDesc.dstColor        == targetDesc.dstAlpha
    );
}

D3D9BlendState::D3D9BlendState(const BlendDescriptor& desc)
{
    /* D3D9 only supports uniform blend states for all targets */
    LLGL_ASSERT(!desc.independentBlendEnabled, "Direct3D9 does not support independent blend states");
    const BlendTargetDescriptor& target0Desc = desc.targets[0];

    if (target0Desc.blendEnabled)
    {
        isBlendFactorEnabled_ = (IsStaticBlendFactorEnabled(desc) ? TRUE : FALSE);
        if (isBlendFactorEnabled_)
            blendFactor_ = D3D9Types::ToD3DColor(desc.blendFactor);

        blendOp_                        = D3D9Types::ToD3DBlendOp(target0Desc.colorArithmetic);
        srcBlend_                       = D3D9Types::ToD3DBlend(target0Desc.srcColor);
        destBlend_                      = D3D9Types::ToD3DBlend(target0Desc.dstColor);

        isAlphaBlendEnabled_            = TRUE;
        isSeparateAlphaBlendEnabled_    = (!IsColorAndAlphaBlendEqual(target0Desc) ? TRUE : FALSE);
        blendOpAlpha_                   = D3D9Types::ToD3DBlendOp(target0Desc.alphaArithmetic);
        srcBlendAlpha_                  = D3D9Types::ToD3DBlend(target0Desc.srcAlpha);
        destBlendAlpha_                 = D3D9Types::ToD3DBlend(target0Desc.dstAlpha);
    }
}

void D3D9BlendState::Bind(D3D9StateManager& stateMngr)
{
    stateMngr.SetRenderState(D3DRS_BLENDOP, blendOp_);
    stateMngr.SetRenderState(D3DRS_SRCBLEND, srcBlend_);
    stateMngr.SetRenderState(D3DRS_DESTBLEND, destBlend_);

    if (isBlendFactorEnabled_)
        stateMngr.SetRenderState(D3DRS_BLENDFACTOR, blendFactor_);

    if (isAlphaBlendEnabled_)
    {
        stateMngr.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        if (isSeparateAlphaBlendEnabled_)
        {
            stateMngr.SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
            stateMngr.SetRenderState(D3DRS_BLENDOP, blendOpAlpha_);
            stateMngr.SetRenderState(D3DRS_SRCBLEND, srcBlendAlpha_);
            stateMngr.SetRenderState(D3DRS_DESTBLEND, destBlendAlpha_);
        }
        else
            stateMngr.SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
    }
    else
        stateMngr.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

int D3D9BlendState::CompareSWO(const D3D9BlendState& lhs, const D3D9BlendState& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( blendOp_              );
    LLGL_COMPARE_MEMBER_SWO( srcBlend_             );
    LLGL_COMPARE_MEMBER_SWO( destBlend_            );
    LLGL_COMPARE_MEMBER_SWO( isBlendFactorEnabled_ );
    if (lhs.isBlendFactorEnabled_)
        LLGL_COMPARE_MEMBER_SWO( isAlphaBlendEnabled_ );
    if (lhs.isAlphaBlendEnabled_)
    {
        LLGL_COMPARE_MEMBER_SWO( isSeparateAlphaBlendEnabled_ );
        if (lhs.isSeparateAlphaBlendEnabled_)
        {
            LLGL_COMPARE_MEMBER_SWO( blendOpAlpha_   );
            LLGL_COMPARE_MEMBER_SWO( srcBlendAlpha_  );
            LLGL_COMPARE_MEMBER_SWO( destBlendAlpha_ );
        }
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
