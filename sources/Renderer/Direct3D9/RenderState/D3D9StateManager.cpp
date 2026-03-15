/*
 * D3D9StateManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9StateManager.h"
#include "D3D9DepthStencilState.h"
#include "D3D9RasterizerState.h"
#include "D3D9BlendState.h"
#include "../Texture/D3D9EmulatedSampler.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
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

void D3D9StateManager::SetSamplerState(DWORD stage, const D3D9SamplerState& d3dState)
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

void D3D9StateManager::BindTexture(DWORD stage, IDirect3DBaseTexture9* texture)
{
    LLGL_ASSERT(stage < D3D9StateManager::numTextureStages);
    if (textureStages_[stage].boundD3DBaseTexture != texture)
    {
        device_->SetTexture(stage, texture);
        textureStages_[stage].boundD3DBaseTexture = texture;
    }
}

void D3D9StateManager::BindSampler(DWORD stage, const D3D9EmulatedSampler* sampler)
{
    LLGL_ASSERT(stage < D3D9StateManager::numTextureStages);
    LLGL_ASSERT_PTR(sampler);
    if (textureStages_[stage].boundSampler != sampler)
    {
        SetSamplerState(stage, sampler->GetD3DState());
        textureStages_[stage].boundSampler = sampler;
    }
}

void D3D9StateManager::BindDepthStencilState(D3D9DepthStencilState* depthStencilState)
{
    if (boundDepthStencilState_ != depthStencilState)
    {
        boundDepthStencilState_ = depthStencilState;
        depthStencilState->Bind(*this);
    }
}

void D3D9StateManager::BindRasterizerState(D3D9RasterizerState* rasterizerState)
{
    if (boundRasterizerState_ != rasterizerState)
    {
        boundRasterizerState_ = rasterizerState;
        rasterizerState->Bind(*this);
    }
}

void D3D9StateManager::BindBlendState(D3D9BlendState* blendState)
{
    if (boundBlendState_ != blendState)
    {
        boundBlendState_ = blendState;
        blendState->Bind(*this);
    }
}

void D3D9StateManager::SetRenderTargets(UINT numColorTargets, IDirect3DSurface9* const * renderTargets, IDirect3DSurface9* depthStencil)
{
    /* Bind new color targets and unbind previous targets that are now inactive */
    for_range(i, numColorTargets)
        device_->SetRenderTarget(i, renderTargets[i]);
    for_subrange(i, numColorTargets, numColorTargets_)
        device_->SetRenderTarget(i, nullptr);
    numColorTargets_ = numColorTargets;

    /* Bind depth-stencil target */
    device_->SetDepthStencilSurface(depthStencil);

    /* Determine new clear mask */
    clearMask_ = 0;
    if (numColorTargets > 0)
        clearMask_ |= D3DCLEAR_TARGET;
    if (depthStencil != nullptr)
        clearMask_ |= (D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER);
}

void D3D9StateManager::Clear(DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
    const DWORD clearFlags = (flags & clearMask_);
    if (clearFlags != 0)
        device_->Clear(0, nullptr, clearFlags, color, z, stencil);
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
