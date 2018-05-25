/*
 * Image.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMAGE_H
#define LLGL_IMAGE_H


#include "Export.h"
#include "Format.h"
#include "RenderSystemFlags.h"
#include "TextureFlags.h"
#include "ColorRGBA.h"
#include <memory>
#include <cstdint>


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

/**
\brief Image format enumeration that applies to each pixel of an image.
\see SrcImageDescriptor::format
\see ImageFormatSize
*/
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
    Depth,          //!< Depth component.
    DepthStencil,   //!< Depth component and stencil index.

    /* Compressed formats */
    CompressedRGB,  //!< Generic compressed format with three color components: Red, Green, Blue.
    CompressedRGBA, //!< Generic compressed format with four color components: Red, Green, Blue, Alpha.
};


/* ----- Structures ----- */

/**
\brief Descriptor structure for an image that is used as source for reading the image data.
\remarks This kind of 'Image' is mainly used to fill the image data of a hardware texture.
*/
struct SrcImageDescriptor
{
    SrcImageDescriptor() = default;
    SrcImageDescriptor(const SrcImageDescriptor&) = default;

    //! Constructor to initialize all attributes.
    inline SrcImageDescriptor(ImageFormat format, DataType dataType, const void* data, std::size_t dataSize) :
        format   { format   },
        dataType { dataType },
        data     { data     },
        dataSize { dataSize }
    {
    }

    //! Specifies the image format. By default ImageFormat::RGBA.
    ImageFormat format      = ImageFormat::RGBA;

    //! Specifies the image data type. This must be DataType::UInt8 for compressed images. By default DataType::UInt8.
    DataType    dataType    = DataType::UInt8;

    //! Pointer to the image data.
    const void* data        = nullptr;

    //! Specifies the size (in bytes) of the image data. This is primarily used for compressed images and serves for robustness.
    std::size_t dataSize    = 0;
};


/* ----- Functions ----- */

/**
\defgroup group_image_util Global image utility functions to classify and convert image data.
\addtogroup group_image_util
@{
*/

/**
\brief Returns the size (in number of components) of the specified image format.
\param[in] imageFormat Specifies the image format.
\return Number of components of the specified image format, or 0 if 'imageFormat' specifies a compressed color format.
\note Compressed formats are not supported.
\see IsCompressedFormat(const ImageFormat)
\see ImageFormat
*/
LLGL_EXPORT std::uint32_t ImageFormatSize(const ImageFormat format);

/**
\brief Returns the required data size (in bytes) of an image with the specified format, data type, and number of pixels.
\param[in] format Specifies the image format.
\param[in] dataType Specifies the data type of each pixel component.
\param[in] numPixels Specifies the number of picture elements (pixels).
\remarks The counterpart for texture buffers is the function TextureBufferSize.
\see TextureBufferSize
*/
LLGL_EXPORT std::uint32_t ImageDataSize(const ImageFormat format, const DataType dataType, std::uint32_t numPixels);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format);

/**
\brief Returns true if the specified color format is a depth-stencil format,
i.e. either ImageFormat::Depth or ImageFormat::DepthStencil.
\see ImageFormat
*/
LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat format);

/**
\brief Finds a suitable image format for the specified texture hardware format.
\param[in] textureFormat Specifies the input texture format.
\param[out] imageFormat Specifies the output image format.
\param[out] dataType Specifies the output image data type.
\return True if a suitable image format has been found. Otherwise, the output parameter 'imageFormat' and 'dataType' have not been modified.
\remarks Texture formats that cannot be converted to an image format are all 16-bit floating-point types, and TextureFormat::Unknown.
*/
LLGL_EXPORT bool FindSuitableImageFormat(const TextureFormat textureFormat, ImageFormat& imageFormat, DataType& dataType);

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
\remarks Compressed images and depth-stencil images cannot be converted.
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

/**
\brief Generates an image buffer with the specified fill data for each pixel.
\param[in] format Specifies the image format of each pixel in the output image.
\param[in] dataType Specifies the data type of each component of each pixel in the output image.
\param[in] imageSize Specifies the 1-Dimensional size (in pixels) of the output image. For a 2D image, this can be width times height for instance.
\param[in] fillColor Specifies the color to fill the image for each pixel.
\return The new allocated and initialized byte buffer.
\remarks This can be used to generate a single-colored n-Dimensional image.
Usage example for a 2D image:
\code
// Generate 2D image of size 512 x 512 with a half-transparent yellow color
auto imageBuffer = LLGL::GenerateImageBuffer(
    LLGL::ImageFormat::RGBA,
    LLGL::DataType::UInt8,
    512 * 512,
    LLGL::ColorRGBAd { 1.0, 1.0, 0.0, 0.5 }
);
\endcode
*/
LLGL_EXPORT ByteBuffer GenerateImageBuffer(
    ImageFormat         format,
    DataType            dataType,
    std::size_t         imageSize,
    const ColorRGBAd&   fillColor
);

/**
\brief Generates a new byte buffer with zeros in each byte.
\param[in] bufferSize Specifies the size (in bytes) of the buffer.
\return The new allocated and initialized byte buffer.
*/
LLGL_EXPORT ByteBuffer GenerateEmptyByteBuffer(std::size_t bufferSize);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
