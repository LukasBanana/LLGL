/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_FLAGS_H
#define LLGL_TEXTURE_FLAGS_H


#include "Export.h"
#include "Types.h"
#include "Format.h"
#include "ResourceFlags.h"

#include <cstddef>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Texture type enumeration.
enum class TextureType
{
    Texture1D,          //!< 1-Dimensional texture.
    Texture2D,          //!< 2-Dimensional texture.
    Texture3D,          //!< 3-Dimensional texture.
    TextureCube,        //!< Cube texture.
    Texture1DArray,     //!< 1-Dimensional array texture.
    Texture2DArray,     //!< 2-Dimensional array texture.
    TextureCubeArray,   //!< Cube array texture.
    Texture2DMS,        //!< 2-Dimensional multi-sample texture.
    Texture2DMSArray,   //!< 2-Dimensional multi-sample array texture.
};

#if 0//TODO: currently unused
/**
\brief Texture component swizzle enumeration.
\remarks Can be used to change the order of texel components independently of a shader.
\see TextureSwizzleRGBA
*/
enum class TextureSwizzle
{
    Zero,   //!< The component is replaced by the constant zero.
    One,    //!< The component is replaced by the constant one.
    Red,    //!< The component is replaced by red component.
    Green,  //!< The component is replaced by green component.
    Blue,   //!< The component is replaced by blue component.
    Alpha   //!< The component is replaced by alpha component.
};
#endif


/* ----- Structures ----- */

#if 0//TODO: currently unused
/**
\brief Texture component swizzle structure for red, green, blue, and alpha components.
\remarks Can be used to change the order of texel components independently of a shader.
*/
struct TextureSwizzleRGBA
{
    TextureSwizzle r = TextureSwizzle::Red;     //!< Red component swizzle. By default TextureSwizzle::Red.
    TextureSwizzle g = TextureSwizzle::Green;   //!< Green component swizzle. By default TextureSwizzle::Green.
    TextureSwizzle b = TextureSwizzle::Blue;    //!< Blue component swizzle. By default TextureSwizzle::Blue.
    TextureSwizzle a = TextureSwizzle::Alpha;   //!< Alpha component swizzle. By default TextureSwizzle::Alpha.
};
#endif

/**
\brief Texture descriptor structure.
\remarks This is used to specifiy the dimensions of a texture which is to be created.
\see RenderSystem::CreateTexture
*/
struct TextureDescriptor
{
    //! Hardware texture type. By default TextureType::Texture1D.
    TextureType     type            = TextureType::Texture1D;

    /**
    \brief These flags describe to which resource slots and render target attachments the texture can be bound. By default BindFlags::SampleBuffer and BindFlags::ColorAttachment.
    \remarks When the texture will be bound as a color attachment to a render target for instance, the BindFlags::ColorAttachment flag is required.
    \see BindFlags
    */
    long            bindFlags       = (BindFlags::SampleBuffer | BindFlags::ColorAttachment);

    /**
    \brief CPU read/write access flags. By default 0.
    \remarks If this is 0 the texture cannot be mapped from GPU memory space into CPU memory space and vice versa.
    \see CPUAccessFlags
    \see RenderSystem::MapTexture
    \todo Not supported yet.
    */
    long            cpuAccessFlags  = 0;

    /**
    \brief Miscellaneous texture flags. By default MiscFlags::FixedSamples.
    \remarks This can be used as a hint for the renderer how frequently the texture will be updated, or whether a multi-sampled texture has fixed sample locations.
    \see MiscFlags
    */
    long            miscFlags       = MiscFlags::FixedSamples;

    //! Hardware texture format. By default Format::RGBA8UNorm.
    Format          format          = Format::RGBA8UNorm;

    /**
    \brief Size of the texture (excluding the number of array layers). By default (1, 1, 1).
    \remarks The \c height component is only used for 2D, 3D, and Cube textures (i.e. TextureType::Texture2D, TextureType::Texture2DArray, TextureType::Texture3D,
    TextureType::TextureCube, TextureType::TextureCubeArray, TextureType::Texture2DMS, and TextureType::Texture2DMSArray).
    \remarks The \c depth component is only used for 3D textures (i.e. TextureType::Texture3D).
    \remarks The \c width and \c height components must be equal for cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray).
    \see IsCubeTexture
    \see arrayLayers
    */
    Extent3D        extent          = { 1, 1, 1 };

    /**
    \brief Number of array layers. By default 1.
    \remarks This can be greater than 1 for array textures and cube textures (i.e. TextureType::Texture1DArray, TextureType::Texture2DArray,
    TextureType::TextureCube, TextureType::TextureCubeArray, TextureType::Texture2DMSArray).
    For cube textures, this must be a multiple of 6 (one array layer for each cube face).
    For all other texture types, this must be 1.
    The index offsets for each cube face are as follows:
    - <code>X+</code> direction has index offset 0.
    - <code>X-</code> direction has index offset 1.
    - <code>Y+</code> direction has index offset 2.
    - <code>Y-</code> direction has index offset 3.
    - <code>Z+</code> direction has index offset 4.
    - <code>Z-</code> direction has index offset 5.
    \see IsArrayTexture
    \see IsCubeTexture
    \see RenderingLimits::maxTextureArrayLayers
    \see extent
    */
    std::uint32_t   arrayLayers     = 1;

    /**
    \brief Number of MIP-map levels. By default 0.
    \remarks If this is 0, the full MIP-chain will be generated.
    If this is 1, no MIP-mapping is used for this texture and it has only a single MIP-map level.
    This field is ignored for multi-sampled textures (i.e. TextureType::Texture2DMS, TextureType::Texture2DMSArray),
    since these texture types only have a single MIP-map level.
    \see NumMipLevels
    \see RenderSystem::GenerateMips
    */
    std::uint32_t   mipLevels       = 0;

    /**
    \brief Number of samples per texel. By default 1.
    \remarks This is only used for multi-sampled textures (i.e. TextureType::Texture2DMS and TextureType::Texture2DMSArray).
    The equivalent member for graphics pipeline states is MultiSamplingDescriptor::samples.
    \see IsMultiSampleTexture
    */
    std::uint32_t   samples         = 1;
};

/**
\brief Texture region structure.
\remarks This is used to write (or partially write) the image data of a texture MIP-map level.
\see RenderSystem::WriteTexture
*/
struct TextureRegion
{
    //! MIP-map level for the sub-texture, where 0 is the base texture, and N > 0 is the N-th MIP-map level. By default 0.
    std::uint32_t   mipLevel    = 0;

    /**
    \brief Sub-texture offset. By default (0, 0, 0).
    \remarks For array textures, the Z component specifies the array layer.
    For cube textures, the Z component specifies the array layer and cube face offset (for 1D-array textures it's the Y component).
    The layer offset for the respective cube faces is described at the TextureDescriptor::arrayLayers member.
    Negative values of this member are not allowed and result in undefined behavior.
    */
    Offset3D        offset      = { 0, 0, 0 };

    /**
    \brief Sub-texture extent. By default (1, 1, 1).
    \remarks For array textures, the depth component specifies the number of array layers (for 1D-array textures it's the height component).
    For cube textures, the depth component specifies the number of array layers and cube faces (where each cube has 6 faces).
    */
    Extent3D        extent      = { 1, 1, 1 };
};


/* ----- Functions ----- */

/**
\defgroup group_tex_util Texture utility functions to determine texture dimension and buffer sizes.
\addtogroup group_tex_util
@{
*/

/**
\brief Returns the number of MIP-map levels for a texture with the specified size.
\param[in] width Specifies the texture width.
\param[in] height Specifies the texture height or number of layers for 1D array textures. By default 1 (if 1D textures are used).
\param[in] depth Specifies the texture depth or number of layers for 2D array textures. By default 1 (if 1D or 2D textures are used).
\remarks The height and depth are optional parameters, so this function can be easily used for 1D, 2D, and 3D textures.
\return 1 + floor(log2(max{ width, height, depth })).
*/
LLGL_EXPORT std::uint32_t NumMipLevels(std::uint32_t width, std::uint32_t height = 1, std::uint32_t depth = 1);

/**
\brief Returns the number of MIP-map levels for the specified texture descriptor.
\param[in] textureDesc Specifies the descriptor whose parameters are used to determine the number of MIP-map levels.
\remarks This function will deduce the number MIP-map levels automatically only if the member "mipLevels" is zero.
Otherwise, the value of this member is returned.
\see NumMipLevels(std::uint32_t, std::uint32_t, std::uint32_t)
*/
LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc);

/**
\brief Returns the required buffer size (in bytes) of a texture with the specified hardware format and number of texels.
\param[in] format Specifies the texture format.
\param[in] numTexels Specifies the number of texture elements (texels).
For the DXT compressed texture formats, this must be a multiple of 16, since these formats compress the image in 4x4 texel blocks.
\return The required buffer size (in bytes), or zero if the input is invalid.
\remarks The counterpart for image data is the function ImageDataSize.
\see ImageDataSize
*/
LLGL_EXPORT std::uint32_t TextureBufferSize(const Format format, std::uint32_t numTexels);

/**
\brief Returns the texture size (in texels) of the specified texture descriptor, or zero if the texture type is invalid.
\see TextureDescriptor::type
*/
LLGL_EXPORT std::uint32_t TextureSize(const TextureDescriptor& textureDesc);

/**
\brief Returns true if the specified texture descriptor describes a texture with MIP-mapping enabled.
\return True if the texture type is not a multi-sampled texture and the number of MIP-map levels in the descriptor is either zero or greater than one.
\see TextureDescriptor::mipLevels
*/
LLGL_EXPORT bool IsMipMappedTexture(const TextureDescriptor& textureDesc);

/**
\brief Returns true if the specified texture type is an array texture.
\return True if \c type is either TextureType::Texture1DArray, TextureType::Texture2DArray,
TextureType::TextureCubeArray, or TextureType::Texture2DMSArray.
*/
LLGL_EXPORT bool IsArrayTexture(const TextureType type);

/**
\brief Returns true if the specified texture type is a multi-sample texture.
\return True if \c type is either TextureType::Texture2DMS, or TextureType::Texture2DMSArray.
*/
LLGL_EXPORT bool IsMultiSampleTexture(const TextureType type);

/**
\brief Returns true if the specified texture type is a cube texture.
\return True if \c type is either TextureType::TextureCube or TextureType::TextureCubeArray.
*/
LLGL_EXPORT bool IsCubeTexture(const TextureType type);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
