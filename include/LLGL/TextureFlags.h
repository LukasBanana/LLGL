/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_FLAGS_H
#define LLGL_TEXTURE_FLAGS_H


#include "Export.h"
#include "Format.h"
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

//! Axis direction (also used for texture cube face).
enum class AxisDirection
{
    XPos = 0,   //!< X+ direction.
    XNeg,       //!< X- direction.
    YPos,       //!< Y+ direction.
    YNeg,       //!< Y- direction.
    ZPos,       //!< Z+ direction.
    ZNeg,       //!< Z- direction.
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

/**
\brief Texture creation flags enumeration.
\see TextureDescriptor::flags
*/
struct TextureFlags
{
    enum
    {
        #if 0
        /**
        \brief Texture mapping with CPU read access is required.
        \see RenderSystem::MapTexture
        */
        MapReadAccess       = (1 << 0),

        /**
        \brief Texture mapping with CPU write access is required.
        \see RenderSystem::MapTexture
        */
        MapWriteAccess      = (1 << 1),

        /*
        \brief Texture mapping with CPU read and write access is required.
        \see TextureFlags::MapReadAccess
        \see TextureFlags::MapWriteAccess
        */
        MapReadWriteAccess  = (MapReadAccess | MapWriteAccess),

        /**
        \brief Hint to the renderer that the texture will be frequently updated from the CPU.
        \see RenderSystem::WriteTexture
        */
        DynamicUsage        = (1 << 2),
        #endif

        /**
        \brief Texture will be used with MIP-mapping. This will create all MIP-map levels at texture creation time.
        \remarks This is part of the default flags.
        \see RenderSystem::GenerateMips
        \see Default
        */
        GenerateMips        = (1 << 3),

        /**
        \brief Texture can be used as render target attachment.
        \remarks This is part of the default flags.
        \see AttachmentDescriptor::texture
        \see Default
        */
        AttachmentUsage     = (1 << 4),

        /**
        \brief Texture can be used for sampling (e.g. "sampler2D" in GLSL, or "Texture2D" in HLSL).
        \remarks This is part of the default flags.
        \see Default
        */
        SampleUsage         = (1 << 5),

        //! Texture can be used as storage texture (e.g. "image2D" in GLSL, or "RWTexture2D" in HLSL).
        StorageUsage        = (1 << 6),

        /**
        \brief Default texture flags: (GenerateMips | AttachmentUsage | SampleUsage).
        \see GenerateMips
        \see AttachmentUsage
        \see SampleUsage
        */
        Default             = (GenerateMips | AttachmentUsage | SampleUsage),
    };
};


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
*/
struct TextureDescriptor
{
    //! 1D- and 1D-Array texture specific descriptor structure.
    struct Texture1D
    {
        std::uint32_t   width;          //!< Texture width.
        std::uint32_t   layers;         //!< Number of texture array layers.
    };

    //! 2D- and 2D-Array texture specific descriptor structure.
    struct Texture2D
    {
        std::uint32_t   width;          //!< Texture width.
        std::uint32_t   height;         //!< Texture height.
        std::uint32_t   layers;         //!< Number of texture array layers.
    };

    //! 3D texture specific descriptor structure.
    struct Texture3D
    {
        std::uint32_t   width;          //!< Texture width.
        std::uint32_t   height;         //!< Texture height.
        std::uint32_t   depth;          //!< Texture depth.
    };

    /**
    \brief Cube- and Cube-Array texture specific descriptor structure.
    \remarks Cube textures must have the same value for width and height.
    However, two parameters are used for convenience in rendering APIs.
    */
    struct TextureCube
    {
        std::uint32_t   width;          //!< Texture width. Must be equal to height.
        std::uint32_t   height;         //!< Texture height. Must be equal to width.

        /**
        \brief Number of texture array layers, one for each cube.
        \remarks Most rendering APIs only use the actual number of array layers, so for cube maps they are always a multiple of 6.
        This attribute, however, specifies the number of cubes in the array texture and LLGL will multiply it by 6 automatically if necessary.
        */
        std::uint32_t   layers;
    };

    //! Multi-sampled 2D- and 2D-Array texture specific descriptor structure.
    struct Texture2DMS
    {
        std::uint32_t   width;          //!< Texture width.
        std::uint32_t   height;         //!< Texture height.
        std::uint32_t   layers;         //!< Number of texture array layers.
        std::uint32_t   samples;        //!< Number of samples.
        bool            fixedSamples;   //!< Specifies whether the sample locations are fixed or not. By default true. \note Only supported with: OpenGL.
    };

    inline TextureDescriptor()
    {
        texture2DMS.width           = 0;
        texture2DMS.height          = 0;
        texture2DMS.layers          = 0;
        texture2DMS.samples         = 0;
        texture2DMS.fixedSamples    = true;
    }

    inline ~TextureDescriptor()
    {
        // Dummy
    }

    //! Hardware texture type. By default TextureType::Texture1D.
    TextureType     type        = TextureType::Texture1D;

    //! Hardware texture format. By default Format::RGBA8UNorm.
    Format          format      = Format::RGBA8UNorm;

    /**
    \brief Specifies the texture creation flags (e.g. if MIP-mapping is required). By default TextureFlags::Default.
    \remarks This can be bitwise OR combination of the entries of the TextureFlags enumeration.
    \see TextureFlags
    */
    long            flags       = TextureFlags::Default;

    union
    {
        Texture1D   texture1D;      //!< Descriptor for 1D- and 1D-Array textures.
        Texture2D   texture2D;      //!< Descriptor for 2D- and 2D-Array textures.
        Texture3D   texture3D;      //!< Descriptor for 3D textures.
        TextureCube textureCube;    //!< Descriptor for Cube- and Cube-Array textures.
        Texture2DMS texture2DMS;    //!< Descriptor for multi-sampled 2D- and 2D-Array textures.
    };
};

/**
\brief Sub-texture descriptor structure.
\remarks This is used to write (or partially write) the image data of a texture MIP-map level.
*/
struct SubTextureDescriptor
{
    struct Texture1D
    {
        std::uint32_t x;              //!< Sub-texture X-axis offset.
        std::uint32_t layerOffset;    //!< Zero-based layer offset.
        std::uint32_t width;          //!< Sub-texture width.
        std::uint32_t layers;         //!< Number of texture array layers.
    };

    struct Texture2D
    {
        std::uint32_t x;              //!< Sub-texture X-axis offset.
        std::uint32_t y;              //!< Sub-texture Y-axis offset.
        std::uint32_t layerOffset;    //!< Zero-based layer offset.
        std::uint32_t width;          //!< Sub-texture width.
        std::uint32_t height;         //!< Sub-texture height.
        std::uint32_t layers;         //!< Number of texture array layers.
    };

    struct Texture3D
    {
        std::uint32_t x;              //!< Sub-texture X-axis offset.
        std::uint32_t y;              //!< Sub-texture Y-axis offset.
        std::uint32_t z;              //!< Sub-texture Z-axis offset.
        std::uint32_t width;          //!< Sub-texture width.
        std::uint32_t height;         //!< Sub-texture height.
        std::uint32_t depth;          //!< Number of texture array layers.
    };

    struct TextureCube
    {
        std::uint32_t x;              //!< Sub-texture X-axis offset.
        std::uint32_t y;              //!< Sub-texture Y-axis offset.
        std::uint32_t layerOffset;    //!< Zero-based layer offset.
        std::uint32_t width;          //!< Sub-texture width.
        std::uint32_t height;         //!< Sub-texture height.
        std::uint32_t cubeFaces;      //!< Number of cube-faces. To have all faces of N cube-texture layers, this value must be N*6.
        AxisDirection cubeFaceOffset; //!< First cube face in the current layer.
    };

    inline SubTextureDescriptor()
    {
        mipLevel                    = 0;
        textureCube.x               = 0;
        textureCube.y               = 0;
        textureCube.layerOffset     = 0;
        textureCube.width           = 0;
        textureCube.height          = 0;
        textureCube.cubeFaces       = 0;
        textureCube.cubeFaceOffset  = AxisDirection::XPos;
    }

    inline ~SubTextureDescriptor()
    {
    }

    //! MIP-map level for the sub-texture, where 0 is the base texture, and n > 0 is the n-th MIP-map level.
    std::uint32_t   mipLevel;

    union
    {
        Texture1D   texture1D;      //!< Descriptor for 1D- and 1D-Array textures.
        Texture2D   texture2D;      //!< Descriptor for 2D- and 2D-Array textures.
        Texture3D   texture3D;      //!< Descriptor for 3D textures.
        TextureCube textureCube;    //!< Descriptor for Cube- and Cube-Array textures.
    };
};


/* ----- Functions ----- */

/**
\defgroup group_tex_util Global texture utility functions to determine texture dimension and buffer sizes.
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
\return Number of MIP-map levels, or 1 if the descriptor has not the 'TextureFlags::GenerateMips' flags bit set.
\see NumMipLevels(std::uint32_t, std::uint32_t, std::uint32_t)
*/
LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc);

/**
\briefs Returns the number of array layers for the specified texture descriptor.
\param[in] textureDesc Specifies the descriptor whose parameters are used to determine the number of array layers.
\return Number of array layers, or 1 if the texture type is not an array texture.
\see IsArrayTexture
*/
LLGL_EXPORT std::uint32_t NumArrayLayers(const TextureDescriptor& textureDesc);

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
\brief Returns true if the specified texture type is an array texture.
\return True if 'type' is . either TextureType::Texture1DArray, TextureType::Texture2DArray,
TextureType::TextureCubeArray, or TextureType::Texture2DMSArray.
*/
LLGL_EXPORT bool IsArrayTexture(const TextureType type);

/**
\brief Returns true if the specified texture type is a multi-sample texture.
\return True if 'type' is either TextureType::Texture2DMS, or TextureType::Texture2DMSArray.
*/
LLGL_EXPORT bool IsMultiSampleTexture(const TextureType type);

/**
\brief Returns true if the specified texture type is a cube texture.
\return True if 'type' is either TextureType::TextureCube or TextureType::TextureCubeArray.
*/
LLGL_EXPORT bool IsCubeTexture(const TextureType type);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
