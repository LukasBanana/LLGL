/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_RENDER_TARGET_H__
#define __LLGL_DBG_RENDER_TARGET_H__


#include <LLGL/RenderTarget.h>


namespace LLGL
{


class DbgRenderTarget : public RenderTarget
{

    public:

        DbgRenderTarget(RenderTarget& instance, unsigned int multiSamples);

        void AttachDepthBuffer(const Gs::Vector2i& size) override;
        void AttachStencilBuffer(const Gs::Vector2i& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2i& size) override;

        void AttachTexture1D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture2D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture3D(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel = 0) override;
        void AttachTexture1DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTexture2DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel = 0) override;

        void DetachTextures() override;

        RenderTarget& instance;

};


} // /namespace LLGL


#endif



// ================================================================================
