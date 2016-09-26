/*
 * D3D11RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderTarget.h"


namespace LLGL
{


D3D11RenderTarget::D3D11RenderTarget(ID3D11Device* device, unsigned int multiSamples)
{
}

void D3D11RenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
}

void D3D11RenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
}

void D3D11RenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
}

void D3D11RenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
}

void D3D11RenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
}

void D3D11RenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
}

void D3D11RenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
}

void D3D11RenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
}

void D3D11RenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
}

void D3D11RenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
}

void D3D11RenderTarget::DetachTextures()
{
}

/* ----- Extended Internal Functions ----- */

//...


/*
 * ======= Private: =======
 */

//...


} // /namespace LLGL



// ================================================================================
