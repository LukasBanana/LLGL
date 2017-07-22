/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_TEXTURE_FLAGS_H
#define LLGL_TEXTURE_FLAGS_H


#include "Export.h"
#include <Gauss/Vector3.h>
#include <cstddef>


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

/**
\brief Hardware texture format enumeration.
\note All integral 32-bit formats are un-normalized!
*/
enum class TextureFormat
{
    Unknown,        //!< Unknown texture format.

    /* --- Base formats --- */
    DepthComponent, //!< Base format: depth component.
    DepthStencil,   //!< Base format: depth- and stencil components.
    R,              //!< Base format: red component.
    RG,             //!< Base format: red and green components.
    RGB,            //!< Base format: red, green, and blue components. \note Only supported with: OpenGL.
    RGBA,           //!< Base format: red, green, blue, and alpha components.

    /* --- Sized formats --- */
    R8,             //!< Sized format: red 8-bit normalized unsigned integer component.
    R8Sgn,          //!< Sized format: red 8-bit normalized signed integer component.

    R16,            //!< Sized format: red 16-bit normalized unsigned interger component.
    R16Sgn,         //!< Sized format: red 16-bit normalized signed interger component.
    R16Float,       //!< Sized format: red 16-bit floating point component.

    R32UInt,        //!< Sized format: red 32-bit un-normalized unsigned interger component.
    R32SInt,        //!< Sized format: red 32-bit un-normalized signed interger component.
    R32Float,       //!< Sized format: red 32-bit floating point component.

    RG8,            //!< Sized format: red, green 8-bit normalized unsigned integer components.
    RG8Sgn,         //!< Sized format: red, green 8-bit normalized signed integer components.

    RG16,           //!< Sized format: red, green 16-bit normalized unsigned interger components.
    RG16Sgn,        //!< Sized format: red, green 16-bit normalized signed interger components.
    RG16Float,      //!< Sized format: red, green 16-bit floating point components.

    RG32UInt,       //!< Sized format: red, green 32-bit un-normalized unsigned interger components.
    RG32SInt,       //!< Sized format: red, green 32-bit un-normalized signed interger components.
    RG32Float,      //!< Sized format: red, green 32-bit floating point components.

    RGB8,           //!< Sized format: red, green, blue 8-bit normalized unsigned integer components. \note Only supported with: OpenGL.
    RGB8Sgn,        //!< Sized format: red, green, blue 8-bit normalized signed integer components. \note Only supported with: OpenGL.

    RGB16,          //!< Sized format: red, green, blue 16-bit normalized unsigned interger components. \note Only supported with: OpenGL.
    RGB16Sgn,       //!< Sized format: red, green, blue 16-bit normalized signed interger components. \note Only supported with: OpenGL.
    RGB16Float,     //!< Sized format: red, green, blue 16-bit floating point components. \note Only supported with: OpenGL.

    RGB32UInt,      //!< Sized format: red, green, blue 32-bit un-normalized unsigned interger components.
    RGB32SInt,      //!< Sized format: red, green, blue 32-bit un-normalized signed interger components.
    RGB32Float,     //!< Sized format: red, green, blue 32-bit floating point components.

    RGBA8,          //!< Sized format: red, green, blue, alpha 8-bit normalized unsigned integer components.
    RGBA8Sgn,       //!< Sized format: red, green, blue, alpha 8-bit normalized signed integer components.

    RGBA16,         //!< Sized format: red, green, blue, alpha 16-bit normalized unsigned interger components.
    RGBA16Sgn,      //!< Sized format: red, green, blue, alpha 16-bit normalized signed interger components.
    RGBA16Float,    //!< Sized format: red, green, blue, alpha 16-bit floating point components.

    RGBA32UInt,     //!< Sized format: red, green, blue, alpha 32-bit un-normalized unsigned interger components.
    RGBA32SInt,     //!< Sized format: red, green, blue, alpha 32-bit un-normalized signed interger components.
    RGBA32Float,    //!< Sized format: red, green, blue, alpha 32-bit floating point components.

    /* --- Compressed formats --- */
    RGB_DXT1,       //!< Compressed format: RGB S3TC DXT1.
    RGBA_DXT1,      //!< Compressed format: RGBA S3TC DXT1.
    RGBA_DXT3,      //!< Compressed format: RGBA S3TC DXT3.
    RGBA_DXT5,      //!< Compressed format: RGBA S3TC DXT5.
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


/* ----- Structures ----- */

/**
\brief Texture descriptor structure.
\remarks This is used to specifiy the dimensions of a texture which is to be created.
*/
struct LLGL_EXPORT TextureDescriptor
{
    struct Texture1DDescriptor
    {
        unsigned int    width;          //!< Texture width.
        unsigned int    layers;         //!< Number of texture array layers.
    };

    struct Texture2DDescriptor
    {
        unsigned int    width;          //!< Texture width.
        unsigned int    height;         //!< Texture height.
        unsigned int    layers;         //!< Number of texture array layers.
    };

    struct Texture3DDescriptor
    {
        unsigned int    width;          //!< Texture width.
        unsigned int    height;         //!< Texture height.
        unsigned int    depth;          //!< Texture depth.
    };

    struct TextureCubeDescriptor
    {
        unsigned int    width;          //!< Texture width.
        unsigned int    height;         //!< Texture height.
        unsigned int    layers;         //!< Number of texture array layers (internally it will be a multiple of 6).
    };

    struct Texture2DMSDescriptor
    {
        unsigned int    width;          //!< Texture width.
        unsigned int    height;         //!< Texture height.
        unsigned int    layers;         //!< Number of texture array layers.
        unsigned int    samples;        //!< Number of samples.
        bool            fixedSamples;   //!< Specifies whether the sample locations are fixed or not. By default true. \note Only supported with: OpenGL.
    };
    
    TextureDescriptor()
    {
        type                        = TextureType::Texture1D;
        format                      = TextureFormat::RGBA;
        texture2DMS.width           = 0;
        texture2DMS.height          = 0;
        texture2DMS.layers          = 0;
        texture2DMS.samples         = 0;
        texture2DMS.fixedSamples    = true;
    }

    ~TextureDescriptor()
    {
        // Dummy
    }

    TextureType                 type;           //!< Texture type. By default TextureType::Texture1D.
    TextureFormat               format;         //!< Texture hardware format. By default TextureFormat::RGBA.

    union
    {
        Texture1DDescriptor     texture1D;      //!< Descriptor for 1D- and 1D-Array textures.
        Texture2DDescriptor     texture2D;      //!< Descriptor for 2D- and 2D-Array textures.
        Texture3DDescriptor     texture3D;      //!< Descriptor for 3D textures.
        TextureCubeDescriptor   textureCube;    //!< Descriptor for Cube- and Cube-Array textures.
        Texture2DMSDescriptor   texture2DMS;    //!< Descriptor for multi-sampled 2D- and 2D-Array textures.
    };
};

/**
\brief Sub-texture descriptor structure.
\remarks This is used to write (or partially write) the image data of a texture MIP-map level.
*/
struct LLGL_EXPORT SubTextureDescriptor
{
    struct Texture1DDescriptor
    {
        unsigned int    x;              //!< Sub-texture X-axis offset.
        unsigned int    layerOffset;    //!< Zero-based layer offset.
        unsigned int    width;          //!< Sub-texture width.
        unsigned int    layers;         //!< Number of texture array layers.
    };

    struct Texture2DDescriptor
    {
        unsigned int    x;              //!< Sub-texture X-axis offset.
        unsigned int    y;              //!< Sub-texture Y-axis offset.
        unsigned int    layerOffset;    //!< Zero-based layer offset.
        unsigned int    width;          //!< Sub-texture width.
        unsigned int    height;         //!< Sub-texture height.
        unsigned int    layers;         //!< Number of texture array layers.
    };

    struct Texture3DDescriptor
    {
        unsigned int    x;              //!< Sub-texture X-axis offset.
        unsigned int    y;              //!< Sub-texture Y-axis offset.
        unsigned int    z;              //!< Sub-texture Z-axis offset.
        unsigned int    width;          //!< Sub-texture width.
        unsigned int    height;         //!< Sub-texture height.
        unsigned int    depth;          //!< Number of texture array layers.
    };

    struct TextureCubeDescriptor
    {
        unsigned int    x;              //!< Sub-texture X-axis offset.
        unsigned int    y;              //!< Sub-texture Y-axis offset.
        unsigned int    layerOffset;    //!< Zero-based layer offset.
        unsigned int    width;          //!< Sub-texture width.
        unsigned int    height;         //!< Sub-texture height.
        unsigned int    cubeFaces;      //!< Number of cube-faces. To have all faces of N cube-texture layers, this value must be N*6.
        AxisDirection   cubeFaceOffset; //!< First cube face in the current layer.
    };

    SubTextureDescriptor()
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
    ~SubTextureDescriptor()
    {
    }

    unsigned int                mipLevel;       //!< MIP-map level for the sub-texture, where 0 is the base texture, and n > 0 is the n-th MIP-map level.

    union
    {
        Texture1DDescriptor     texture1D;      //!< Descriptor for 1D- and 1D-Array textures.
        Texture2DDescriptor     texture2D;      //!< Descriptor for 2D- and 2D-Array textures.
        Texture3DDescriptor     texture3D;      //!< Descriptor for 3D textures.
        TextureCubeDescriptor   textureCube;    //!< Descriptor for Cube- and Cube-Array textures.
    };
};


/* ----- Functions ----- */

/**
\brief Returns the number of MIP-map levels for a texture with the specified size.
\param[in] width Specifies the texture width.
\param[in] height Specifies the texture height or number of layers for 1D array textures. By default 1 (if 1D textures are used).
\param[in] depth Specifies the texture depth or number of layers for 2D array textures. By default 1 (if 1D or 2D textures are used).
\remarks The height and depth are optional parameters, so this function can be easily used for 1D, 2D, and 3D textures.
\return 1 + floor(log2(max{ x, y, z })).
*/
LLGL_EXPORT unsigned int NumMipLevels(unsigned int width, unsigned int height = 1, unsigned int depth = 1);

/**
\brief Returns true if the specified texture format is a compressed format,
i.e. either TextureFormat::RGB_DXT1, TextureFormat::RGBA_DXT1, TextureFormat::RGBA_DXT3, or TextureFormat::RGBA_DXT5.
\see TextureFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const TextureFormat format);

/**
\brief Returns true if the specified texture format is a depth or depth-stencil format,
i.e. either TextureFormat::DepthComponent, or TextureFormat::DepthStencil.
\see TextureFormat
*/
LLGL_EXPORT bool IsDepthStencilFormat(const TextureFormat format);

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


} // /namespace LLGL


#endif



// ================================================================================
