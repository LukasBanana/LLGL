/*
 * Image.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_IMAGE_H__
#define __LLGL_IMAGE_H__


#include "Export.h"
#include "RenderSystemFlags.h"
#include "TextureFlags.h"
#include <memory>


namespace LLGL
{


/* ----- Types ----- */

/**
\brief Specifies the maximal number of threads the system supports.
\see ConvertImageBuffer
*/
static const std::size_t maxThreadCount = ~0;

/**
\brief Image buffer type.
\remarks Commonly this would be an std::vector<char>, but the image conversion is an optimized process,
where the default initialization of an std::vector is undesired.
Therefore, the image buffer type is an std::unique_ptr<char[]>.
\see ConvertImage
*/
using ImageBuffer = std::unique_ptr<char[]>;


/* ----- Enumerations ----- */

//! Renderer data types enumeration.
enum class DataType
{
    Int8,   //!< 8-bit signed integer (char).
    UInt8,  //!< 8-bit unsigned integer (unsigned char).

    Int16,  //!< 16-bit signed integer (short).
    UInt16, //!< 16-bit unsigned integer (unsigned short).

    Int32,  //!< 32-bit signed integer (int).
    UInt32, //!< 32-bit unsigned integer (unsiged int).
    
    Float,  //!< 32-bit floating-point (float).
    Double, //!< 64-bit real type (double).
};

//! Image format used to write texture data.
enum class ImageFormat
{
    R,              //!< Single color component: Red.
    RG,             //!< Two color components: Red, Green.
    RGB,            //!< Three color components: Red, Green, Blue.
    BGR,            //!< Three color components: Blue, Green, Red.
    RGBA,           //!< Four color components: Red, Green, Blue, Alpha.
    BGRA,           //!< Four color components: Blue, Green, Red, Alpha.
    Depth,          //!< Single color component used as depth component.
    DepthStencil,   //!< Pair of depth and stencil component.
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
    ImageDescriptor(ImageFormat format, DataType dataType, const void* buffer) :
        format  ( format   ),
        dataType( dataType ),
        buffer  ( buffer   )
    {
    }

    //! Constructor for compressed image data.
    ImageDescriptor(ImageFormat format, const void* buffer, unsigned int compressedSize) :
        format          ( format         ),
        buffer          ( buffer         ),
        compressedSize  ( compressedSize )
    {
    }

    ImageFormat     format          = ImageFormat::RGBA;    //!< Specifies the image format. By default ImageFormat::RGBA.
    DataType        dataType        = DataType::UInt8;      //!< Specifies the image data type. This must be DataType::UInt8 for compressed images.
    const void*     buffer          = nullptr;              //!< Pointer to the image buffer.
    unsigned int    compressedSize  = 0;                    //!< Specifies the size (in bytes) of a compressed image. This must be 0 for uncompressed images.
};


/* ----- Functions ----- */

//! Returns the size (in bytes) of the specified data type.
LLGL_EXPORT std::size_t DataTypeSize(const DataType dataType);

/**
\brief Returns the size (in number of components) of the specified image format.
\param[in] imageFormat Specifies the image format.
\return Number of components of the specified image format, or 0 if 'imageFormat' specifies a compressed color format.
\see IsCompressedFormat(const ImageFormat)
*/
LLGL_EXPORT std::size_t ImageFormatSize(const ImageFormat imageFormat);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format);

/**
\brief Returns true if the specified color foramt is a depth-stencil format,
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
\return Image buffer with the converted image data or null if no conversion is necessary.
This can be casted to the respective target data type (e.g. "unsigned char", "int", "float" etc.).
\remarks Compressed images and depth-stencil images can not be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination,
if a depth-stencil format is specified either as source or destination,
if the source buffer size is not a multiple of the source data type size times the image format size,
or if 'srcBuffer' is a null pointer.
\see maxThreadCount
\see ImageBuffer
\see DataTypeSize
*/
LLGL_EXPORT ImageBuffer ConvertImageBuffer(
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
