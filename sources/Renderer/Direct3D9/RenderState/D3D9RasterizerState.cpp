/*
 * D3D9RasterizerState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9RasterizerState.h"
#include "../D3D9Types.h"
#include "../../../Core/MacroUtils.h"
#include "D3D9StateManager.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


static DWORD FloatToDWORD(float value)
{
    return *reinterpret_cast<const DWORD*>(&(value));
}

D3D9RasterizerState::D3D9RasterizerState(const RasterizerDescriptor& desc)
{
    scissorTestEnable_      = desc.scissorTestEnabled ? TRUE : FALSE;
    cullMode_               = D3D9Types::ToD3DCull(desc.cullMode);
    fillMode_               = D3D9Types::ToD3DFillMode(desc.polygonMode);
    depthBias_              = FloatToDWORD(desc.depthBias.constantFactor);
    slopeScaleDepthBias_    = FloatToDWORD(desc.depthBias.slopeFactor);
}

void D3D9RasterizerState::Bind(D3D9StateManager& stateMngr)
{
    stateMngr.SetRenderState(D3DRS_SCISSORTESTENABLE, scissorTestEnable_);
    stateMngr.SetRenderState(D3DRS_CULLMODE, cullMode_);
    stateMngr.SetRenderState(D3DRS_FILLMODE, fillMode_);
    stateMngr.SetRenderState(D3DRS_DEPTHBIAS, depthBias_);
    stateMngr.SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, slopeScaleDepthBias_);
}

int D3D9RasterizerState::CompareSWO(const D3D9RasterizerState& lhs, const D3D9RasterizerState& rhs)
{
    LLGL_COMPARE_BOOL_MEMBER_SWO( scissorTestEnable_ );

    LLGL_COMPARE_MEMBER_SWO( cullMode_            );
    LLGL_COMPARE_MEMBER_SWO( fillMode_            );
    LLGL_COMPARE_MEMBER_SWO( depthBias_           );
    LLGL_COMPARE_MEMBER_SWO( slopeScaleDepthBias_ );

    return 0;
}


} // /namespace LLGL



// ================================================================================
