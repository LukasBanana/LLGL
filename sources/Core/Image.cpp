/*
 * Image.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Image.h>
#include <algorithm>


namespace LLGL
{


static void BitBlit(
    const Extent3D& copyExtent,
    char* dst, std::uint32_t dstRowStride, std::uint32_t dstDepthStride,
    const char* src, std::uint32_t srcRowStride, std::uint32_t srcDepthStride)
{
    const auto copyRowStride    = std::min(dstRowStride, srcRowStride);
    const auto copyDepthStride  = std::min(dstDepthStride, srcDepthStride);

    srcDepthStride -= srcRowStride * copyExtent.height;

    if (srcRowStride == dstRowStride)
    {
        if (srcDepthStride == dstDepthStride)
        {
            /* Copy region directly into output data */
            ::memcpy(dst, src, copyDepthStride * copyExtent.depth);
        }
        else
        {
            /* Copy region directly into output data */
            for (std::uint32_t z = 0; z < copyExtent.depth; ++z)
            {
                /* Copy current slice */
                ::memcpy(dst, src, copyDepthStride);

                /* Move pointers to next slice */
                dst += dstDepthStride;
                src += srcDepthStride;
            }
        }
    }
    else
    {
        /* Copy region directly into output data */
        for (std::uint32_t z = 0; z < copyExtent.depth; ++z)
        {
            /* Copy current slice */
            for (std::uint32_t y = 0; y < copyExtent.height; ++y)
            {
                /* Copy current row */
                ::memcpy(dst, src, copyRowStride);

                /* Move pointers to next row */
                dst += dstRowStride;
                src += srcRowStride;
            }

            /* Move pointers to next slice */
            src += srcDepthStride;
        }
    }
}

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

/* ----- Storage ----- */

void Image::Convert(const ImageFormat format, const DataType dataType, std::size_t threadCount)
{
    /* Convert image buffer (if necessary) */
    if (data_)
    {
        if (auto convertedData = ConvertImageBuffer(QuerySrcDesc(), format, dataType, threadCount))
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
        data_   = GenerateImageBuffer(GetFormat(), GetDataType(), GetNumPixels(), fillColor);
        extent_ = extent;
    }
    else
    {
        /* Clear image by fill color */
        Fill({ 0, 0, 0 }, extent, fillColor);
    }
}

void Image::Resize(const Extent3D& extent, const ColorRGBAd& fillColor, const Offset2D& offset)
{
    //todo
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

void Image::Blit(Offset3D dstRegionOffset, const Image& srcImage, Offset3D srcRegionOffset, Extent3D srcRegionExtent)
{
    //TODO
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
                extent,
                dst, dstRowStride, dstDepthStride,
                src, srcRowStride, srcDepthStride
            );
        }
        else
        {
            /* Copy region into temporary sub-image */
            Image subImage { extent, GetFormat(), GetDataType() };

            BitBlit(
                extent,
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
                extent,
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
                extent,
                dst, dstRowStride, dstDepthStride,
                reinterpret_cast<const char*>(subImage.GetData()), subImage.GetRowStride(), subImage.GetDepthStride()
            );
        }
    }
}

/* ----- Attributes ----- */

SrcImageDescriptor Image::QuerySrcDesc() const
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

DstImageDescriptor Image::QueryDstDesc()
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
