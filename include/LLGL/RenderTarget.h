/*
 * RenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_RENDER_TARGET_H__
#define __LLGL_RENDER_TARGET_H__


#include "Export.h"
#include "TextureFlags.h"


namespace LLGL
{


class Texture;

//! Render target interface.
class LLGL_EXPORT RenderTarget
{

    public:

        virtual ~RenderTarget()
        {
        }

        virtual void AttachTexture1D(Texture& texture, int mipLevel = 0) = 0;
        virtual void AttachTexture2D(Texture& texture, int mipLevel = 0) = 0;
        virtual void AttachTexture3D(Texture& texture, int layer, int mipLevel = 0) = 0;
        virtual void AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel = 0) = 0;
        virtual void AttachTexture1DArray(Texture& texture, int layer, int mipLevel = 0) = 0;
        virtual void AttachTexture2DArray(Texture& texture, int layer, int mipLevel = 0) = 0;
        virtual void AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel = 0) = 0;

        //! Detaches all textures from this render target.
        virtual void DetachTextures() = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
