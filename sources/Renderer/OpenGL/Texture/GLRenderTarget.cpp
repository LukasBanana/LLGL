/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../../CheckedCast.h"
#include "../GLTypes.h"


namespace LLGL
{


void GLRenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTexture1D(NextColorAttachment(), textureGL, GL_TEXTURE_1D, mipLevel);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTexture2D(NextColorAttachment(), textureGL, GL_TEXTURE_2D, mipLevel);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTexture3D(NextColorAttachment(), textureGL, GL_TEXTURE_3D, mipLevel, layer);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTexture2D(NextColorAttachment(), textureGL, GLTypes::Map(cubeFace), mipLevel);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer);
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind();
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer * 6 + static_cast<int>(cubeFace));
    }
    frameBuffer_.Unbind();
}

void GLRenderTarget::DetachTextures()
{
    //todo...
}


/*
 * ======= Private: =======
 */

GLenum GLRenderTarget::NextColorAttachment()
{
    return (GL_COLOR_ATTACHMENT0 + (attachments_++));
}


} // /namespace LLGL



// ================================================================================
