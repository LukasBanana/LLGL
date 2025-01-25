/*
 * ImageFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IMAGE_FLAGS_H
#define LLGL_IMAGE_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Format.h>
#include <LLGL/Tags.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Container/DynamicArray.h>
#include <LLGL/Deprecated.h>
#include <memory>
#include <cstdint>
#include <cstddef>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Image view structure used as source when writing the image data to a hardware texture.
\remarks This kind of image is mainly used to fill a MIP-map within a hardware texture by reading from a source image.
The counterpart for reading a MIP-map from a hardware texture by writing to a destination image is the MutableImageView structure.
\see MutableImageView
\see ConvertImageBuffer
\see RenderSystem::CreateTexture
\see RenderSystem::WriteTexture
*/
struct ImageView
{
    ImageView() = default;
    ImageView(const ImageView&) = default;

    //! Constructor to initialize all attributes.
    inline ImageView(ImageFormat format, DataType dataType, const void* data, std::size_t dataSize, std::uint32_t rowStride = 0) :
        format    { format    },
        dataType  { dataType  },
        data      { data      },
        dataSize  { dataSize  },
        rowStride { rowStride }
    {
    }

    //! Specifies the image format. By default ImageFormat::RGBA.
    ImageFormat format      = ImageFormat::RGBA;

    //! Specifies the image data type. This must be DataType::UInt8 for compressed images. By default DataType::UInt8.
    DataType    dataType    = DataType::UInt8;

    //! Read-only pointer to the image data.
    const void* data        = nullptr;

    //! Specifies the size (in bytes) of the image data. This is primarily used for compressed images and serves for robustness.
    std::size_t dataSize    = 0;

    /**
     \brief Specifies the size of one row of the image data.
     \remarks When this is zero then the width of the texture region is used.
     \note Only supported with: Vulkan, OpenGL.
    */
    std::uint32_t rowStride = 0;
};

/**
\brief Mutable image view structure used as destination when reading the image data from a hardware texture.
\remarks This kind of image is mainly used to fill the image data of a hardware texture.
\see ImageView
\see ConvertImageBuffer
\see RenderSystem::ReadTexture
*/
struct MutableImageView
{
    MutableImageView() = default;
    MutableImageView(const MutableImageView&) = default;

    //! Constructor to initialize all attributes.
    inline MutableImageView(ImageFormat format, DataType dataType, void* data, std::size_t dataSize) :
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

    //! Mutable pointer to the image data.
    void*       data        = nullptr;

    //! Specifies the size (in bytes) of the image data. This is primarily used for compressed images and serves for robustness.
    std::size_t dataSize    = 0;
};

struct LLGL_DEPRECATED("LLGL::SrcImageDescriptor is deprecated since 0.04b; Use LLGL::ImageView instead!", "ImageView") SrcImageDescriptor
{
    SrcImageDescriptor() = default;
    SrcImageDescriptor(const SrcImageDescriptor&) = default;

    inline SrcImageDescriptor(ImageFormat format, DataType dataType, const void* data, std::size_t dataSize) :
        format   { format   },
        dataType { dataType },
        data     { data     },
        dataSize { dataSize }
    {
    }

    inline SrcImageDescriptor(const ImageView& view) :
        format   { view.format   },
        dataType { view.dataType },
        data     { view.data     },
        dataSize { view.dataSize }
    {
    }

    inline operator ImageView() const
    {
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4996)
        #endif
        return ImageView{ format, dataType, data, dataSize };
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif
    }

    ImageFormat format      = ImageFormat::RGBA;
    DataType    dataType    = DataType::UInt8;
    const void* data        = nullptr;
    std::size_t dataSize    = 0;
};

struct LLGL_DEPRECATED("LLGL::DstImageDescriptor is deprecated since 0.04b; Use LLGL::MutableImageView instead!", "MutableImageView") DstImageDescriptor
{
    DstImageDescriptor() = default;
    DstImageDescriptor(const DstImageDescriptor&) = default;

    inline DstImageDescriptor(ImageFormat format, DataType dataType, void* data, std::size_t dataSize) :
        format   { format   },
        dataType { dataType },
        data     { data     },
        dataSize { dataSize }
    {
    }

    inline DstImageDescriptor(const MutableImageView& view) :
        format   { view.format   },
        dataType { view.dataType },
        data     { view.data     },
        dataSize { view.dataSize }
    {
    }

    inline operator MutableImageView() const
    {
        #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable : 4996)
        #endif
        return MutableImageView{ format, dataType, data, dataSize };
        #ifdef _MSC_VER
        #pragma warning(pop)
        #endif
    }

    ImageFormat format      = ImageFormat::RGBA;
    DataType    dataType    = DataType::UInt8;
    void*       data        = nullptr;
    std::size_t dataSize    = 0;
};


/* ----- Functions ----- */

/**
\defgroup group_image_util Image utility functions to classify and convert image data.
\addtogroup group_image_util
@{
*/

/**
\brief Converts the image format and data type of the source image (only uncompressed color formats).
\param[in] srcImageView Specifies the source image view.
\param[out] dstImageView Specifies the destination image view.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is less than 2, no multi-threading is used. If this is equal to \c LLGL_MAX_THREAD_COUNT,
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return True if any conversion was necessary. Otherwise, no conversion was necessary and the destination buffer is not modified!
\note Compressed images and depth-stencil images cannot be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination.
\throw std::invalid_argument If a depth-stencil format is specified either as source or destination.
\throw std::invalid_argument If the source buffer size is not a multiple of the source data type size times the image format size.
\throw std::invalid_argument If the source buffer is a null pointer.
\throw std::invalid_argument If the destination buffer size does not match the required output buffer size.
\throw std::invalid_argument If the destination buffer is a null pointer.
\see LLGL_MAX_THREAD_COUNT
\see GetMemoryFootprint
*/
LLGL_EXPORT bool ConvertImageBuffer(
    const ImageView&        srcImageView,
    const MutableImageView& dstImageView,
    unsigned                threadCount = 0
);

/**
\brief Converst the image format and data type of the source image (only uncompressed color formats) and returns the new generated image buffer.
\param[in] srcImageView Specifies the source image view.
\param[in] dstFormat Specifies the destination image format.
\param[in] dstDataType Specifies the destination image data type.
\param[in] threadCount Specifies the number of threads to use for conversion.
If this is less than 2, no multi-threading is used. If this is equal to \c LLGL_MAX_THREAD_COUNT,
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return Byte buffer with the converted image data or null if no conversion is necessary.
This can be casted to the respective target data type (e.g. <code>unsigned char</code>, <code>int</code>, <code>float</code> etc.).
\note Compressed images and depth-stencil images cannot be converted.
\throw std::invalid_argument If a compressed image format is specified either as source or destination.
\throw std::invalid_argument If a depth-stencil format is specified either as source or destination.
\throw std::invalid_argument If the source buffer size is not a multiple of the source data type size times the image format size.
\throw std::invalid_argument If the source buffer is a null pointer.
\see LLGL_MAX_THREAD_COUNT
\see GetMemoryFootprint
*/
LLGL_EXPORT DynamicByteArray ConvertImageBuffer(
    const ImageView&    srcImageView,
    ImageFormat         dstFormat,
    DataType            dstDataType,
    unsigned            threadCount = 0
);

//! \deprecated Since 0.04b; Use second version with explicit compression format instead!
LLGL_DEPRECATED("This version of DecompressImageBufferToRGBA8UNorm() is deprecated since 0.04b; Use second version with explicit compression format instead!")
LLGL_EXPORT DynamicByteArray DecompressImageBufferToRGBA8UNorm(
    const ImageView&    srcImageView,
    const Extent2D&     extent,
    unsigned            threadCount = 0
);

/**
\brief Decompresses the specified image buffer to RGBA format with 8-bit unsigned normalized integers.
\param[in] srcImageView Specifies the source image image.
\param[in] extent Specifies the image extent. This is required as most compression formats work in block sizes.
\param[in] threadCount Specifies the number of threads to use for decompression.
If this is less than 2, no multi-threading is used. If this is equal to \c LLGL_MAX_THREAD_COUNT,
the maximal count of threads the system supports will be used (e.g. 4 on a quad-core processor). By default 0.
\return Byte buffer with the decompressed image data or null if the compression format is not supported for decompression.
*/
LLGL_EXPORT DynamicByteArray DecompressImageBufferToRGBA8UNorm(
    Format              compressedFormat,
    const ImageView&    srcImageView,
    const Extent2D&     extent,
    unsigned            threadCount = 0
);

/**
\brief Copies an image buffer region from the source buffer to the destination buffer.
\param[out] dstImageView Specifies the destination image view.
\param[in] dstOffset Specifies the 3D offset of the destination image.
\param[in] dstRowStride Specifies the number of pixels for each row in the destination image.
\param[in] dstLayerStride Specifies the number of pixels for each slice in the destination image.
\param[in] srcImageView Specifies the source image view.
\param[in] srcOffset Specifies the 3D offset of the source image.
\param[in] srcRowStride Specifies the number of pixels for each row in the source image.
\param[in] srcLayerStride Specifies the number of pixels for each slice in the source image.
\param[in] extent Specifies the region extent to be copied.
\remarks Only performs a bitwise copy. No blending or other operation is performed.
\throw std::invalid_argument If the destination buffer is a null pointer.
\throw std::invalid_argument If the destination buffer size does not match the required output buffer size.
\throw std::invalid_argument If the source buffer is a null pointer.
\throw std::invalid_argument If the source buffer size is not a multiple of the source data type size times the image format size.
\throw std::invalid_argument If source and destination image descriptors do not have the same format and data type.
\throw std::out_of_range If \c srcOffset plus \c extent is outside the boundary of the source image.
\throw std::out_of_range If \c dstOffset plus \c extent is outside the boundary of the destination image.
*/
LLGL_EXPORT void CopyImageBufferRegion(
    // Destination
    const MutableImageView& dstImageView,
    const Offset3D&         dstOffset,
    std::uint32_t           dstRowStride,
    std::uint32_t           dstLayerStride,

    // Source
    const ImageView&        srcImageView,
    const Offset3D&         srcOffset,
    std::uint32_t           srcRowStride,
    std::uint32_t           srcLayerStride,

    // Region
    const Extent3D&         extent
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
const float fillColor[4] = { 1.0f, 1.0f, 0.0f, 0.5f };
auto imageBuffer = LLGL::GenerateImageBuffer(
    LLGL::ImageFormat::RGBA,
    LLGL::DataType::UInt8,
    512 * 512,
    fillColor
);
\endcode
*/
LLGL_EXPORT DynamicByteArray GenerateImageBuffer(
    ImageFormat format,
    DataType    dataType,
    std::size_t imageSize,
    const float fillColor[4]
);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
