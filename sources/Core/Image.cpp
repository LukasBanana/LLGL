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


//TODO: use the "AllocByteArray" function from <ImageFlags.cpp>
static ByteBuffer AllocByteArray(std::size_t size)
{
    return ByteBuffer { new char[size] };
}

/* ----- Common ----- */

Image::Image(const Image& rhs) :
    format_   { rhs.format_                       },
    dataType_ { rhs.dataType_                     },
    data_     { AllocByteArray(rhs.GetDataSize()) },
    extent_   { rhs.extent_                       }
{
    std::copy(rhs.data_.get(), rhs.data_.get() + rhs.GetDataSize(), data_.get());
}

Image::Image(Image&& rhs) :
    format_   { rhs.format_          },
    dataType_ { rhs.dataType_        },
    data_     { std::move(rhs.data_) },
    extent_   { rhs.extent_          }
{
}

/* ----- Storage ----- */

void Image::Convert(const ImageFormat format, const DataType dataType, std::size_t threadCount)
{
    if (auto convertedData = ConvertImageBuffer(QuerySrcDesc(), format, dataType, threadCount))
    {
        format_     = format;
        dataType_   = dataType;
        data_       = std::move(convertedData);
    }
}

void Image::Resize(const Extent3D& extent)
{
    /* Allocate new image buffer or release it if the extent is zero */
    extent_ = extent;
    if (extent.width > 0 && extent.height > 0 && extent.depth > 0)
        data_ = AllocByteArray(GetDataSize());
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

/* ----- Pixels ----- */

void Image::Blit(const Image& srcImage, Offset3D offset, Extent3D extent)
{
    //todo
}

void Image::Fill(Offset3D offset, Extent3D extent, const ColorRGBAd& fillColor)
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

ByteBuffer Image::Release()
{
    format_         = ImageFormat::RGBA;
    dataType_       = DataType::UInt8;
    extent_.width   = 0;
    extent_.height  = 0;
    extent_.depth   = 0;
    return std::move(data_);
}


} // /namespace LLGL



// ================================================================================
