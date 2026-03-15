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
#include "../Shader/D3D9VertexShader.h"
#include "../Texture/D3D9EmulatedSampler.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>


namespace LLGL
{


D3D9StateManager::D3D9StateManager(IDirect3DDevice9* device) :
    device_               { device         },
    autoGenIndexBuffer16_ { D3DFMT_INDEX16 },
    autoGenIndexBuffer32_ { D3DFMT_INDEX32 }
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

//private
void D3D9StateManager::SetStreamSourceFreqInternal(UINT stream, UINT divider)
{
    LLGL_ASSERT(stream < maxNumStreams);
    if (inputAssembly_.streamSourceFreq.dividers[stream] != divider)
    {
        inputAssembly_.streamSourceFreq.dividers[stream] = divider;
        device_->SetStreamSourceFreq(stream, divider);
    }
}

void D3D9StateManager::SetStreamSource(UINT stream, const D3DStreamSource& source)
{
    LLGL_ASSERT(stream < maxNumStreams);
    D3DStreamSource& cacheEntry = inputAssembly_.streamSource[stream];
    if (cacheEntry.vertexBuffer != source.vertexBuffer  ||
        cacheEntry.offset       != source.offset        ||
        cacheEntry.stride       != source.stride)
    {
        cacheEntry = source;
        device_->SetStreamSource(stream, source.vertexBuffer, source.offset, source.stride);
    }
}

void D3D9StateManager::SetInstanceOffset(UINT firstInstance)
{
    if (firstInstance > 0)
    {
        UINT streamSourceUpperBound = 0;

        for_range(stream, inputAssembly_.streamSourceFreq.streamUpperBound)
        {
            if ((inputAssembly_.streamSourceFreq.dividers[stream] & D3DSTREAMSOURCE_INSTANCEDATA) != 0)
            {
                /* Map instance offset to byte offset */
                const UINT offset = inputAssembly_.streamSource[stream].stride * firstInstance;

                /* Modify stream source offset */
                D3DStreamSource& cacheEntry = inputAssembly_.streamSource[stream];
                if (cacheEntry.vertexBuffer != nullptr && cacheEntry.offset != offset)
                {
                    cacheEntry.offset = offset;
                    device_->SetStreamSource(stream, cacheEntry.vertexBuffer, cacheEntry.offset, cacheEntry.stride);

                    streamSourceUpperBound = std::max<UINT>(streamSourceUpperBound, stream + 1);
                }
            }
        }

        inputAssembly_.streamSourceOffsetUpperBound = streamSourceUpperBound;
    }
    else if (inputAssembly_.streamSourceOffsetUpperBound > 0)
    {
        /* Reset stream source offets to zero */
        for_range(stream, inputAssembly_.streamSourceOffsetUpperBound)
        {
            D3DStreamSource& cacheEntry = inputAssembly_.streamSource[stream];
            if (cacheEntry.vertexBuffer != nullptr && cacheEntry.offset != 0)
            {
                cacheEntry.offset = 0;
                device_->SetStreamSource(stream, cacheEntry.vertexBuffer, cacheEntry.offset, cacheEntry.stride);
            }
        }
        inputAssembly_.streamSourceOffsetUpperBound = 0;
    }
}

void D3D9StateManager::SetStreamSourceFreqIndexData(UINT numInstances)
{
    if (numInstances > 0)
        SetStreamSourceFreqInternal(0, D3DSTREAMSOURCE_INDEXEDDATA | numInstances);
    else
        SetStreamSourceFreqInternal(0, 1);
}

void D3D9StateManager::SetStreamSourceFreqInstanceData(UINT count, const D3D9StreamSourceFreq* streamSourceFreq)
{
    UINT newDividers[maxNumStreams] = {};

    /* Initialize new diviers with reset value from previous upper bound */
    for_range(stream, inputAssembly_.streamSourceFreq.streamUpperBound)
    {
        if ((inputAssembly_.streamSourceFreq.dividers[stream] & D3DSTREAMSOURCE_INSTANCEDATA) != 0)
            newDividers[stream] = 1;
    }

    /* Override new stream source frequencies */
    UINT streamUpperBound = 0;
    for_range(i, count)
    {
        newDividers[streamSourceFreq[i].stream] = streamSourceFreq[i].divider;
        streamUpperBound = std::max<UINT>(streamUpperBound, streamSourceFreq[i].stream + 1);
    }

    /* Set new diviers */
    for_range(stream, std::max<UINT>(inputAssembly_.streamSourceFreq.streamUpperBound, streamUpperBound))
    {
        if (newDividers[stream] != 0)
            SetStreamSourceFreqInternal(stream, newDividers[stream]);
    }

    /* Cache new upper bound */
    inputAssembly_.streamSourceFreq.streamUpperBound = streamUpperBound;
}

void D3D9StateManager::SetIndices(IDirect3DIndexBuffer9* indexBuffer)
{
    if (inputAssembly_.indexBuffer != indexBuffer)
    {
        inputAssembly_.indexBuffer = indexBuffer;
        device_->SetIndices(indexBuffer);
    }
}

void D3D9StateManager::SetAutoIndices(UINT numIndices)
{
    if (IDirect3DIndexBuffer9* indexBuffer16 = autoGenIndexBuffer16_.GetNativeForMaxIndex(device_.Get(), numIndices))
        SetIndices(indexBuffer16);
    else if (IDirect3DIndexBuffer9* indexBuffer32 = autoGenIndexBuffer32_.GetNativeForMaxIndex(device_.Get(), numIndices))
        SetIndices(indexBuffer32);
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
