/*
 * DbgRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderTarget.h"
#include "DbgTexture.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, unsigned int multiSamples) :
    instance( instance )
{
}

void DbgRenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    instance.AttachDepthBuffer(size);
}

void DbgRenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
    instance.AttachStencilBuffer(size);
}

void DbgRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    instance.AttachDepthStencilBuffer(size);
}

void DbgRenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture1D(textureDbg.instance, mipLevel);
}

void DbgRenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture2D(textureDbg.instance, mipLevel);
}

void DbgRenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture3D(textureDbg.instance, layer, mipLevel);
}

void DbgRenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTextureCube(textureDbg.instance, cubeFace, mipLevel);
}

void DbgRenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture1DArray(textureDbg.instance, layer, mipLevel);
}

void DbgRenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture2DArray(textureDbg.instance, layer, mipLevel);
}

void DbgRenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTextureCubeArray(textureDbg.instance, layer, cubeFace, mipLevel);
}

void DbgRenderTarget::DetachAll()
{
    instance.DetachAll();
}


} // /namespace LLGL



// ================================================================================
