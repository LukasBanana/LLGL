/*
 * TextureFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TEXTURE_FLAGS_H
#define LLGL_TEXTURE_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Types.h>
#include <LLGL/Format.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include <cstddef>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Texture type enumeration.
enum class TextureType
{
    /**
    \brief 1-Dimensional texture.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12, Metal.
    */
    Texture1D,

    //! 2-Dimensional texture.
    Texture2D,

    //! 3-Dimensional texture.
    Texture3D,

    /**
    \brief Cube texture.
    \see TextureDescriptor::arrayLayers
    */
    TextureCube,

    /**
    \brief 1-Dimensional texture.
    \note Only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12, Metal.
    \see TextureDescriptor::arrayLayers
    */
    Texture1DArray,

    /**
    \brief 2-Dimensional array texture.
    \see TextureDescriptor::arrayLayers
    */
    Texture2DArray,

    /**
    \brief Cube array texture.
    \see TextureDescriptor::arrayLayers
    \note Only supported with: OpenGL, OpenGLES 3.2, Vulkan, Direct3D 11, Direct3D 12, Metal.
    */
    TextureCubeArray,

    /**
    \brief 2-Dimensional multi-sample texture.
    \note Only supported with: OpenGL, OpenGLES 3.1, Vulkan, Direct3D 11, Direct3D 12, Metal.
    \see TextureDescriptor::samples
    */
    Texture2DMS,

    /**
    \brief 2-Dimensional multi-sample array texture.
    \note Only supported with: OpenGL, OpenGLES 3.2, Vulkan, Direct3D 11, Direct3D 12, Metal.
    \see TextureDescriptor::samples
    */
    Texture2DMSArray,
};

/**
\brief Texture component swizzle enumeration.
\remarks Can be used to change the order of texel components independently of a shader.
\see TextureSwizzleRGBA
*/
enum class TextureSwizzle : std::uint8_t
{
    Zero,   //!< The component is replaced by the constant zero.
    One,    //!< The component is replaced by the constant one.
    Red,    //!< The component is replaced by red component.
    Green,  //!< The component is replaced by green component.
    Blue,   //!< The component is replaced by blue component.
    Alpha   //!< The component is replaced by alpha component.
};


/* ----- Structures ----- */

/**
\brief Texture component swizzle structure for red, green, blue, and alpha components.
\remarks Can be used to change the order of texel components independently of a shader.
\see TextureViewDescriptor::swizzle
*/
struct TextureSwizzleRGBA
{
    TextureSwizzle r = TextureSwizzle::Red;     //!< Red component swizzle. By default TextureSwizzle::Red.
    TextureSwizzle g = TextureSwizzle::Green;   //!< Green component swizzle. By default TextureSwizzle::Green.
    TextureSwizzle b = TextureSwizzle::Blue;    //!< Blue component swizzle. By default TextureSwizzle::Blue.
    TextureSwizzle a = TextureSwizzle::Alpha;   //!< Alpha component swizzle. By default TextureSwizzle::Alpha.
};

/**
\brief Texture subresource descriptor which specifies the array layer and MIP-map level range of a texture resource.
\remarks The default values refer to the first array layer and the first MIP-map level.
\see TextureRegion::subresource
\see TextureViewDescriptor::subresource
*/
struct TextureSubresource
{
    TextureSubresource() = default;
    TextureSubresource(const TextureSubresource&) = default;

    //! Constructor to initialize base MIP-map level and base array layer only.
    inline TextureSubresource(std::uint32_t baseArrayLayer, std::uint32_t baseMipLevel) :
        baseArrayLayer { baseArrayLayer },
        baseMipLevel   { baseMipLevel   }
    {
    }

    //! Constructor to initialize all attributes.
    inline TextureSubresource(std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, std::uint32_t baseMipLevel, std::uint32_t numMipLevels) :
        baseArrayLayer { baseArrayLayer },
        numArrayLayers { numArrayLayers },
        baseMipLevel   { baseMipLevel   },
        numMipLevels   { numMipLevels   }
    {
    }

    /**
    \brief Zero-based index of the first array layer. By default 0.
    \remarks Only used by array texture types (i.e. TextureType::Texture1DArray, TextureType::Texture2DArray, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray).
    \remarks This field is ignored by all other texture types.
    \see TextureDescriptor::arrayLayers
    */
    std::uint32_t   baseArrayLayer  = 0;

    /**
    \brief Number of array layers. By default 1.
    \remarks \b Must be greater than zero.
    \see TextureDescriptor::arrayLayers
    */
    std::uint32_t   numArrayLayers  = 1;

    /**
    \brief MIP-map level for the sub-texture, where 0 is the base texture, and N > 0 is the N-th MIP-map level. By default 0.
    \see TextureDescriptor::mipLevels
    */
    std::uint32_t   baseMipLevel    = 0;

    /**
    \brief Number of MIP-map levels. By default 1.
    \remarks \b Must be greater than zero.
    \see TextureDescriptor::mipLevels
    */
    std::uint32_t   numMipLevels    = 1;
};

/**
\brief Texture location structure: MIP-map level and offset.
\remarks This is used to specifiy the source and destination location of a texture copy operation.
\see CommandBuffer::CopyTexture
\see TextureRegion
*/
struct TextureLocation
{
    TextureLocation() = default;
    TextureLocation(const TextureLocation&) = default;

    //! Constructor to initialize all attributes.
    inline TextureLocation(const Offset3D& offset, std::uint32_t arrayLayer = 0, std::uint32_t mipLevel = 0) :
        offset     { offset     },
        arrayLayer { arrayLayer },
        mipLevel   { mipLevel   }
    {
    }

    /**
    \brief Zero-based offset within the texture data.
    \remarks Any component of this field that is not meant for the respective texture type is ignored.
    All other components must be greater than or equal to zero.
    */
    Offset3D        offset;

    /**
    \brief Zero-based array layer index.
    \remarks Only used by array texture types (i.e. TextureType::Texture1DArray, TextureType::Texture2DArray, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray).
    \remarks This field is ignored by all other texture types.
    \see TextureDescriptor::arrayLayers
    */
    std::uint32_t   arrayLayer  = 0;

    /**
    \brief MIP-map level for the sub-texture, where 0 is the base texture, and N > 0 is the N-th MIP-map level. By default 0.
    \see TextureDescriptor::mipLevels
    */
    std::uint32_t   mipLevel    = 0;
};

/**
\brief Texture region structure: Subresource (MIP-map level and array layer range), offset, and extent.
\remarks This is used to write (or partially write) and read (or partially read) the image data of a \b single texture MIP-map level.
\see RenderSystem::WriteTexture
\see RenderSystem::ReadTexture
\see CommandBuffer::CopyBufferFromTexture
\see CommandBuffer::CopyTextureFromBuffer
\see TextureLocation
*/
struct TextureRegion
{
    TextureRegion() = default;
    TextureRegion(const TextureRegion&) = default;

    //! Constructor to initialize offset and extent only.
    inline TextureRegion(const Offset3D& offset, const Extent3D& extent) :
        offset { offset },
        extent { extent }
    {
    }

    //! Constructor to initialize all members.
    inline TextureRegion(const TextureSubresource& subresource, const Offset3D& offset, const Extent3D& extent) :
        subresource { subresource },
        offset      { offset      },
        extent      { extent      }
    {
    }

    /**
    \brief Specifies the texture subresource, i.e. MIP-map level and array layer range. By default only the first MIP-map level and first array layer is addressed.
    \remarks For texture regions, \c numMipLevels of the TextureSubresource structure must always be 1, i.e. texture regions can only select a single MIP-map at a time.
    \see TextureSubresource::numMipLevels
    */
    TextureSubresource  subresource;

    /**
    \brief Zero-based offset within the texture data.
    \remarks Any component of this field that is not meant for the respective texture type is ignored.
    All other components must be greater than or equal to zero.
    */
    Offset3D            offset;

    /**
    \brief Extent of the sub texture region.
    \remarks All components of the extent must be greater than zero. By default (0, 0, 0).
    \see TextureDescriptor::extent
    */
    Extent3D            extent;
};

/**
\brief Texture descriptor structure.
\remarks Contains all information about type, format, and dimension to create a texture resource.
\see RenderSystem::CreateTexture
*/
struct TextureDescriptor
{
    //! Hardware texture type. By default TextureType::Texture2D.
    TextureType     type            = TextureType::Texture2D;

    /**
    \brief These flags describe to which resource slots and render target attachments the texture can be bound. By default BindFlags::Sampled and BindFlags::ColorAttachment.
    \remarks When the texture will be bound as a color attachment to a render target for instance, the BindFlags::ColorAttachment flag is required.
    \see BindFlags
    */
    long            bindFlags       = (BindFlags::Sampled | BindFlags::ColorAttachment);

    /**
    \brief Miscellaneous texture flags. By default MiscFlags::FixedSamples and MiscFlags::GenerateMips.
    \remarks This can be used as a hint for the renderer how frequently the texture will be updated, or whether a multi-sampled texture has fixed sample locations.
    \see MiscFlags
    */
    long            miscFlags       = (MiscFlags::FixedSamples | MiscFlags::GenerateMips);

    /**
    \brief Hardware texture format. By default Format::RGBA8UNorm.
    \see RenderingCapabilities::textureFormats
    */
    Format          format          = Format::RGBA8UNorm;

    /**
    \brief Size of the texture (excluding the number of array layers). By default (1, 1, 1).
    \remarks The \c height component is only used for 2D, 3D, and Cube textures (i.e. TextureType::Texture2D, TextureType::Texture2DArray, TextureType::Texture3D,
    TextureType::TextureCube, TextureType::TextureCubeArray, TextureType::Texture2DMS, and TextureType::Texture2DMSArray).
    \remarks The \c depth component is only used for 3D textures (i.e. TextureType::Texture3D).
    \remarks The \c width and \c height components must be equal for cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray).
    \see IsCubeTexture
    */
    Extent3D        extent          = { 1, 1, 1 };

    /**
    \brief Number of array layers. By default 1.
    \remarks For array textures and cube textures (i.e. TextureType::Texture1DArray, TextureType::Texture2DArray,
    TextureType::TextureCube, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray), this \b must be greater than or equal to 1.
    \remarks For cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray), this \b must be a multiple of 6 and greater than zero (one array layer for each cube face).
    \remarks For all other texture types, this \b must be 1.
    \remarks The index offsets for each cube face are as follows:
    - <code>X+</code> direction has index offset 0.
    - <code>X-</code> direction has index offset 1.
    - <code>Y+</code> direction has index offset 2.
    - <code>Y-</code> direction has index offset 3.
    - <code>Z+</code> direction has index offset 4.
    - <code>Z-</code> direction has index offset 5.
    \see IsArrayTexture
    \see IsCubeTexture
    \see RenderingLimits::maxTextureArrayLayers
    */
    std::uint32_t   arrayLayers     = 1;

    /**
    \brief Number of MIP-map levels. By default 0.
    \remarks If this is 0, the number of MIP-map levels will be determined automatically by the texture type and extent for a full MIP-chain.
    \remarks If this is 1, no MIP-mapping is used for this texture and it has only a single MIP-map level.
    \remarks For multi-sampled textures (i.e. TextureType::Texture2DMS, TextureType::Texture2DMSArray), this value must be either 0 or 1,
    whereas 0 specifies a full MIP-chain which implies 1 for multi-sampled textures.
    \see NumMipLevels
    \see CommandBuffer::GenerateMips
    */
    std::uint32_t   mipLevels       = 0;

    /**
    \brief Number of samples per texel. By default 1.
    \remarks This is only used for multi-sampled textures (i.e. TextureType::Texture2DMS and TextureType::Texture2DMSArray).
    \see IsMultiSampleTexture
    \see RenderingLimits::maxColorBufferSamples
    \see RenderingLimits::maxDepthBufferSamples
    \see RenderingLimits::maxStencilBufferSamples
    */
    std::uint32_t   samples         = 1;

    /**
    \brief Specifies a clear value to initialize the texture with, if no initial image data is provided.
    \remarks The initial texture data is only determined by this attribute if the \c imageDesc parameter of RenderSystem::CreateTexture is null
    and the MiscFlags::NoInitialData bit is \b not set in the \c miscFlags attribute.
    In either case, this value may be used by the renderer API as a hint which clear value the resource is optimized for (especially for Direct3D 12).
    \see RenderSystem::CreateTexture
    \see TextureDescriptor::miscFlags
    */
    ClearValue      clearValue;
};

/**
\brief Texture view descriptor structure.
\remarks Contains all information about type, format, and dimension to create a texture view that shares the image data of another texture resource.
\see ResourceViewDescriptor::textureView
\see RenderingFeatures::hasTextureViews
*/
struct TextureViewDescriptor
{
    /**
    \brief Hardware texture type. By default TextureType::Texture2D.
    \remarks The types of a shared texture can be mapped to the following type of texture-views:
    | Shared texture type | Compatible texture view types |
    |---------------------|-------------------------------|
    | TextureType::Texture1D | TextureType::Texture1D, TextureType::Texture1DArray |
    | TextureType::Texture2D | TextureType::Texture2D, TextureType::Texture2DArray |
    | TextureType::Texture3D | TextureType::Texture3D |
    | TextureType::TextureCube | TextureType::Texture2D, TextureType::Texture2DArray, TextureType::TextureCube, TextureType::TextureCubeArray |
    | TextureType::Texture1DArray | TextureType::Texture1D, TextureType::Texture1DArray |
    | TextureType::Texture2DArray | TextureType::Texture2D, TextureType::Texture2DArray |
    | TextureType::TextureCubeArray | TextureType::Texture2D, TextureType::Texture2DArray, TextureType::TextureCube, TextureType::TextureCubeArray |
    | TextureType::Texture2DMS | TextureType::Texture2DMS, TextureType::Texture2DMSArray |
    | TextureType::Texture2DMSArray | TextureType::Texture2DMS, TextureType::Texture2DMSArray |
    */
    TextureType         type            = TextureType::Texture2D;

    /**
    \brief Hardware texture format. By default Format::RGBA8UNorm.
    \remarks The format of the shared texture and the texture-view must be in the same format class:
    | Class | Compatible texture formats |
    |------:|----------------------------|
    | 128 Bits | Format::RGBA32UInt, Format::RGBA32SInt, Format::RGBA32Float |
    | 96 Bits | Format::RGB32UInt, Format::RGB32SInt, Format::RGB32Float |
    | 64 Bits | Format::RG32UInt, Format::RG32SInt, Format::RG32Float, Format::RGBA16UNorm, Format::RGBA16SNorm, Format::RGBA16UInt, Format::RGBA16SInt, Format::RGBA16Float |
    | 48 Bits | Format::RGB16UNorm, Format::RGB16SNorm, Format::RGB16UInt, Format::RGB16SInt, Format::RGB16Float |
    | 32 Bits | Format::RG16UNorm, Format::RG16SNorm, Format::RG16UInt, Format::RG16SInt, Format::RG16Float, Format::RGBA8UNorm, Format::RGBA8SNorm, Format::RGBA8UInt, Format::RGBA8SInt |
    | 24 Bits | Format::RGB8UNorm, Format::RGB8SNorm, Format::RGB8UInt, Format::RGB8SInt |
    | 16 Bits | Format::R16UNorm, Format::R16SNorm, Format::R16UInt, Format::R16SInt, Format::R16Float, Format::RG8UNorm, Format::RG8SNorm, Format::RG8UInt, Format::RG8SInt |
    | 8 Bits | Format::R8UNorm, Format::R8SNorm, Format::R8UInt, Format::R8SInt |
    */
    Format              format          = Format::RGBA8UNorm;

    /**
    \brief Specifies the texture subresource, i.e. MIP-map level and array layer range. By default only the first MIP-map level and first array layer is addressed.
    \remarks For texture subresources that are bound with the binding flag BindFlags::Storage, \c numMipLevels \b must be 1.
    */
    TextureSubresource  subresource;

    /**
    \brief Specifies the color component mapping. Each component is mapped to its identity by default.
    \remarks If texture swizzling is not supported, this must be equal to the default value.
    \note Only supported with: OpenGL, Vulkan, Metal, Direct3D 12.
    \see RenderingFeatures::hasTextureViewSwizzle
    \see IsTextureSwizzleIdentity
    */
    TextureSwizzleRGBA  swizzle;
};

/**
\brief Memory footprint structure for texture subresources.
\see Texture::GetSubresourceFootprint
*/
struct SubresourceFootprint
{
    //! Total size (in bytes) of the texture subresource.
    std::uint64_t size          = 0;

    //! Alignment (in bytes) for each row. Minimum alignment a renderer must return for a texture is 1.
    std::uint32_t rowAlignment  = 0;

    /**
    \brief Size (in bytes) of each row in the texture subresource.
    \remarks Not to be confused with row stride although this value \e might be equal to rowStride.
    */
    std::uint32_t rowSize       = 0;

    /**
    \brief Stride (in bytes) of each row in the texture subresource. This is aligned to rowAlignment.
    \see rowAlignment
    */
    std::uint32_t rowStride     = 0;

    /**
    \brief Size (in bytes) of each layer in the texture subresource. For 3D textures, this counts as a depth layer.
    \remarks Not to be confused with layer stride although this value \e might be equal to layerStride.
    */
    std::uint32_t layerSize     = 0;

    //! Stride (in bytes) for each layer. For 3D textures, this counts as depth layer.
    std::uint32_t layerStride   = 0;
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
\return \f$\left\lfloor 1 + log_2\left(\max\left\{ \texttt{width}, \texttt{height}, \texttt{depth} \right\} \right) \right\rfloor\f$
*/
LLGL_EXPORT std::uint32_t NumMipLevels(std::uint32_t width, std::uint32_t height = 1, std::uint32_t depth = 1);

/**
\brief Returns the number of MIP-map levels for the specified texture attributes.
\param[in] type Specifies the texture type for which the MIP-map extent is to be determined.
\param[in] extent Specifies the extent of the first MIP-map level.
\see NumMipLevels(std::uint32_t, std::uint32_t, std::uint32_t)
*/
LLGL_EXPORT std::uint32_t NumMipLevels(const TextureType type, const Extent3D& extent);

/**
\brief Returns the number of MIP-map levels for the specified texture descriptor.
\param[in] textureDesc Specifies the descriptor whose parameters are used to determine the number of MIP-map levels.
\remarks This function will deduce the number MIP-map levels automatically only if the member \c mipLevels is zero.
Otherwise, the value of this member is returned.
\see NumMipLevels(const TextureType, const Extent3D&)
*/
LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc);

/**
\brief Returns the number of texture elements (texels) for the specified texture attributes, or zero extent if \c mipLevel is out of bounds (see \c NumMipLevels).
\param[in] type Specifies the texture type for which the number of texels are to be determined.
\param[in] extent Specifies the extent of the first MIP-map level.
\param[in] mipLevel Specifies the MIP-map level whose number of texels is to be determined. The first and largest MIP-map level has index zero.
\see TextureDescriptor::type
\see NumMipLevels(const TextureType, const Extent3D&)
*/
LLGL_EXPORT std::uint32_t NumMipTexels(const TextureType type, const Extent3D& extent, std::uint32_t mipLevel);

/**
\brief Returns the number of texture elements (texels) for the specified texture subresource range.
\param[in] type Specifies the texture type for which the number of texels are to be determined.
\param[in] extent Specifies the extent of the first MIP-map level.
\param[in] subresource Specifies the subresource range.
Which dimension in \c extent specifies the number of array layers depends on the texture type.
\see TextureDescriptor::arrayLayers
\see TextureSubresource::baseArrayLayer
\see TextureSubresource::numArrayLayers
\see NumMipTexels(const TextureType, const Extent3D&, std::uint32_t)
*/
LLGL_EXPORT std::uint32_t NumMipTexels(const TextureType type, const Extent3D& extent, const TextureSubresource& subresource);

/**
\brief Returns the number of texture elements (texels) for the specified texture descriptor.
\param[in] textureDesc Specifies the texture descriptor for which the number of MIP-map texels are determined.
\param[in] mipLevel Optional parameter to specify only a single MIP-map level.
If this is \c 0xFFFFFFFF, the number of texels for the entire MIP-map chain is determined. By default \c 0xFFFFFFFF.
The size of the MIP-map chain is determined by \c textureDesc.mipLevels.
\see NumMipTexels(const TextureType, const Extent3D&, std::uint32_t)
\see TextureDescriptor::mipLevels
*/
LLGL_EXPORT std::uint32_t NumMipTexels(const TextureDescriptor& textureDesc, std::uint32_t mipLevel = ~0u);

/**
\brief Returns the number of MIP-map dimensions for the specified texture type. This is either 1, 2, 3, or 0 if the input is invalid.
\remarks MIP-map dimensions <b>do count</b> array layers as a dimension, e.g. for TextureType::Texture2DArray this function returns 3.
\see Texture::GetMipExtent
\see NumTextureDimensions
*/
LLGL_EXPORT std::uint32_t NumMipDimensions(const TextureType type);

/**
\brief Returns the number of texture dimensions for the specified texture type. This is either 1, 2, 3, or 0 if the input is invalid.
\remarks Texture dimensions <b>don't count</b> array layers as a dimension, e.g. for TextureType::Texture2DArray this function returns 2.
\see TextureDescriptor::extent
\see NumMipDimensions
*/
LLGL_EXPORT std::uint32_t NumTextureDimensions(const TextureType type);

/**
\brief Returns the MIP-map extent (including array layers) for the specified texture type, or an empty extent if \c mipLevel is out of bounds (see \c NumMipLevels).
\param[in] type Specifies the texture type for which the MIP-map extent is to be determined.
\param[in] extent Specifies the extent of the first MIP-map level.
\param[in] mipLevel Specifies the MIP-map level whose extent is to be determined. The first and largest MIP-map level has index zero.
\see Texture::GetMipExtent
\see TextureDescriptor::extent
\see NumMipLevels(const TextureType, const Extent3D&)
*/
LLGL_EXPORT Extent3D GetMipExtent(const TextureType type, const Extent3D& extent, std::uint32_t mipLevel);

/**
\brief Returns the MIP-map extent (including array layers) for the specified texture descriptor.
\param[in] textureDesc Specifies the texture descriptor.
\param[in] mipLevel Specifies the MIP-map level whose extent is to be determined. The first and largest MIP-map level has index zero.
\see Texture::GetMipExtent
\see TextureDescriptor::extent
\see TextureDescriptor::arrayLayers
*/
LLGL_EXPORT Extent3D GetMipExtent(const TextureDescriptor& textureDesc, std::uint32_t mipLevel = 0);

/**
\brief Returns the memory footprint (in bytes) of a texture subresource with the specified hardware format and extent.
\param[in] type Specifies the texture type which determines how extent and subresource will result into the final texture dimensions.
\param[in] format Specifies the hardware format.
\param[in] extent Specifies the extent of the texture subresource.
\param[in] subresource Specifies the texture subresource.
\see GetMemoryFootprint(const Format, std::uint64_t)
\see Texture::GetMemoryFootprint
\see Texture::GetMipExtent
\see Texture::GetFormat
*/
LLGL_EXPORT std::size_t GetMemoryFootprint(const TextureType type, const Format format, const Extent3D& extent, const TextureSubresource& subresource);

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

/**
\brief Returns true if the specified texture swizzling is equal to the identity mapping.
\return True if the components are mapped as follows:
- \c r equals TextureSwizzle::Red
- \c g equals TextureSwizzle::Green
- \c b equals TextureSwizzle::Blue
- \c a equals TextureSwizzle::Alpha
\see TextureSwizzle
*/
LLGL_EXPORT bool IsTextureSwizzleIdentity(const TextureSwizzleRGBA& swizzle);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
