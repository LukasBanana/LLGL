/*
 * Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_H
#define LLGL_TEXTURE_H


#include "Export.h"
#include "Types.h"
#include "Image.h"
#include "TextureFlags.h"
#include <Gauss/Vector3.h>
#include <cstdint>


namespace LLGL
{


//! Texture interface.
class LLGL_EXPORT Texture
{

    public:

        Texture(const Texture&) = delete;
        Texture& operator = (const Texture&) = delete;

        virtual ~Texture();

        //! Returns the type of this texture.
        inline TextureType GetType() const
        {
            return type_;
        }

        /**
        \brief Returns the texture size for the specified MIP-level.
        \param[in] mipLevel Specifies the MIP-map level to querey from. The first and largest MIP-map is level zero.
        If this level is greater than or equal to the number of MIP-maps this texture has, the return value is undefined (i.e. depends on the render system).
        \remarks This function is guaranteed to keep the currently bound textures, i.e. all previously set textures using the 'CommandBuffer::SetTexture' function will remain.
        \see RenderContext::GenerateMips
        \see CommandBuffer::SetTexture
        */
        virtual Extent3D QueryMipLevelSize(std::uint32_t mipLevel) const = 0;

        /**
        \brief Queries a descriptor of this texture (including its type and size).
        \remarks This function is guaranteed to keep the currently bound textures, i.e. all previously set textures using the 'CommandBuffer::SetTexture' function will remain.
        \see TextureDescriptor
        */
        virtual TextureDescriptor QueryDesc() const = 0;

    protected:

        Texture(const TextureType type);

    private:

        TextureType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
