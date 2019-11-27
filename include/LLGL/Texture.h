/*
 * Texture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_H
#define LLGL_TEXTURE_H


#include "Resource.h"
#include "Types.h"
#include "TextureFlags.h"
#include <cstdint>


namespace LLGL
{


/**
\brief Texture interface.
\see RenderSystem::CreateTexture
\see TextureDescriptor
\see TextureLocation
\see TextureRegion
*/
class LLGL_EXPORT Texture : public Resource
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Texture );

    public:

        //! Returns ResourceType::Texture.
        ResourceType GetResourceType() const override final;

        //! Returns the type of this texture.
        inline TextureType GetType() const
        {
            return type_;
        }

        /**
        \brief Queries a descriptor of this texture.
        \remarks This function only queries the following attributes:
        - \c type
        - \c format
        - \c extent
        - \c arrayLayers
        - \c mipLevels
        - \c samples
        \remarks All other attributes (such as \c bindFlags, \c miscFlags etc.) cannot be queried by this function.
        Those attributes are either set to zero (for flags) or the default value specified in TextureDescriptor is used.
        \see TextureDescriptor
        \see Buffer::GetDesc
        */
        virtual TextureDescriptor GetDesc() const = 0;

        /**
        \brief Returns the texture extent for the specified MIP-level. This also includes the number of array layers.
        \param[in] mipLevel Specifies the MIP-map level to query from. The first and largest MIP-map is level zero.
        If this level is greater than or equal to the maxmimum number of MIP-maps for this texture, the return value is undefined (i.e. depends on the render system).
        \remarks For a 1D array texture, the number of array layers is stored in the height extent.
        \remarks For a 2D and cube array texture, the number of array layers is stored in the depth extent.
        \remarks For cube textures and cube array textures (i.e. texture type TextureType::TextureCube and TextureType::TextureCubeArray), the depth extent will be a multiple of 6.
        This is in contrast to the other handling of cube array layers, because this function determines the actual buffer extent of the hardware texture.
        \see TextureDescriptor::mipLevels
        \see CommandBuffer::GenerateMips
        \see NumMipDimensions
        \see LLGL::GetMipExtent
        */
        virtual Extent3D GetMipExtent(std::uint32_t mipLevel) const = 0;

    protected:

        Texture(const TextureType type);

    private:

        TextureType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
