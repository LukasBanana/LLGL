/*
 * Image.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMAGE_H
#define LLGL_IMAGE_H


#include "Export.h"
#include "Types.h"
#include "ImageFlags.h"
#include "SamplerFlags.h"


namespace LLGL
{


/**
\brief Utility class to manage the storage and attributes of an image.

This class is not required for any interaction with the render system.
It can be used as utility to handle 2D and 3D image data before passing it to a hardware texture.
\remarks This class holds the ownership of an image buffer and its attributes.
The primary functions are implemented as global functions like GenerateImageBuffer for instance.
\note All image operations of this class do NOT make use of hardware acceleration.
\see GenerateImageBuffer
\see ConvertImageBuffer
*/
class LLGL_EXPORT Image
{

    public:

        /* ----- Common ----- */

        Image() = default;

        //! Copy constructor which copies the entire image buffer from the specified source image.
        Image(const Image& rhs);

        //! Move constructor which takes the ownership of the specified source image.
        Image(Image&& rhs);

        /* ----- Storage ----- */

        /**
        \brief Converts the image format and data type.
        \see ConvertImageBuffer
        */
        void Convert(const ImageFormat format, const DataType dataType, std::size_t threadCount = 0);

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
        \brief GenerateImageBuffer
        */
        void Resize(const Extent3D& extent, const ColorRGBAd& fillColor);

        /**
        \brief Resizes the image, moves the previous pixels by an offset, and initializes the new pixels outside the previous extent with the specified color.
        \param[in] extent Specifies the new image size.
        \param[in] fillColor Specifies the color to fill the pixels with that are outside the previous extent.
        \param[in] offset Specifies the offset to move the previous pixels to.
        \brief GenerateImageBuffer
        \todo Not implemented yet.
        */
        void Resize(const Extent3D& extent, const ColorRGBAd& fillColor, const Offset2D& offset);

        /**
        \brief Resizes the image and resamples the pixels from the previous image buffer.
        \param[in] extent Specifies the new image size.
        \param[in] filter Specifies the sampling filter.
        \brief GenerateImageBuffer
        \todo Not implemented yet.
        */
        void Resize(const Extent3D& extent, const TextureFilter filter);

        /* ----- Pixels ----- */

        /**
        \brief Copies a region of the specified source image into this image.
        \param[in] srcImage Specifies the source image whose region is to be copied.
        \param[in] offset Specifies the offset where to copy the region into this image. This can also be outside of the image.
        \param[in] extent Specifies the extent of the region. This will be clamped if it exceeds the maximal possible extent.
        \todo Not implemented yet.
        */
        void Blit(const Image& srcImage, Offset3D offset, Extent3D extent);

        /**
        \brief Fills a region of this image by the specified color.
        \param[in] offset Specifies the offset where the region begins.
        \param[in] extent Specifies the extent of the region.
        \param[in] fillColor Specifies the color to fill the region with.
        \todo Not implemented yet.
        */
        void Fill(Offset3D offset, Extent3D extent, const ColorRGBAd& fillColor);

        /* ----- Attributes ----- */

        //! Returns a source image descriptor for this image with read-only access to the image data.
        SrcImageDescriptor QuerySrcDesc() const;

        //! Returns a destination image descriptor for this image with read/write access to the image data.
        DstImageDescriptor QueryDstDesc();

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

        //! Returns the size (in bytes) of the image buffer.
        std::uint32_t GetDataSize() const;

        /**
        \brief Returns the number of pixels this image has.
        \remarks This is equivalent to the following code example:
        \code
        const auto& extent = myImage.GetExtent();
        return extent.width * extent.height * extent.depth;
        \endcode
        */
        std::uint32_t GetNumPixels() const;

        //! Returns the extent of the image as 3D vector.
        inline const Extent3D& GetExtent() const
        {
            return extent_;
        }

        //! Releases the ownership of the image buffer and resets all attributes.
        ByteBuffer Release();

    private:

        ImageFormat format_     = ImageFormat::RGBA;
        DataType    dataType_   = DataType::UInt8;
        ByteBuffer  data_;
        Extent3D    extent_;

};


} // /namespace LLGL


#endif



// ================================================================================
