/*
 * D3D11RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderTarget.h"
#include "../D3D11RenderSystem.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D11RenderTarget::D3D11RenderTarget(D3D11RenderSystem& renderSystem, unsigned int multiSamples) :
    renderSystem_( renderSystem ),
    multiSamples_( multiSamples )
{
}

void D3D11RenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    CreateDepthStencilAndDSV(size, DXGI_FORMAT_D24_UNORM_S8_UINT);
}

void D3D11RenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    //todo...
}

void D3D11RenderTarget::DetachTextures()
{
    /* Reset all ComPtr instances */
    ResetResolution();

    renderTargetViews_.clear();
    renderTargetViewsRef_.clear();

    depthStencil_.Reset();
    depthStencilView_.Reset();
}

/* ----- Extended Internal Functions ----- */

//...


/*
 * ======= Private: =======
 */

void D3D11RenderTarget::CreateDepthStencilAndDSV(const Gs::Vector2i& size, DXGI_FORMAT format)
{
    /* Apply size to render target resolution, and create depth-stencil */
    ApplyResolution(size);

    renderSystem_.CreateDXDepthStencilAndDSV(
        static_cast<UINT>(size.x), static_cast<UINT>(size.y), multiSamples_, format,
        depthStencil_, depthStencilView_
    );
}


} // /namespace LLGL



// ================================================================================
