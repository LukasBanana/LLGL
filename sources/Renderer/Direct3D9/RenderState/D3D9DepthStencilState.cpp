/*
 * D3D9DepthStencilState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9DepthStencilState.h"
#include "D3D9StateManager.h"
#include "../D3D9Types.h"
#include "../../../Core/MacroUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D9DepthStencilState::D3D9DepthStencilState(const DepthDescriptor& depthDesc, const StencilDescriptor& stencilDesc)
{
    /* Convert depth states */
    zEnable_                = (depthDesc.testEnabled  ? TRUE : FALSE);
    zWriteEnable_           = (depthDesc.writeEnabled ? TRUE : FALSE);
    zFunc_                  = D3D9Types::ToD3DCmpFunc(depthDesc.compareOp);

    /* Convert stencil states */
    stencilEnable_          = (stencilDesc.testEnabled ? TRUE : FALSE);
    stencilRef_             = stencilDesc.front.reference;
    stencilMask_            = stencilDesc.front.readMask;
    stencilWriteMask_       = stencilDesc.front.writeMask;

    stencilFront_.opFail    = D3D9Types::ToD3DStenciOp(stencilDesc.front.stencilFailOp);
    stencilFront_.opZFail   = D3D9Types::ToD3DStenciOp(stencilDesc.front.depthFailOp);
    stencilFront_.opPass    = D3D9Types::ToD3DStenciOp(stencilDesc.front.depthPassOp);
    stencilFront_.cmpFunc   = D3D9Types::ToD3DCmpFunc(stencilDesc.front.compareOp);

    stencilBack_.opFail     = D3D9Types::ToD3DStenciOp(stencilDesc.back.stencilFailOp);
    stencilBack_.opZFail    = D3D9Types::ToD3DStenciOp(stencilDesc.back.depthFailOp);
    stencilBack_.opPass     = D3D9Types::ToD3DStenciOp(stencilDesc.back.depthPassOp);
    stencilBack_.cmpFunc    = D3D9Types::ToD3DCmpFunc(stencilDesc.back.compareOp);
}

void D3D9DepthStencilState::Bind(D3D9StateManager& stateMngr)
{
    /* Setup depth state */
    if (zEnable_)
    {
        stateMngr.SetRenderState(D3DRS_ZENABLE, TRUE);
        stateMngr.SetRenderState(D3DRS_ZFUNC, zFunc_);
    }
    else
        stateMngr.SetRenderState(D3DRS_ZENABLE, FALSE);

    stateMngr.SetRenderState(D3DRS_ZWRITEENABLE, zWriteEnable_);

    /* Setup stencil state */
    if (stencilEnable_)
    {
        stateMngr.SetRenderState(D3DRS_STENCILENABLE, TRUE);

        stateMngr.SetRenderState(D3DRS_STENCILREF, stencilRef_);
        stateMngr.SetRenderState(D3DRS_STENCILMASK, stencilMask_);
        stateMngr.SetRenderState(D3DRS_STENCILWRITEMASK, stencilWriteMask_);

        stateMngr.SetRenderState(D3DRS_STENCILFAIL, stencilFront_.opFail);
        stateMngr.SetRenderState(D3DRS_STENCILZFAIL, stencilFront_.opZFail);
        stateMngr.SetRenderState(D3DRS_STENCILPASS, stencilFront_.opPass);
        stateMngr.SetRenderState(D3DRS_STENCILFUNC, stencilFront_.cmpFunc);

        if (twoSidedStencilMode_)
        {
            stateMngr.SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);

            stateMngr.SetRenderState(D3DRS_CCW_STENCILFAIL, stencilBack_.opFail);
            stateMngr.SetRenderState(D3DRS_CCW_STENCILZFAIL, stencilBack_.opZFail);
            stateMngr.SetRenderState(D3DRS_CCW_STENCILPASS, stencilBack_.opPass);
            stateMngr.SetRenderState(D3DRS_CCW_STENCILFUNC, stencilBack_.cmpFunc);
        }
        else
            stateMngr.SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
    }
    else
        stateMngr.SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

int D3D9DepthStencilState::CompareSWO(const D3D9DepthStencilState& lhs, const D3D9DepthStencilState& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO( zEnable_ );
    if (lhs.zEnable_)
    {
        LLGL_COMPARE_MEMBER_SWO( zWriteEnable_ );
        LLGL_COMPARE_MEMBER_SWO( zFunc_ );
    }

    LLGL_COMPARE_BOOL_MEMBER_SWO( stencilEnable_ );
    if (lhs.stencilEnable_)
    {
        LLGL_COMPARE_BOOL_MEMBER_SWO( twoSidedStencilMode_ );

        LLGL_COMPARE_MEMBER_SWO( stencilRef_ );
        LLGL_COMPARE_MEMBER_SWO( stencilMask_ );
        LLGL_COMPARE_MEMBER_SWO( stencilWriteMask_ );

        {
            int order = D3D9StencilFaceState::CompareSWO(lhs.stencilFront_, rhs.stencilFront_);
            if (order != 0)
                return order;
        }

        if (lhs.twoSidedStencilMode_)
        {
            int order = D3D9StencilFaceState::CompareSWO(lhs.stencilBack_, rhs.stencilBack_);
            if (order != 0)
                return order;
        }
    }

    return 0;
}


/*
 * D3D9StencilFaceState structure
 */

int D3D9DepthStencilState::D3D9StencilFaceState::CompareSWO(const D3D9StencilFaceState& lhs, const D3D9StencilFaceState& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO( opFail      );
    LLGL_COMPARE_BOOL_MEMBER_SWO( opZFail     );
    LLGL_COMPARE_BOOL_MEMBER_SWO( opPass      );
    LLGL_COMPARE_BOOL_MEMBER_SWO( cmpFunc      );
    return 0;
}


} // /namespace LLGL



// ================================================================================
