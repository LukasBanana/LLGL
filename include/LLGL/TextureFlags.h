/*
 * TextureFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_TEXTURE_FLAGS_H__
#define __LLGL_TEXTURE_FLAGS_H__


#include "Export.h"
#include "RenderSystemFlags.h"
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
    /* --- Base formats --- */
    DepthComponent, //!< Base internal format: depth component.
    DepthStencil,   //!< Base internal format: depth- and stencil components.
    R,              //!< Base internal format: red component.
    RG,             //!< Base internal format: red and green components.
    RGB,            //!< Base internal format: red, green and blue components.
    RGBA,           //!< Base internal format: red, green, blue and alpha components.

    /* --- Sized formats --- */
    R8,             //!< Sized internal format: red 8-bit normalized unsigned integer component.
    R8Sgn,          //!< Sized internal format: red 8-bit normalized signed integer component.

    R16,            //!< Sized internal format: red 16-bit normalized unsigned interger component.
    R16Sgn,         //!< Sized internal format: red 16-bit normalized signed interger component.
    R16Float,       //!< Sized internal format: red 16-bit floating point component.

    R32UInt,        //!< Sized internal format: red 32-bit un-normalized unsigned interger component.
    R32SInt,        //!< Sized internal format: red 32-bit un-normalized signed interger component.
    R32Float,       //!< Sized internal format: red 32-bit floating point component.

    RG8,            //!< Sized internal format: red, green 8-bit normalized unsigned integer components.
    RG8Sgn,         //!< Sized internal format: red, green 8-bit normalized signed integer components.

    RG16,           //!< Sized internal format: red, green 16-bit normalized unsigned interger components.
    RG16Sgn,        //!< Sized internal format: red, green 16-bit normalized signed interger components.
    RG16Float,      //!< Sized internal format: red, green 16-bit floating point components.

    RG32UInt,       //!< Sized internal format: red, green 32-bit un-normalized unsigned interger components.
    RG32SInt,       //!< Sized internal format: red, green 32-bit un-normalized signed interger components.
    RG32Float,      //!< Sized internal format: red, green 32-bit floating point components.

    RGB8,           //!< Sized internal format: red, green, blue 8-bit normalized unsigned integer components.
    RGB8Sgn,        //!< Sized internal format: red, green, blue 8-bit normalized signed integer components.

    RGB16,          //!< Sized internal format: red, green, blue 16-bit normalized unsigned interger components.
    RGB16Sgn,       //!< Sized internal format: red, green, blue 16-bit normalized signed interger components.
    RGB16Float,     //!< Sized internal format: red, green, blue 16-bit floating point components.

    RGB32UInt,      //!< Sized internal format: red, green, blue 32-bit un-normalized unsigned interger components.
    RGB32SInt,      //!< Sized internal format: red, green, blue 32-bit un-normalized signed interger components.
    RGB32Float,     //!< Sized internal format: red, green, blue 32-bit floating point components.

    RGBA8,          //!< Sized internal format: red, green, blue, alpha 8-bit normalized unsigned integer components.
    RGBA8Sgn,       //!< Sized internal format: red, green, blue, alpha 8-bit normalized signed integer components.

    RGBA16,         //!< Sized internal format: red, green, blue, alpha 16-bit normalized unsigned interger components.
    RGBA16Sgn,      //!< Sized internal format: red, green, blue, alpha 16-bit normalized signed interger components.
    RGBA16Float,    //!< Sized internal format: red, green, blue, alpha 16-bit floating point components.

    RGBA32UInt,     //!< Sized internal format: red, green, blue, alpha 32-bit un-normalized unsigned interger components.
    RGBA32SInt,     //!< Sized internal format: red, green, blue, alpha 32-bit un-normalized signed interger components.
    RGBA32Float,    //!< Sized internal format: red, green, blue, alpha 32-bit floating point components.
};

//! Color format used to write texture data.
enum class ColorFormat
{
    Gray,           //!< Single color component: Gray or rather brightness.
    GrayAlpha,      //!< Two color components: Gray, Alpha.
    RGB,            //!< Three color components: Red, Green, Blue.
    BGR,            //!< Three color components: Blue, Green, Red.
    RGBA,           //!< Four color components: Red, Green, Blue, Alpha.
    BGRA,           //!< Four color components: Blue, Green, Red, Alpha.
    Depth,          //!< Single color component used as depth component.
    DepthStencil,   //!< Pair of depth and stencil component.
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

//! Texture data descriptor structure.
struct LLGL_EXPORT ImageDataDescriptor
{
    ImageDataDescriptor() = default;
    ImageDataDescriptor(ColorFormat dataFormat, DataType dataType, const void* data) :
        dataFormat  ( dataFormat ),
        dataType    ( dataType   ),
        data        ( data       )
    {
    }

    ColorFormat dataFormat  = ColorFormat::Gray;
    DataType    dataType    = DataType::UByte;
    const void* data        = nullptr;
};

//! Texture descriptor union.
union TextureDescriptor
{
    TextureType type;

    struct Texture1DDescriptor
    {
        TextureType     type;
        int             width;
        unsigned int    layers;
    }
    texture1DDesc;

    struct Texture2DDescriptor
    {
        TextureType     type;
        int             width;
        int             height;
        unsigned int    layers;
    }
    texture2DDesc;

    struct Texture3DDescriptor
    {
        TextureType     type;
        int             width;
        int             height;
        int             depth;
    }
    texture3DDesc;

    struct TextureCubeDescriptor
    {
        TextureType     type;
        int             width;
        int             height;
        unsigned int    layers;
    }
    textureCubeDesc;
};


/* ----- Functions ----- */

//! Returns the size (in number of components) of the specified color format.
LLGL_EXPORT std::size_t ColorFormatSize(const ColorFormat colorFormat);

/**
\brief Returns the number of MIP-map levels for a texture with the specified size.
\return 1 + floor(log2(max{ x, y, z })).
*/
LLGL_EXPORT int NumMipLevels(const Gs::Vector3i& textureSize);


} // /namespace LLGL


#endif



// ================================================================================
