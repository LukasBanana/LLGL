/*
 * Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_H
#define LLGL_TEXTURE_H


#include "Resource.h"
#include "Types.h"
#include "Image.h"
#include "TextureFlags.h"
#include <cstdint>


namespace LLGL
{


/**
\brief Texture interface.
\see RenderSystem::CreateTexture
*/
class LLGL_EXPORT Texture : public Resource
{

    public:

        //! Returns ResourceType::Texture.
        ResourceType QueryResourceType() const override;

        //! Returns the type of this texture.
        inline TextureType GetType() const
        {
            return type_;
        }

        /**
        \brief Queries a descriptor of this texture (including its type and size).
        \remarks This function is guaranteed to keep the currently bound textures, i.e. all previously bounds textures (e.g. using the CommandBufferExt::SetTexture function) will remain.
        \see TextureDescriptor
        */
        virtual TextureDescriptor QueryDesc() const = 0;

        /**
        \brief Returns the texture extent for the specified MIP-level. This also includes the number of array layers.

        For a 1D array texture, the number of array layers is stored in the height extent.
        For a 2D array texture, the number of array layers is stored in the depth extent.
        \param[in] mipLevel Specifies the MIP-map level to query from. The first and largest MIP-map is level zero.
        If this level is greater than or equal to the number of MIP-maps this texture has, the return value is undefined (i.e. depends on the render system).
        \remarks This function is guaranteed to keep the currently bound textures, i.e. all previously bounds textures (e.g. using the CommandBufferExt::SetTexture function) will remain.
        \note For cube array textures (i.e. texture type TextureType::TextureCubeArray), the depth extent will be a multiple of 6.
        This is in contrast to the other handling of cube array layers, because this function determines the actual buffer extent of the hardware texture.
        \see RenderSystem::GenerateMips
        \see CommandBufferExt::SetTexture
        \todo Rename to "QueryExtent"
        */
        virtual Extent3D QueryMipLevelSize(std::uint32_t mipLevel) const = 0;

    protected:

        Texture(const TextureType type);

    private:

        TextureType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
