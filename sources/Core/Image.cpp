/*
 * Image.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Utils/Image.h>
#include "ImageUtils.h"
#include "Exception.h"
#include "PrintfUtils.h"
#include <algorithm>
#include <string.h>


namespace LLGL
{


/* ----- Common ----- */

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType) :
    extent_   { extent                           },
    format_   { format                           },
    dataType_ { dataType                         },
    data_     { GetDataSize(), UninitializeTag{} }
{
}

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, const ColorRGBAf& fillColor) :
    extent_   { extent                                                                 },
    format_   { format                                                                 },
    dataType_ { dataType                                                               },
    data_     { GenerateImageBuffer(format, dataType, GetNumPixels(), fillColor.Ptr()) }
{
}

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, DynamicByteArray&& data) :
    extent_   { extent          },
    format_   { format          },
    dataType_ { dataType        },
    data_     { std::move(data) }
{
}

Image::Image(const Image& rhs) :
    Image { rhs.GetExtent(), rhs.GetFormat(), rhs.GetDataType() }
{
    ::memcpy(data_.get(), rhs.data_.get(), rhs.GetDataSize());
}

Image::Image(Image&& rhs) noexcept :
    extent_   { rhs.extent_          },
    format_   { rhs.format_          },
    dataType_ { rhs.dataType_        },
    data_     { std::move(rhs.data_) }
{
    rhs.ResetAttributes();
}

/* ----- Operators ----- */

Image& Image::operator = (const Image& rhs)
{
    extent_     = rhs.GetExtent();
    format_     = rhs.GetFormat();
    dataType_   = rhs.GetDataType();
    data_       = DynamicByteArray{ GetDataSize(), UninitializeTag{} };
    ::memcpy(data_.get(), rhs.data_.get(), rhs.GetDataSize());
    return *this;
}

Image& Image::operator = (Image&& rhs)
{
    Reset(rhs.GetExtent(), rhs.GetFormat(), rhs.GetDataType(), std::move(rhs.data_));
    rhs.ResetAttributes();
    return *this;
}

/* ----- Storage ----- */

void Image::Convert(const ImageFormat format, const DataType dataType, unsigned threadCount)
{
    /* Convert image buffer (if necessary) */
    if (data_)
    {
        if (DynamicByteArray convertedData = ConvertImageBuffer(GetView(), format, dataType, threadCount))
            data_ = std::move(convertedData);
    }

    /* Store new attributes */
    format_     = format;
    dataType_   = dataType;
}

void Image::Resize(const Extent3D& extent)
{
    /* Allocate new image buffer or release it if the extent is zero */
    extent_ = extent;
    if (extent.width > 0 && extent.height > 0 && extent.depth > 0)
        data_ = DynamicByteArray{ GetDataSize(), UninitializeTag{} };
    else
        data_.clear();
}

void Image::Resize(const Extent3D& extent, const ColorRGBAf& fillColor)
{
    if (extent_ != extent)
    {
        /* Generate new image buffer with fill color */
        extent_ = extent;
        data_   = GenerateImageBuffer(GetFormat(), GetDataType(), GetNumPixels(), fillColor.Ptr());
    }
    else
    {
        /* Clear image by fill color */
        //Fill({ 0, 0, 0 }, extent, fillColor);
    }
}

void Image::Resize(const Extent3D& extent, const ColorRGBAf& fillColor, const Offset3D& offset)
{
    if (extent != GetExtent())
    {
        /* Store ownership of current image buffer in temporary image */
        Image prevImage;

        prevImage.extent_   = GetExtent();
        prevImage.format_   = GetFormat();
        prevImage.dataType_ = GetDataType();
        prevImage.data_     = std::move(data_);

        if ( extent.width  > GetExtent().width  ||
             extent.height > GetExtent().height ||
             extent.depth  > GetExtent().depth )
        {
            /* Resize image buffer with fill color */
            extent_ = extent;
            data_   = GenerateImageBuffer(GetFormat(), GetDataType(), GetNumPixels(), fillColor.Ptr());
        }
        else
        {
            /* Resize image buffer with uninitialized image buffer */
            extent_ = extent;
            data_   = DynamicByteArray{ GetDataSize(), UninitializeTag{} };
        }

        /* Copy previous image into new image */
        Blit(offset, prevImage, { 0, 0, 0 }, prevImage.GetExtent());
    }
}

void Image::Swap(Image& rhs)
{
    std::swap(extent_,   rhs.extent_  );
    std::swap(format_,   rhs.format_  );
    std::swap(dataType_, rhs.dataType_);
    std::swap(data_,     rhs.data_    );
}

void Image::Reset()
{
    ResetAttributes();
    data_.clear();
}

void Image::Reset(const Extent3D& extent, const ImageFormat format, const DataType dataType, DynamicByteArray&& data)
{
    extent_     = extent;
    format_     = format;
    dataType_   = dataType;
    data_       = std::move(data);
}

DynamicByteArray Image::Release()
{
    ResetAttributes();
    return std::move(data_);
}

/* ----- Pixels ----- */

static bool ShiftNegative1DRegion(std::int32_t& dstOffset, std::uint32_t dstExtent, std::int32_t& srcOffset, std::uint32_t& srcExtent)
{
    if (dstOffset < 0)
    {
        const std::uint32_t dstOffsetInv = static_cast<std::uint32_t>(-dstOffset);
        if (dstOffsetInv < srcExtent)
        {
            /* Reduce source region extent, and clamp destination offset to zero */
            srcExtent -= dstOffsetInv;
            srcOffset -= dstOffset;
            dstOffset = 0;
        }
        else
        {
            /* Shift operation will set the extent to zero, so no blitting is necessary */
            return false;
        }
    }

    if (static_cast<std::uint32_t>(dstOffset) + srcExtent > dstExtent)
    {
        const std::uint32_t shift = static_cast<std::uint32_t>(dstOffset) + srcExtent - dstExtent;
        if (shift < srcExtent)
        {
            /* Reduce source region extent */
            srcExtent -= shift;
        }
        else
        {
            /* Shift operation will set the extent to zero, so no blitting is necessary */
            return false;
        }
    }

    return true;
}

static bool Overlap1DRegion(std::int32_t dstOffset, std::int32_t srcOffset, std::uint32_t extent)
{
    const std::uint32_t dstOffsetMin = static_cast<std::uint32_t>(dstOffset);
    const std::uint32_t dstOffsetMax = dstOffsetMin + extent;

    const std::uint32_t srcOffsetMin = static_cast<std::uint32_t>(srcOffset);
    const std::uint32_t srcOffsetMax = srcOffsetMin + extent;

    return (dstOffsetMin <= srcOffsetMax && dstOffsetMax >= srcOffsetMin);
}

static bool Overlap3DRegion(const Offset3D& dstOffset, const Offset3D& srcOffset, const Extent3D& extent)
{
    return
    (
        Overlap1DRegion(dstOffset.x, srcOffset.x, extent.width ) &&
        Overlap1DRegion(dstOffset.y, srcOffset.y, extent.height) &&
        Overlap1DRegion(dstOffset.z, srcOffset.z, extent.depth )
    );
}

void Image::Blit(Offset3D dstRegionOffset, const Image& srcImage, Offset3D srcRegionOffset, Extent3D srcRegionExtent)
{
    if (GetFormat() == srcImage.GetFormat() && GetDataType() == srcImage.GetDataType())
    {
        /* First clamp source region to source image dimension */
        srcImage.ClampRegion(srcRegionOffset, srcRegionExtent);

        /* Then shift negative destination region */
        if ( ShiftNegative1DRegion(dstRegionOffset.x, GetExtent().width,  srcRegionOffset.x, srcRegionExtent.width ) &&
             ShiftNegative1DRegion(dstRegionOffset.y, GetExtent().height, srcRegionOffset.y, srcRegionExtent.height) &&
             ShiftNegative1DRegion(dstRegionOffset.z, GetExtent().depth,  srcRegionOffset.z, srcRegionExtent.depth ) )
        {
            /* Check if a temporary copy of the source image must be allocated */
            Image srcImageTemp;
            const Image* srcImageRef = &srcImage;

            if (srcImageRef == this && Overlap3DRegion(dstRegionOffset, srcRegionOffset, srcRegionExtent))
            {
                /* Copy source image */
                srcImageTemp = *this;
                srcImageRef = &srcImageTemp;
            }

            /* Copy image buffer region */
            const Extent3D srcExtent = srcImageRef->GetExtent();
            const Extent3D dstExtent = GetExtent();

            CopyImageBufferRegion(
                GetMutableView(),
                dstRegionOffset,
                dstExtent.width,
                dstExtent.width * dstExtent.height,
                srcImageRef->GetView(),
                srcRegionOffset,
                srcExtent.width,
                srcExtent.width * srcExtent.height,
                srcRegionExtent
            );
        }
    }
}

static std::size_t GetRequiredImageDataSize(const Extent3D& extent, const ImageFormat format, const DataType dataType)
{
    return GetMemoryFootprint(format, dataType, extent.width * extent.height * extent.depth);
}

static void ValidateImageDataSize(const Extent3D& extent, const MutableImageView& imageView)
{
    const std::uint64_t requiredDataSize = GetRequiredImageDataSize(extent, imageView.format, imageView.dataType);
    if (imageView.dataSize < requiredDataSize)
    {
        LLGL_TRAP(
            "data size of destinaton image descriptor is too small: 0x%016" PRIX64 " is required, but only 0x%016" PRIX64 " was specified",
            requiredDataSize, imageView.dataSize
        );
    }
}

static void ValidateImageDataSize(const Extent3D& extent, const ImageView& imageView)
{
    const std::size_t requiredDataSize = GetRequiredImageDataSize(extent, imageView.format, imageView.dataType);
    if (imageView.dataSize < requiredDataSize)
    {
        LLGL_TRAP(
            "data size of source image descriptor is too small: 0x%016" PRIX64 " is required, but only 0x%016" PRIX64 " was specified",
            requiredDataSize, imageView.dataSize
        );
    }
}

void Image::ReadPixels(const Offset3D& offset, const Extent3D& extent, const MutableImageView& imageView, unsigned threadCount) const
{
    if (imageView.data && IsRegionInside(offset, extent))
    {
        /* Validate required size */
        ValidateImageDataSize(extent, imageView);

        /* Get source image parameters */
        const std::uint32_t bpp             = GetBytesPerPixel();
        const std::uint32_t srcRowStride    = bpp * GetExtent().width;
        const std::uint32_t srcDepthStride  = srcRowStride * GetExtent().height;
        const char*         src             = data_.get() + GetDataPtrOffset(offset);

        if (GetFormat() == imageView.format && GetDataType() == imageView.dataType)
        {
            /* Get destination image parameters */
            const std::uint32_t dstRowStride    = bpp * extent.width;
            const std::uint32_t dstDepthStride  = dstRowStride * extent.height;
            char*               dst             = reinterpret_cast<char*>(imageView.data);

            /* Blit region into destination image */
            BitBlit(
                extent, bpp,
                dst, dstRowStride, dstDepthStride,
                src, srcRowStride, srcDepthStride
            );
        }
        else
        {
            /* Copy region into temporary sub-image */
            Image subImage{ extent, GetFormat(), GetDataType() };

            BitBlit(
                extent, bpp,
                reinterpret_cast<char*>(subImage.GetData()), subImage.GetRowStride(), subImage.GetDepthStride(),
                src, srcRowStride, srcDepthStride
            );

            /* Convert sub-image */
            subImage.Convert(imageView.format, imageView.dataType, threadCount);

            /* Copy sub-image into output data */
            ::memcpy(imageView.data, subImage.GetData(), imageView.dataSize);
        }
    }
}

void Image::WritePixels(const Offset3D& offset, const Extent3D& extent, const ImageView& imageView, unsigned threadCount)
{
    if (imageView.data && IsRegionInside(offset, extent))
    {
        /* Validate required size */
        ValidateImageDataSize(extent, imageView);

        /* Get destination image parameters */
        const std::uint32_t bpp             = GetBytesPerPixel();
        const std::uint32_t dstRowStride    = bpp * GetExtent().width;
        const std::uint32_t dstDepthStride  = dstRowStride * GetExtent().height;
        char*               dst             = data_.get() + GetDataPtrOffset(offset);

        if (GetFormat() == imageView.format && GetDataType() == imageView.dataType)
        {
            /* Get source image parameters */
            const std::uint32_t srcRowStride    = bpp * extent.width;
            const std::uint32_t srcDepthStride  = srcRowStride * extent.height;
            const char*         src             = reinterpret_cast<const char*>(imageView.data);

            /* Blit source image into region */
            BitBlit(
                extent, bpp,
                dst, dstRowStride, dstDepthStride,
                src, srcRowStride, srcDepthStride
            );
        }
        else
        {
            /* Copy input data into sub-image into */
            Image subImage{ extent, imageView.format, imageView.dataType };
            ::memcpy(subImage.GetData(), imageView.data, imageView.dataSize);

            /* Convert sub-image */
            subImage.Convert(GetFormat(), GetDataType(), threadCount);

            /* Copy temporary sub-image into region */
            BitBlit(
                extent, bpp,
                dst, dstRowStride, dstDepthStride,
                reinterpret_cast<const char*>(subImage.GetData()), subImage.GetRowStride(), subImage.GetDepthStride()
            );
        }
    }
}

/* ----- Attributes ----- */

ImageView Image::GetView() const
{
    ImageView imageView;
    {
        imageView.format    = GetFormat();
        imageView.dataType  = GetDataType();
        imageView.data      = GetData();
        imageView.dataSize  = GetDataSize();
    }
    return imageView;
}

MutableImageView Image::GetMutableView()
{
    MutableImageView imageView;
    {
        imageView.format    = GetFormat();
        imageView.dataType  = GetDataType();
        imageView.data      = GetData();
        imageView.dataSize  = GetDataSize();
    }
    return imageView;
}

std::uint32_t Image::GetBytesPerPixel() const
{
    return static_cast<std::uint32_t>(GetMemoryFootprint(format_, dataType_, 1));
}

std::uint32_t Image::GetRowStride() const
{
    return (GetBytesPerPixel() * GetExtent().width);
}

std::uint32_t Image::GetDepthStride() const
{
    return (GetRowStride() * GetExtent().height);
}

std::uint32_t Image::GetDataSize() const
{
    return (GetNumPixels() * GetBytesPerPixel());
}

std::uint32_t Image::GetNumPixels() const
{
    return (extent_.width * extent_.height * extent_.depth);
}

static bool Is1DRegionValid(std::int32_t offset, std::uint32_t extent, std::uint32_t limit)
{
    return (offset >= 0 && static_cast<std::uint32_t>(offset) + extent <= limit);
}

bool Image::IsRegionInside(const Offset3D& offset, const Extent3D& extent) const
{
    return
    (
        Is1DRegionValid(offset.x, extent.width , extent_.width ) &&
        Is1DRegionValid(offset.y, extent.height, extent_.height) &&
        Is1DRegionValid(offset.z, extent.depth , extent_.depth )
    );
}


/*
 * ======= Private: =======
 */

void Image::ResetAttributes() noexcept
{
    format_     = ImageFormat::RGBA;
    dataType_   = DataType::UInt8;
    extent_     = { 0, 0, 0 };
}

std::size_t Image::GetDataPtrOffset(const Offset3D& offset) const
{
    const std::size_t bpp   = static_cast<std::size_t>(GetBytesPerPixel());
    const std::size_t x     = static_cast<std::size_t>(offset.x);
    const std::size_t y     = static_cast<std::size_t>(offset.y);
    const std::size_t z     = static_cast<std::size_t>(offset.z);
    const std::size_t w     = static_cast<std::size_t>(GetExtent().width);
    const std::size_t h     = static_cast<std::size_t>(GetExtent().height);
    return (bpp * (x + (y + z * h) * w));
}

void Image::ClampRegion(Offset3D& offset, Extent3D& extent) const
{
    offset.x        = std::max(offset.x, 0);
    offset.y        = std::max(offset.y, 0);
    offset.z        = std::max(offset.z, 0);

    extent.width    = std::min(extent.width, GetExtent().width);
    extent.height   = std::min(extent.height, GetExtent().height);
    extent.depth    = std::min(extent.depth, GetExtent().depth);
}


} // /namespace LLGL



// ================================================================================
