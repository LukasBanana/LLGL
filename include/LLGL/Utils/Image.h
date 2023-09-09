/*
 * Image.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IMAGE_H
#define LLGL_IMAGE_H


#include <LLGL/Export.h>
#include <LLGL/Types.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/Utils/ColorRGBA.h>


namespace LLGL
{


/**
\brief Utility class to manage the storage and attributes of an image.

This class is not required for any interaction with the render system.
It can be used as utility to handle 2D and 3D image data before passing it to a hardware texture.
\remarks This class holds the ownership of an image buffer and its attributes.
The primary functions are implemented as global functions like <code>GenerateImageBuffer</code> for instance.
\note All image operations of this class do NOT make use of hardware acceleration.
\see GenerateImageBuffer
\see ConvertImageBuffer
*/
class LLGL_EXPORT Image
{

    public:

        /* ----- Common ----- */

        Image() = default;

        /**
        \brief Constructor to initialize the image with a format, data type, and extent.
        \note The image buffer will be uninitialized!
        \see Fill
        */
        Image(const Extent3D& extent, const ImageFormat format, const DataType dataType);

        /**
        \brief Constructor to initialize the image with a format, data type, and extent. The image buffer will be filled with the specified color.
        \see GenerateImageBuffer
        */
        Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, const ColorRGBAf& fillColor);

        /**
        \brief Constructor to initialize the image with all atributes, including the image buffer specified by the 'data' parameter.
        \note If the specified data does not manage an image buffer of the specified extent and format, the behavior is undefined.
        \see Reset(const Extent3D&, const ImageFormat, const DataType, DynamicByteArray&&)
        */
        Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, DynamicByteArray&& data);

        //! Copy constructor which copies the entire image buffer from the specified source image.
        Image(const Image& rhs);

        //! Move constructor which takes the ownership of the specified source image.
        Image(Image&& rhs);

        /* ----- Operators ----- */

        //! Copy operator which copies the entire image buffer and attributes.
        Image& operator = (const Image& rhs);

        //! Move operator which takes the ownership of the image buffer.
        Image& operator = (Image&& rhs);

        /* ----- Storage ----- */

        /**
        \brief Converts the image format and data type.
        \see ConvertImageBuffer
        */
        void Convert(const ImageFormat format, const DataType dataType, unsigned threadCount = 0);

        /**
        \brief Resizes the image and resets the image buffer.
        \param[in] extent Specifies the new image size.
        \note The new image buffer will be uninitialized!
        */
        void Resize(const Extent3D& extent);

        /**
        \brief Resizes the image and initializes the new pixels with the specified color.
        \param[in] extent Specifies the new image size.
        \param[in] fillColor Specifies the color to fill the pixels with.
        \see GenerateImageBuffer
        */
        void Resize(const Extent3D& extent, const ColorRGBAf& fillColor);

        /**
        \brief Resizes the image, moves the previous pixels by an offset, and initializes the new pixels outside the previous extent with the specified color.
        \param[in] extent Specifies the new image size.
        \param[in] fillColor Specifies the color to fill the pixels with that are outside the previous extent.
        \param[in] offset Specifies the offset to move the previous pixels to. This will be clamped if it exceeds the image area.
        \see GenerateImageBuffer
        */
        void Resize(const Extent3D& extent, const ColorRGBAf& fillColor, const Offset3D& offset);

        //! Swaps all attributes with the specified image.
        void Swap(Image& rhs);

        //! Resets all image attributes to its default values.
        void Reset();

        /**
        \brief Resets all image attributes to the specified values.
        \note If the specified data does not manage an image buffer of the specified extent and format, the behavior is undefined.
        \see GenerateImageBuffer
        */
        void Reset(const Extent3D& extent, const ImageFormat format, const DataType dataType, DynamicByteArray&& data);

        //! Releases the ownership of the image buffer and resets all attributes.
        DynamicByteArray Release();

        /* ----- Pixels ----- */

        /**
        \brief Copies a region of the specified source image into this image.
        \param[in] dstRegionOffset Specifies the offset within the destination image (i.e. this Image instance). This can also be outside of the image area.
        \param[in] srcImage Specifies the source image whose region is to be copied. This must have the same format and data type as this image.
        If the source image is the same object as this image and the destination and source regions overlap, an internal temporary copy is allocated for reading the data.
        \param[in] srcRegionOffset Specifies the offset within the source image. This will be clamped if it exceeds the source image area.
        \param[in] srcRegionExtent Specifies the extent of the region to copy. This will be clamped if it exceeds the source or destination image area.
        \remarks If one of the region offsets is clamped, the region extent will be adjusted respectively.
        If the source image has a different format or data type compared to this image, the function has no effect.
        \see ConvertImageBuffer
        */
        void Blit(Offset3D dstRegionOffset, const Image& srcImage, Offset3D srcRegionOffset, Extent3D srcRegionExtent);

        /**
        \brief Reads a region of pixels from this image into the destination image buffer specified by 'imageDesc'.
        \param[in] offset Specifies the region offset within this image to read from.
        \param[in] extent Specifies the region extent within this image to read from.
        \param[in] imageDesc Specifies the destination image descriptor to write the region to.
        If the 'data' member of this descriptor is null or if the sub-image region is not inside the image, this function has no effect.
        \param[in] threadCount Specifies the number of threads to use if the data needs to be converted (see ConvertImageBuffer for more details). By default 0.
        \remarks To read a single pixel, use the following code example:
        \code
        LLGL::ColorRGBAub ReadSinglePixelRGBAub(const LLGL::Image& image, const LLGL::Offset3D& position) {
            LLGL::ColorRGBAub pixelColor;
            const DstImageDescriptor imageDesc { LLGL::ImageFormat::RGBA, LLGL::DataType::UInt8, &pixelColor, sizeof(pixelColor) };
            image.ReadPixels(position, { 1, 1, 1 }, imageDesc);
            return pixelColor;
        }
        \endcode
        \throws std::invalid_argument If the 'data' member of the image descriptor is non-null, the sub-image region is inside the image,
        but the 'dataSize' member of the image descriptor is too small.
        \see IsRegionInside
        \see ConvertImageBuffer
        */
        void ReadPixels(const Offset3D& offset, const Extent3D& extent, const DstImageDescriptor& imageDesc, unsigned threadCount = 0) const;

        /**
        \brief Writes a region of pixels to this image from the source image buffer specified by 'imageDesc'.
        \param[in] offset Specifies the region offset within this image to write to.
        \param[in] extent Specifies the region extent within this image to write to.
        \param[in] imageDesc Specifies the source image descriptor to read the region from.
        If the 'data' member of this descriptor is null or if the sub-image region is not inside the image, this function has no effect.
        \param[in] threadCount Specifies the number of threads to use if the data needs to be converted (see ConvertImageBuffer for more details). By default 0.
        \see IsRegionInside
        \see ConvertImageBuffer
        */
        void WritePixels(const Offset3D& offset, const Extent3D& extent, const SrcImageDescriptor& imageDesc, unsigned threadCount = 0);

        /* ----- Attributes ----- */

        //! Returns a source image descriptor for this image with read-only access to the image data.
        SrcImageDescriptor GetSrcDesc() const;

        //! Returns a destination image descriptor for this image with read/write access to the image data.
        DstImageDescriptor GetDstDesc();

        //! Returns the extent of the image as 3D vector.
        inline const Extent3D& GetExtent() const
        {
            return extent_;
        }

        //! Returns the format for each pixel. By default ImageFormat::RGBA.
        inline ImageFormat GetFormat() const
        {
            return format_;
        }

        //! Returns the data type for each pixel component. By default DataType::UInt8.
        inline DataType GetDataType() const
        {
            return dataType_;
        }

        //! Returns the image data buffer as constant raw pointer.
        inline const void* GetData() const
        {
            return data_.get();
        }

        //! Returns the image data buffer as raw pointer.
        inline void* GetData()
        {
            return data_.get();
        }

        /**
        \brief Returns the size (in bytes) for each pixel.
        \see GetFormat
        \see GetDataType
        \see GetMemoryFootprint
        */
        std::uint32_t GetBytesPerPixel() const;

        //! Returns the stride (in bytes) for each row.
        std::uint32_t GetRowStride() const;

        //! Returns the stride (in bytes) for each depth slice.
        std::uint32_t GetDepthStride() const;

        /**
        \brief Returns the number of pixels this image has.
        \remarks This is equivalent to the following code example:
        \code
        const auto& extent = myImage.GetExtent();
        return extent.width * extent.height * extent.depth;
        \endcode
        \see GetExtent
        */
        std::uint32_t GetNumPixels() const;

        /**
        \brief Returns the size (in bytes) of the image buffer.
        \see GetBytesPerPixel
        \see GetNumPixels
        */
        std::uint32_t GetDataSize() const;

        //! Returns true if the specified sub-image region is inside the image.
        bool IsRegionInside(const Offset3D& offset, const Extent3D& extent) const;

    private:

        void ResetAttributes();

        std::size_t GetDataPtrOffset(const Offset3D& offset) const;

        void ClampRegion(Offset3D& offset, Extent3D& extent) const;

    private:

        Extent3D            extent_;
        ImageFormat         format_     = ImageFormat::RGBA;
        DataType            dataType_   = DataType::UInt8;
        DynamicByteArray    data_;

};


} // /namespace LLGL


#endif



// ================================================================================
