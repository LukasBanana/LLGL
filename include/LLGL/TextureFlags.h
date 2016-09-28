/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE_FLAGS_H__
#define __LLGL_TEXTURE_FLAGS_H__


#include "Export.h"
#include <Gauss/Vector3.h>
#include <cstddef>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Texture type enumeration.
enum class TextureType
{
    Undefined,          //!< Initial value of a Texture object.
    Texture1D,          //!< 1-Dimensional texture.
    Texture2D,          //!< 2-Dimensional texture.
    Texture3D,          //!< 3-Dimensional texture.
    TextureCube,        //!< Cube texture.
    Texture1DArray,     //!< 1-Dimensional array texture.
    Texture2DArray,     //!< 2-Dimensional array texture.
    TextureCubeArray,   //!< Cube array texture.
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

//! Texture descriptor structure.
struct TextureDescriptor
{
    TextureDescriptor()
    {
        texture3DDesc.width     = 0;
        texture3DDesc.height    = 0;
        texture3DDesc.depth     = 0;
    }
    ~TextureDescriptor()
    {
    }

    TextureType             type    = TextureType::Undefined;   //!< Texture type.
    TextureFormat           format  = TextureFormat::Unknown;   //!< Texture hardware format.

    union
    {
        struct Texture1DDescriptor
        {
            int             width;  //!< Texture width.
        }
        texture1DDesc;

        struct Texture2DDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
        }
        texture2DDesc;

        struct Texture3DDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            int             depth;  //!< Texture depth.
        }
        texture3DDesc;

        struct TextureCubeDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
        }
        textureCubeDesc;

        struct Texture1DArrayDescriptor
        {
            int             width;  //!< Texture width.
            unsigned int    layers; //!< Number of texture array layers.
        }
        texture1DArrayDesc;

        struct Texture2DArrayDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            unsigned int    layers; //!< Number of texture array layers.
        }
        texture2DArrayDesc;

        struct TextureCubeArrayDescriptor
        {
            int             width;  //!< Texture width.
            int             height; //!< Texture height.
            unsigned int    layers; //!< Number of texture array layers (this will be multiplied by 6 internally to fit the 6-cube-faces requirement).
        }
        textureCubeArrayDesc;
    };
};


/* ----- Functions ----- */

/**
\brief Returns the number of MIP-map levels for a texture with the specified size.
\return 1 + floor(log2(max{ x, y, z })).
*/
LLGL_EXPORT int NumMipLevels(const Gs::Vector3i& textureSize);

/**
\brief Returns true if the specified texture format is a compressed format,
i.e. either TextureFormat::RGB_DXT1, TextureFormat::RGBA_DXT1, TextureFormat::RGBA_DXT3, or TextureFormat::RGBA_DXT5.
\see TextureFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const TextureFormat format);


} // /namespace LLGL


#endif



// ================================================================================
