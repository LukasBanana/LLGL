/*
 * Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE_H__
#define __LLGL_TEXTURE_H__


#include "Export.h"
#include "TextureFlags.h"


namespace LLGL
{


//! Texture base interface.
class LLGL_EXPORT Texture
{

    public:

        virtual ~Texture()
        {
        }

        /**
        \brief Returns the texture type. This type can only be changed by the render system.
        \see RenderSystem::WriteTexture1D
        \see RenderSystem::WriteTexture2D
        \see RenderSystem::WriteTexture3D
        \see RenderSystem::WriteTextureCube
        */
        inline TextureType GetType() const
        {
            return type_;
        }

    protected:

        inline void SetType(TextureType type)
        {
            type_ = type;
        }

    private:

        TextureType type_ = TextureType::Undefined;

};


} // /namespace LLGL


#endif



// ================================================================================
