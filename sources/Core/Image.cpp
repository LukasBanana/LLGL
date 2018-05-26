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

void Image::Resize(const Extent3D& extent, const TextureFilter filter)
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
    //todo
}

void Image::Fill(Offset3D offset, Extent3D extent, const ColorRGBAd& fillColor)
{
    //todo
}

void Image::ReadPixels(Offset3D offset, Extent3D extent, const DstImageDescriptor& imageDesc) const
{
    //todo
}

void Image::WritePixels(Offset3D offset, Extent3D extent, const SrcImageDescriptor& imageDesc)
{
    //todo
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

std::uint32_t Image::GetDataSize() const
{
    return (GetNumPixels() * ImageFormatSize(format_) * DataTypeSize(dataType_));
}

std::uint32_t Image::GetNumPixels() const
{
    return (extent_.width * extent_.height * extent_.depth);
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


} // /namespace LLGL



// ================================================================================
