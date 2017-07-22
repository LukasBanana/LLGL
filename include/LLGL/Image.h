/*
 * Image.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMAGE_H
#define LLGL_IMAGE_H


#include "Export.h"
#include "Format.h"
#include "RenderSystemFlags.h"
#include "TextureFlags.h"
#include <memory>


namespace LLGL
{


/* ----- Types ----- */

/**
\brief Common byte buffer type.
\remarks Commonly this would be an std::vector<char>, but the buffer conversion is an optimized process,
where the default initialization of an std::vector is undesired.
Therefore, the byte buffer type is an std::unique_ptr<char[]>.
\see ConvertImageBuffer
*/
using ByteBuffer = std::unique_ptr<char[]>;


/* ----- Enumerations ----- */

//! Image format used to write texture data.
enum class ImageFormat
{
    /* Color formats */
    R,              //!< Single color component: Red.
    RG,             //!< Two color components: Red, Green.
    RGB,            //!< Three color components: Red, Green, Blue.
    BGR,            //!< Three color components: Blue, Green, Red.
    RGBA,           //!< Four color components: Red, Green, Blue, Alpha.
    BGRA,           //!< Four color components: Blue, Green, Red, Alpha.
    ARGB,           //!< Four color components: Alpha, Red, Green, Blue. Old format, mainly used in Direct3D 9.
    ABGR,           //!< Four color components: Alpha, Blue, Green, Red. Old format, mainly used in Direct3D 9.

    /* Depth-stencil formats */
    Depth,          //!< 32-bit depth component.
    DepthStencil,   //!< 24-bit depth- and 8-bit stencil component.

    /* Compressed formats */
    CompressedRGB,  //!< Generic compressed format with three color components: Red, Green, Blue.
    CompressedRGBA, //!< Generic compressed format with four color components: Red, Green, Blue, Alpha.
};


/* ----- Structures ----- */

/**
\brief Image descriptor structure.
\remarks This kind of 'Image' is mainly used to fill the image data of a hardware texture.
*/
struct LLGL_EXPORT ImageDescriptor
{
    ImageDescriptor() = default;

    // Constructor for uncompressed image data.
    inline ImageDescriptor(ImageFormat format, DataType dataType, const void* buffer) :
        format   { format   },
        dataType { dataType },
        buffer   { buffer   }
    {
    }

    //! Constructor for compressed image data.
    inline ImageDescriptor(ImageFormat format, const void* buffer, unsigned int compressedSize) :
        format         { format         },
        buffer         { buffer         },
        compressedSize { compressedSize }
    {
    }

    /**
    \brief Returns the size (in bytes) for each image element (i.e. per "texel" or "pixel")
    \return
    \code
    ImageFormatSize(format) * DataTypeSize(dataType);
    \endcode
    */
    unsigned int GetElementSize() const;

    ImageFormat     format          = ImageFormat::RGBA;    //!< Specifies the image format. By default ImageFormat::RGBA.
    DataType        dataType        = DataType::UInt8;      //!< Specifies the image data type. This must be DataType::UInt8 for compressed images.
    const void*     buffer          = nullptr;              //!< Pointer to the image buffer.
    unsigned int    compressedSize  = 0;                    //!< Specifies the size (in bytes) of a compressed image. This must be 0 for uncompressed images.
};


/* ----- Functions ----- */

/**
\brief Returns the size (in number of components) of the specified image format.
\param[in] imageFormat Specifies the image format.
\return Number of components of the specified image format, or 0 if 'imageFormat' specifies a compressed color format.
\see IsCompressedFormat(const ImageFormat)
*/
LLGL_EXPORT unsigned int ImageFormatSize(const ImageFormat imageFormat);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format);

/**
\brief Returns true if the specified color format is a depth-stencil format,
i.e. either ImageFormat::Depth or ImageFormat::DepthStencil.
*/
LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat format);

/**
\brief Converts the image format and data type of the source image (only uncompressed color formats).
\param[in] srcFormat Specifies the source image format.
\param[in] srcDataType Specifies the source data type.
\param[in] srcBuffer Pointer to the source image buffer which is to be converted.
\param[in] srcBufferSize Specifies the size (in bytes) of the source image buffer.
\param[in] dstFormat Specifies the destination image format.
\param[in] dstDataType Specifies the destination data type.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is less than 2, no multi-threading is used. If this is 'maxThreadCount',
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return Byte buffer with the converted image data or null if no conversion is necessary.
This can be casted to the respective target data type (e.g. "unsigned char", "int", "float" etc.).
\remarks Compressed images and depth-stencil images can not be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination,
if a depth-stencil format is specified either as source or destination,
if the source buffer size is not a multiple of the source data type size times the image format size,
or if 'srcBuffer' is a null pointer.
\see maxThreadCount
\see ByteBuffer
\see DataTypeSize
*/
LLGL_EXPORT ByteBuffer ConvertImageBuffer(
    ImageFormat srcFormat,
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    ImageFormat dstFormat,
    DataType    dstDataType,
    std::size_t threadCount = 0
);


} // /namespace LLGL


#endif



// ================================================================================
