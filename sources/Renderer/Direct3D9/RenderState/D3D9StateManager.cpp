/*
 * D3D9StateManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9StateManager.h"
#include "../Texture/D3D9EmulatedSampler.h"
#include "../../../Core/Assertion.h"
#include <string.h>


namespace LLGL
{


D3D9StateManager::D3D9StateManager(IDirect3DDevice9* device) :
    device_ { device }
{
    ::memset(renderStates_, 0xFFFFFFFF, sizeof(renderStates_));
    ::memset(textureStages_, 0xFFFFFFFF, sizeof(textureStages_));
    InitializeForFixedFunctionPipeline();
}

void D3D9StateManager::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    const DWORD stateIndex = state - 1;
    if (renderStates_[stateIndex] != value)
    {
        renderStates_[stateIndex] = value;
        device_->SetRenderState(state, value);
    }
}

void D3D9StateManager::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
    LLGL_ASSERT(stage < D3D9StateManager::numTextureStages);
    const DWORD stateIndex = static_cast<DWORD>(type) - 1;
    if (textureStages_[stage].stageStates[stateIndex] != value)
    {
        textureStages_[stage].stageStates[stateIndex] = value;
        device_->SetTextureStageState(stage, type, value);
    }
}

void D3D9StateManager::SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE type, DWORD value)
{
    LLGL_ASSERT(stage < D3D9StateManager::numTextureStages);
    SetSamplerStateInternal(stage, type, value);
}

void D3D9StateManager::SetSamplerStates(DWORD stage, const D3D9SamplerState& d3dState)
{
    SetSamplerStateInternal(stage, D3DSAMP_ADDRESSU,      d3dState.addressU     );
    SetSamplerStateInternal(stage, D3DSAMP_ADDRESSV,      d3dState.addressV     );
    SetSamplerStateInternal(stage, D3DSAMP_ADDRESSW,      d3dState.addressW     );
    SetSamplerStateInternal(stage, D3DSAMP_BORDERCOLOR,   d3dState.borderColor  );
    SetSamplerStateInternal(stage, D3DSAMP_MAGFILTER,     d3dState.magFilter    );
    SetSamplerStateInternal(stage, D3DSAMP_MINFILTER,     d3dState.minFilter    );
    SetSamplerStateInternal(stage, D3DSAMP_MIPFILTER,     d3dState.mipFilter    );
    SetSamplerStateInternal(stage, D3DSAMP_MIPMAPLODBIAS, d3dState.mipMapLodBias);
    SetSamplerStateInternal(stage, D3DSAMP_MAXMIPLEVEL,   d3dState.maxMipLevel  );
    SetSamplerStateInternal(stage, D3DSAMP_MAXANISOTROPY, d3dState.maxAnisotropy);
}

void D3D9StateManager::BindSampler(DWORD stage, const D3D9EmulatedSampler* sampler)
{
    LLGL_ASSERT(stage < D3D9StateManager::numTextureStages);
    LLGL_ASSERT_PTR(sampler);
    if (textureStages_[stage].boundSampler != sampler)
    {
        SetSamplerStates(stage, sampler->GetD3DState());
        textureStages_[stage].boundSampler = sampler;
    }
}


/*
 * ======= Private: =======
 */

void D3D9StateManager::SetSamplerStateInternal(DWORD stage, D3DSAMPLERSTATETYPE type, DWORD value)
{
    const DWORD stateIndex = static_cast<DWORD>(type) - 1;
    if (textureStages_[stage].samplerStates[stateIndex] != value)
    {
        textureStages_[stage].samplerStates[stateIndex] = value;
        device_->SetSamplerState(stage, type, value);
    }
}

void D3D9StateManager::InitializeForFixedFunctionPipeline()
{
    SetRenderState(D3DRS_LIGHTING, FALSE);
}


} // /namespace LLGL



// ================================================================================
