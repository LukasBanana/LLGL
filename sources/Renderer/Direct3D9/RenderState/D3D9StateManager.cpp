/*
 * D3D9StateManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9StateManager.h"
#include "../../../Core/Assertion.h"
#include <string.h>


namespace LLGL
{


D3D9StateManager::D3D9StateManager(IDirect3DDevice9* device) :
    device_ { device }
{
    ::memset(renderStates_, 0xFFFFFFFF, sizeof(renderStates_));
    ::memset(textureStages_, 0xFFFFFFFF, sizeof(textureStages_));
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
    const DWORD stateIndex = type - 1;
    if (textureStages_[stage].stageStates[stateIndex] != value)
    {
        textureStages_[stage].stageStates[stateIndex] = value;
        device_->SetTextureStageState(stage, type, value);
    }
}


} // /namespace LLGL



// ================================================================================
