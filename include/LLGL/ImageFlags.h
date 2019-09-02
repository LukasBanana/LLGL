/*
 * ImageFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMAGE_FLAGS_H
#define LLGL_IMAGE_FLAGS_H


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


/* ----- Structures ----- */

/**
\brief Descriptor structure for an image that is used as source for reading the image data.
\remarks This kind of 'Image' is mainly used to fill a MIP-map within a hardware texture by reading from a source image.
The counterpart for reading a MIP-map from a hardware texture by writing to a destination image is the DstImageDescriptor structure.
\see DstImageDescriptor
\see ConvertImageBuffer
\see RenderSystem::CreateTexture
\see RenderSystem::WriteTexture
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

    //! Pointer to the read-only image data.
    const void* data        = nullptr;

    //! Specifies the size (in bytes) of the image data. This is primarily used for compressed images and serves for robustness.
    std::size_t dataSize    = 0;
};

/**
\brief Descriptor structure for an image that is used as destination for writing the image data.
\remarks This kind of 'Image' is mainly used to fill the image data of a hardware texture.
\see SrcImageDescriptor
\see ConvertImageBuffer
\see RenderSystem::ReadTexture
*/
struct DstImageDescriptor
{
    DstImageDescriptor() = default;
    DstImageDescriptor(const DstImageDescriptor&) = default;

    //! Constructor to initialize all attributes.
    inline DstImageDescriptor(ImageFormat format, DataType dataType, void* data, std::size_t dataSize) :
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

    //! Pointer to the read/write image data.
    void*       data        = nullptr;

    //! Specifies the size (in bytes) of the image data. This is primarily used for compressed images and serves for robustness.
    std::size_t dataSize    = 0;
};


/* ----- Functions ----- */

/**
\defgroup group_image_util Image utility functions to classify and convert image data.
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
LLGL_EXPORT std::uint32_t ImageFormatSize(const ImageFormat imageFormat);

/**
\brief Returns the required data size (in bytes) of an image with the specified format, data type, and number of pixels.
\param[in] imageFormat Specifies the image format.
\param[in] dataType Specifies the data type of each pixel component.
\param[in] numPixels Specifies the number of picture elements (pixels).
\remarks The counterpart for texture buffers is the function TextureBufferSize.
\see TextureBufferSize
*/
LLGL_EXPORT std::uint32_t ImageDataSize(const ImageFormat imageFormat, const DataType dataType, std::uint32_t numPixels);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat imageFormat);

/**
\brief Returns true if the specified color format is a depth-stencil format,
i.e. either ImageFormat::Depth or ImageFormat::DepthStencil.
\see ImageFormat
*/
LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat imageFormat);

/**
\brief Converts the image format and data type of the source image (only uncompressed color formats).
\param[in] srcImageDesc Specifies the source image descriptor.
\param[out] dstImageDesc Specifies the destination image descriptor.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is less than 2, no multi-threading is used. If this is 'Constants::maxThreadCount',
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return True if any conversion was necessary. Otherwise, no conversion was necessary and the destination buffer is not modified!
\note Compressed images and depth-stencil images cannot be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination.
\throw std::invalid_argument If a depth-stencil format is specified either as source or destination.
\throw std::invalid_argument If the source buffer size is not a multiple of the source data type size times the image format size.
\throw std::invalid_argument If the source buffer is a null pointer.
\throw std::invalid_argument If the destination buffer size does not match the required output buffer size.
\throw std::invalid_argument If the destination buffer is a null pointer.
\see Constants::maxThreadCount
\see DataTypeSize
\see ImageFormatSize
*/
LLGL_EXPORT bool ConvertImageBuffer(
    const SrcImageDescriptor&   srcImageDesc,
    const DstImageDescriptor&   dstImageDesc,
    std::size_t                 threadCount = 0
);

/**
\brief Converst the image format and data type of the source image (only uncompressed color formats) and returns the new generated image buffer.
\param[in] srcImageDesc Specifies the source image descriptor.
\param[in] dstFormat Specifies the destination image format.
\param[in] dstDataType Specifies the destination image data type.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is less than 2, no multi-threading is used. If this is 'Constants::maxThreadCount',
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return Byte buffer with the converted image data or null if no conversion is necessary.
This can be casted to the respective target data type (e.g. <code>unsigned char</code>, <code>int</code>, <code>float</code> etc.).
\note Compressed images and depth-stencil images cannot be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination.
\throw std::invalid_argument If a depth-stencil format is specified either as source or destination.
\throw std::invalid_argument If the source buffer size is not a multiple of the source data type size times the image format size.
\throw std::invalid_argument If the source buffer is a null pointer.
\see Constants::maxThreadCount
\see ByteBuffer
\see DataTypeSize
\see ImageFormatSize
*/
LLGL_EXPORT ByteBuffer ConvertImageBuffer(
    const SrcImageDescriptor&   srcImageDesc,
    ImageFormat                 dstFormat,
    DataType                    dstDataType,
    std::size_t                 threadCount = 0
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
    LLGL::ColorRGBAd{ 1.0, 1.0, 0.0, 0.5 }
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
\param[in] initialize Specifies whether to initialize the byte buffer with zeros. By default true.
\return The new allocated and initialized byte buffer.
\remarks Use GenerateImageBuffer to generate an image buffer with a fill color.
\see GenerateImageBuffer
*/
LLGL_EXPORT ByteBuffer GenerateEmptyByteBuffer(std::size_t bufferSize, bool initialize = true);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
