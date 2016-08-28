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
#include <Gauss/Vector3.h>


namespace LLGL
{


//! Texture interface.
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

        /**
        \brief Returns the texture size for the specified MIP-level.
        \param[in] mipLevel Specifies the MIP-map level to querey from. The first and largest MIP-map is level zero.
        If this level is greater than or equal to the number of MIP-maps this texture has, the return value is undefined (i.e. depends on the render system).
        \see RenderContext::GenerateMips
        */
        virtual Gs::Vector3i QueryMipLevelSize(int mipLevel) const = 0;

    protected:

        inline void SetType(const TextureType type)
        {
            type_ = type;
        }

    private:

        TextureType type_ = TextureType::Undefined;

};


} // /namespace LLGL


#endif



// ================================================================================
