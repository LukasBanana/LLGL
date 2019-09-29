/*
 * Image.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Image.h>
#include "ImageUtils.h"
#include <algorithm>
#include <string.h>


namespace LLGL
{


/* ----- Common ----- */

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType) :
    extent_   { extent                                        },
    format_   { format                                        },
    dataType_ { dataType                                      },
    data_     { GenerateEmptyByteBuffer(GetDataSize(), false) }
{
}

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, const ColorRGBAd& fillColor) :
    extent_   { extent                                                           },
    format_   { format                                                           },
    dataType_ { dataType                                                         },
    data_     { GenerateImageBuffer(format, dataType, GetNumPixels(), fillColor) }
{
}

Image::Image(const Extent3D& extent, const ImageFormat format, const DataType dataType, ByteBuffer&& data) :
    extent_   { extent          },
    format_   { format          },
    dataType_ { dataType        },
    data_     { std::move(data) }
{
}

Image::Image(const Image& rhs) :
    Image { rhs.GetExtent(), rhs.GetFormat(), rhs.GetDataType() }
{
    std::copy(rhs.data_.get(), rhs.data_.get() + rhs.GetDataSize(), data_.get());
}

Image::Image(Image&& rhs) :
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
    data_       = GenerateEmptyByteBuffer(GetDataSize(), false);
    std::copy(rhs.data_.get(), rhs.data_.get() + rhs.GetDataSize(), data_.get());
    return *this;
}

Image& Image::operator = (Image&& rhs)
{
    Reset(rhs.GetExtent(), rhs.GetFormat(), rhs.GetDataType(), std::move(rhs.data_));
    rhs.ResetAttributes();
    return *this;
}

/* ----- Storage ----- */

void Image::Convert(const ImageFormat format, const DataType dataType, std::size_t threadCount)
{
    /* Convert image buffer (if necessary) */
    if (data_)
    {
        if (auto convertedData = ConvertImageBuffer(GetSrcDesc(), format, dataType, threadCount))
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
        data_ = GenerateEmptyByteBuffer(GetDataSize(), false);
    else
        data_.reset();
}

void Image::Resize(const Extent3D& extent, const ColorRGBAd& fillColor)
{
    if (extent_ != extent)
    {
        /* Generate new image buffer with fill color */
        extent_ = extent;
        data_   = GenerateImageBuffer(GetFormat(), GetDataType(), GetNumPixels(), fillColor);
    }
    else
    {
        /* Clear image by fill color */
        Fill({ 0, 0, 0 }, extent, fillColor);
    }
}

void Image::Resize(const Extent3D& extent, const ColorRGBAd& fillColor, const Offset3D& offset)
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
            data_   = GenerateImageBuffer(GetFormat(), GetDataType(), GetNumPixels(), fillColor);
        }
        else
        {
            /* Resize image buffer with uninitialized image buffer */
            extent_ = extent;
            data_   = GenerateEmptyByteBuffer(GetDataSize(), false);
        }

        /* Copy previous image into new image */
        Blit(offset, prevImage, { 0, 0, 0 }, prevImage.GetExtent());
    }
}

void Image::Resize(const Extent3D& extent, const SamplerFilter filter)
{
    //todo
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
    data_.reset();
}

void Image::Reset(const Extent3D& extent, const ImageFormat format, const DataType dataType, ByteBuffer&& data)
{
    extent_     = extent;
    format_     = format;
    dataType_   = dataType;
    data_       = std::move(data);
}

ByteBuffer Image::Release()
{
    ResetAttributes();
    return std::move(data_);
}

/* ----- Pixels ----- */

static bool ShiftNegative1DRegion(std::int32_t& dstOffset, std::uint32_t dstExtent, std::int32_t& srcOffset, std::uint32_t& srcExtent)
{
    if (dstOffset < 0)
    {
        auto dstOffsetInv = static_cast<std::uint32_t>(-dstOffset);
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
        auto shift = static_cast<std::uint32_t>(dstOffset) + srcExtent - dstExtent;
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
    auto dstOffsetMin = static_cast<std::uint32_t>(dstOffset);
    auto dstOffsetMax = dstOffsetMin + extent;

    auto srcOffsetMin = static_cast<std::uint32_t>(srcOffset);
    auto srcOffsetMax = srcOffsetMin + extent;

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
            const auto srcExtent = srcImageRef->GetExtent();
            const auto dstExtent = GetExtent();

            CopyImageBufferRegion(
                GetDstDesc(),
                dstRegionOffset,
                dstExtent.width,
                dstExtent.width * dstExtent.height,
                srcImageRef->GetSrcDesc(),
                srcRegionOffset,
                srcExtent.width,
                srcExtent.width * srcExtent.height,
                srcRegionExtent
            );
        }
    }
}

void Image::Fill(Offset3D offset, Extent3D extent, const ColorRGBAd& fillColor)
{
    //ClampRegion(offset, extent);

    //TODO
}

static std::size_t GetRequiredImageDataSize(const Extent3D& extent, const ImageFormat format, const DataType dataType)
{
    return static_cast<std::size_t>(ImageFormatSize(format) * DataTypeSize(dataType) * extent.width * extent.height * extent.depth);
}

static void ValidateImageDataSize(const Extent3D& extent, const DstImageDescriptor& imageDesc)
{
    const auto requiredDataSize = GetRequiredImageDataSize(extent, imageDesc.format, imageDesc.dataType);
    if (imageDesc.dataSize < requiredDataSize)
    {
        throw std::invalid_argument(
            "data size of destinaton image descriptor is too small (" + std::to_string(requiredDataSize) +
            " is required, but only " + std::to_string(imageDesc.dataSize) + " was specified)"
        );
    }
}

static void ValidateImageDataSize(const Extent3D& extent, const SrcImageDescriptor& imageDesc)
{
    const auto requiredDataSize = GetRequiredImageDataSize(extent, imageDesc.format, imageDesc.dataType);
    if (imageDesc.dataSize < requiredDataSize)
    {
        throw std::invalid_argument(
            "data size of source image descriptor is too small (" + std::to_string(requiredDataSize) +
            " is required, but only " + std::to_string(imageDesc.dataSize) + " was specified)"
        );
    }
}

void Image::ReadPixels(const Offset3D& offset, const Extent3D& extent, const DstImageDescriptor& imageDesc, std::size_t threadCount) const
{
    if (imageDesc.data && IsRegionInside(offset, extent))
    {
        /* Validate required size */
        ValidateImageDataSize(extent, imageDesc);

        /* Get source image parameters */
        const auto  bpp             = GetBytesPerPixel();
        const auto  srcRowStride    = bpp * GetExtent().width;
        const auto  srcDepthStride  = srcRowStride * GetExtent().height;
        auto        src             = data_.get() + GetDataPtrOffset(offset);

        if (GetFormat() == imageDesc.format && GetDataType() == imageDesc.dataType)
        {
            /* Get destination image parameters */
            const auto  dstRowStride    = bpp * extent.width;
            const auto  dstDepthStride  = dstRowStride * extent.height;
            auto        dst             = reinterpret_cast<char*>(imageDesc.data);

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
            Image subImage { extent, GetFormat(), GetDataType() };

            BitBlit(
                extent, bpp,
                reinterpret_cast<char*>(subImage.GetData()), subImage.GetRowStride(), subImage.GetDepthStride(),
                src, srcRowStride, srcDepthStride
            );

            /* Convert sub-image */
            subImage.Convert(imageDesc.format, imageDesc.dataType, threadCount);

            /* Copy sub-image into output data */
            ::memcpy(imageDesc.data, subImage.GetData(), imageDesc.dataSize);
        }
    }
}

void Image::WritePixels(const Offset3D& offset, const Extent3D& extent, const SrcImageDescriptor& imageDesc, std::size_t threadCount)
{
    if (imageDesc.data && IsRegionInside(offset, extent))
    {
        /* Validate required size */
        ValidateImageDataSize(extent, imageDesc);

        /* Get destination image parameters */
        const auto  bpp             = GetBytesPerPixel();
        const auto  dstRowStride    = bpp * GetExtent().width;
        const auto  dstDepthStride  = dstRowStride * GetExtent().height;
        auto        dst             = data_.get() + GetDataPtrOffset(offset);

        if (GetFormat() == imageDesc.format && GetDataType() == imageDesc.dataType)
        {
            /* Get source image parameters */
            const auto  srcRowStride    = bpp * extent.width;
            const auto  srcDepthStride  = srcRowStride * extent.height;
            auto        src             = reinterpret_cast<const char*>(imageDesc.data);

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
            Image subImage { extent, imageDesc.format, imageDesc.dataType };
            ::memcpy(subImage.GetData(), imageDesc.data, imageDesc.dataSize);

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

void Image::MirrorYZPlane()
{
    //TODO
}

void Image::MirrorXZPlane()
{
    //TODO
}

void Image::MirrorXYPlane()
{
    //TODO
}

/* ----- Attributes ----- */

SrcImageDescriptor Image::GetSrcDesc() const
{
    SrcImageDescriptor imageDesc;
    {
        imageDesc.format    = GetFormat();
        imageDesc.dataType  = GetDataType();
        imageDesc.data      = GetData();
        imageDesc.dataSize  = GetDataSize();
    }
    return imageDesc;
}

DstImageDescriptor Image::GetDstDesc()
{
    DstImageDescriptor imageDesc;
    {
        imageDesc.format    = GetFormat();
        imageDesc.dataType  = GetDataType();
        imageDesc.data      = GetData();
        imageDesc.dataSize  = GetDataSize();
    }
    return imageDesc;
}

std::uint32_t Image::GetBytesPerPixel() const
{
    return (ImageFormatSize(format_) * DataTypeSize(dataType_));
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

void Image::ResetAttributes()
{
    format_     = ImageFormat::RGBA;
    dataType_   = DataType::UInt8;
    extent_     = { 0, 0, 0 };
}

std::size_t Image::GetDataPtrOffset(const Offset3D& offset) const
{
    const auto bpp  = static_cast<std::size_t>(GetBytesPerPixel());
    const auto x    = static_cast<std::size_t>(offset.x);
    const auto y    = static_cast<std::size_t>(offset.y);
    const auto z    = static_cast<std::size_t>(offset.z);
    const auto w    = static_cast<std::size_t>(GetExtent().width);
    const auto h    = static_cast<std::size_t>(GetExtent().height);
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
