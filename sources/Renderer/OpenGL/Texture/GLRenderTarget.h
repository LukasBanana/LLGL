/*
 * GLRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_TARGET_H__
#define __LLGL_GL_RENDER_TARGET_H__


#include <LLGL/RenderTarget.h>
#include "GLTexture.h"


namespace LLGL
{


class GLRenderTarget : public RenderTarget
{

    public:

        void AttachTexture1D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture2D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture3D(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel = 0) override;
        void AttachTexture1DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTexture2DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel = 0) override;

        void DetachTextures() override;

    private:



};


} // /namespace LLGL


#endif



// ================================================================================
