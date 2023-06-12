/*
 * Texture.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TEXTURE_H
#define LLGL_TEXTURE_H


#include <LLGL/Resource.h>
#include <LLGL/Types.h>
#include <LLGL/TextureFlags.h>
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

        /*
        \brief Returns the binding flags this texture was created with.
        \see TextureDescriptor::bindFlags
        */
        inline long GetBindFlags() const
        {
            return bindFlags_;
        }

        /**
        \brief Queries a descriptor of this texture.
        \remarks This function only queries the following attributes:
        - \c type
        - \c bindFlags
        - \c format
        - \c extent
        - \c arrayLayers
        - \c mipLevels
        - \c samples
        \remarks All other attributes (i.e. \c miscFlags and \c clearValue) cannot be queried by this function.
        Those attributes are either set to zero (for flags) or the default value specified in TextureDescriptor is used.
        If only the texture format is required, use \c GetFormat instead.
        \see TextureDescriptor
        \see GetFormat
        \see Buffer::GetDesc
        */
        virtual TextureDescriptor GetDesc() const = 0;

        /**
        \brief Returns the hardware format of this texture.
        \remarks This is usually the format this texture was created with.
        However, sometimes the internal hardware format might be different from what the client programmer requested, especially with the OpenGL backend.
        This function returns the actual internal hardware format.
        \see TextureDescriptor::format.
        */
        virtual Format GetFormat() const = 0;

        /**
        \brief Returns the texture extent for the specified MIP-level. This also includes the number of array layers.
        \param[in] mipLevel Specifies the MIP-map level to query from. The first and largest MIP-map is level zero.
        If this level is greater than or equal to the maxmimum number of MIP-maps for this texture, the return value is undefined (i.e. depends on the render system).
        \remarks For a 1D array texture, the number of array layers is stored in the height extent.
        \remarks For a 2D and cube array texture, the number of array layers is stored in the depth extent.
        \remarks For cube textures and cube array textures (i.e. texture type TextureType::TextureCube and TextureType::TextureCubeArray), the depth extent will be a multiple of 6.
        \see TextureDescriptor::mipLevels
        \see CommandBuffer::GenerateMips
        \see NumMipDimensions
        \see LLGL::GetMipExtent
        */
        virtual Extent3D GetMipExtent(std::uint32_t mipLevel) const = 0;

        /**
        \brief Returns the memory footprint of the specified MIP-map subresource.
        \param[in] mipLevel Specifies the MIP-map level to query from. The first and largest MIP-map level is zero.
        \return Memory footprint of the specified subresource. If this operation failed, all fields in the returned structure are default initialized.
        \see GetMipExtent
        */
        virtual SubresourceFootprint GetSubresourceFootprint(std::uint32_t mipLevel) const = 0;

    protected:

        Texture(const TextureType type, long bindFlags);

    private:

        TextureType type_;
        long        bindFlags_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
