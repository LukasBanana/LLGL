/*
 * ImageConverter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ImageConverter.h>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <thread>
#include "../Renderer/Assertion.h"


namespace LLGL
{


/* ----- Internal structures ----- */

template <typename TBase, typename TMod>
struct VariantType
{
    using type = TBase;
};

template <typename TBase>
struct VariantType<TBase, int>
{
    using type = TBase;
};

template <typename TBase>
struct VariantType<TBase, int*>
{
    using type = TBase*;
};

template <typename TBase>
struct VariantType<TBase, const int*>
{
    using type = const TBase*;
};

template <typename T>
union VariantT
{
    VariantT(typename VariantType<void, T>::type raw) :
        raw( raw )
    {
    }

    typename VariantType<void, T>::type             raw;
    typename VariantType<std::int8_t, T>::type      int8;
    typename VariantType<std::uint8_t, T>::type     uint8;
    typename VariantType<std::int16_t, T>::type     int16;
    typename VariantType<std::uint16_t, T>::type    uint16;
    typename VariantType<std::int32_t, T>::type     int32;
    typename VariantType<std::uint32_t, T>::type    uint32;
    typename VariantType<float, T>::type            real32;
    typename VariantType<double, T>::type           real64;
};

using Variant = VariantT<int>;
using VariantBuffer = VariantT<int*>;
using VariantConstBuffer = VariantT<const int*>;


/* ----- Internal functions ----- */

#if 0

template <typename T>
void FillEmpty(std::vector<T>& dstImage)
{
    std::fill(dstImage.begin(), dstImage.end(), T(0));
}

template <typename T>
std::vector<T> ConvertImageRGBtoRGBA(const T* srcImage, std::size_t imageSize)
{
    imageSize /= sizeof(T);
    std::vector<T> dstImage(imageSize/3*4);

    if (srcImage)
    {
        for (std::size_t i = 0, j = 0; i < imageSize; i += 3, j += 4)
        {
            dstImage[j  ] = srcImage[i  ];
            dstImage[j+1] = srcImage[i+1];
            dstImage[j+2] = srcImage[i+2];
            dstImage[j+3] = std::numeric_limits<T>::max();
        }
    }
    else
        FillEmpty(dstImage);

    return dstImage;
}

template <typename T0, typename T1>
std::vector<T0> ConvertImageDataType(const T1* srcImage, std::size_t imageSize)
{
    imageSize /= sizeof(T1);
    std::vector<T0> dstImage(imageSize);

    if (srcImage)
    {
        for (std::size_t i = 0; i < imageSize; ++i)
            dstImage[i] = static_cast<T0>(srcImage[i]);
    }
    else
        FillEmpty(dstImage);

    return dstImage;
}

#endif

// Reads the specified source variant and returns it to the normalized range [0, 1].
template <typename T>
double ReadNormalizedVariant(const T& src)
{
    auto min = static_cast<double>(std::numeric_limits<T>::min());
    auto max = static_cast<double>(std::numeric_limits<T>::max());
    return (static_cast<double>(src) - min) / (max - min);
}

// Writes the specified value from the range [0, 1] to the destination variant.
template <typename T>
void WriteNormalizedVariant(T& dst, double value)
{
    auto min = static_cast<double>(std::numeric_limits<T>::min());
    auto max = static_cast<double>(std::numeric_limits<T>::max());
    dst = static_cast<T>(value * (max - min) + min);
}

static double ReadNormalizedTypedVariant(DataType srcDataType, const VariantConstBuffer& srcBuffer, std::size_t idx)
{
    switch (srcDataType)
    {
        case DataType::Int8:
            return ReadNormalizedVariant(srcBuffer.int8[idx]);
        case DataType::UInt8:
            return ReadNormalizedVariant(srcBuffer.uint8[idx]);
        case DataType::Int16:
            return ReadNormalizedVariant(srcBuffer.int16[idx]);
        case DataType::UInt16:
            return ReadNormalizedVariant(srcBuffer.uint16[idx]);
        case DataType::Int32:
            return ReadNormalizedVariant(srcBuffer.int32[idx]);
        case DataType::UInt32:
            return ReadNormalizedVariant(srcBuffer.uint32[idx]);
        case DataType::Float:
            return static_cast<double>(srcBuffer.real32[idx]);
        case DataType::Double:
            return srcBuffer.real64[idx];
    }
    return 0.0;
}

static void WriteNormalizedTypedVariant(DataType dstDataType, VariantBuffer& dstBuffer, std::size_t idx, double value)
{
    switch (dstDataType)
    {
        case DataType::Int8:
            WriteNormalizedVariant(dstBuffer.int8[idx], value);
            break;
        case DataType::UInt8:
            WriteNormalizedVariant(dstBuffer.uint8[idx], value);
            break;
        case DataType::Int16:
            WriteNormalizedVariant(dstBuffer.int16[idx], value);
            break;
        case DataType::UInt16:
            WriteNormalizedVariant(dstBuffer.uint16[idx], value);
            break;
        case DataType::Int32:
            WriteNormalizedVariant(dstBuffer.int32[idx], value);
            break;
        case DataType::UInt32:
            WriteNormalizedVariant(dstBuffer.uint32[idx], value);
            break;
        case DataType::Float:
            dstBuffer.real32[idx] = static_cast<float>(value);
            break;
        case DataType::Double:
            dstBuffer.real64[idx] = value;
            break;
    }
}

static ImageBuffer AllocByteArray(std::size_t size)
{
    return ImageBuffer(new char[size]);
}

static ImageBuffer ConvertImageBufferDataType(
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    DataType    dstDataType,
    std::size_t threadCount)
{
    /* Allocate destination buffer */
    auto imageSize      = srcBufferSize / DataTypeSize(srcDataType);
    auto dstBufferSize  = imageSize * DataTypeSize(dstDataType);

    auto dstImage = AllocByteArray(dstBufferSize);

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src(srcBuffer);
    VariantBuffer dst(dstImage.get());

    double value = 0.0;

    for (std::size_t i = 0; i < imageSize; ++i)
    {
        /* Read normalized variant from source buffer */
        value = ReadNormalizedTypedVariant(srcDataType, src, i);

        /* Write normalized variant to destination buffer */
        WriteNormalizedTypedVariant(dstDataType, dst, i, value);
    }

    return dstImage;
}

static ImageBuffer ConvertImageBufferFormat(
    ImageFormat srcFormat,
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    ImageFormat dstFormat,
    std::size_t threadCount)
{
    /* Allocate destination buffer */
    auto dataTypeSize   = DataTypeSize(srcDataType);
    auto srcFormatSize  = ImageFormatSize(srcFormat);
    auto dstFormatSize  = ImageFormatSize(dstFormat);

    auto imageSize      = srcBufferSize / srcFormatSize;
    auto dstBufferSize  = imageSize * dstFormatSize;
    imageSize /= dataTypeSize;

    auto dstImage = AllocByteArray(dstBufferSize);

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src(srcBuffer);
    VariantBuffer dst(dstImage.get());

    for (std::size_t i = 0; i < imageSize; ++i)
    {



    }

    return dstImage;
}


/* ----- Public functions ----- */

LLGL_EXPORT ImageBuffer ConvertImageBuffer(
    ImageFormat srcFormat,
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    ImageFormat dstFormat,
    DataType    dstDataType,
    std::size_t threadCount)
{
    /* Validate input parameters */
    LLGL_ASSERT_PTR(srcBuffer);

    if (IsCompressedFormat(srcFormat) || IsCompressedFormat(dstFormat))
        throw std::invalid_argument("can not convert compressed image formats");
    if (srcBufferSize % (DataTypeSize(srcDataType) * ImageFormatSize(srcFormat)) != 0)
        throw std::invalid_argument("source buffer size is not a multiple of the source data type size");

    ImageBuffer dstImage;
    
    if (srcDataType != dstDataType)
    {
        /* Convert image data type */
        dstImage = ConvertImageBufferDataType(srcDataType, srcBuffer, srcBufferSize, dstDataType, threadCount);

        /* Set new source buffer and source data type */
        srcBufferSize   = srcBufferSize / DataTypeSize(srcDataType) * DataTypeSize(dstDataType);
        srcDataType     = dstDataType;
        srcBuffer       = dstImage.get();
    }

    if (srcFormat != dstFormat)
    {
        /* Convert image format */
        dstImage = ConvertImageBufferFormat(srcFormat, srcDataType, srcBuffer, srcBufferSize, dstFormat, threadCount);
    }
    
    return dstImage;
}


#if 1 //TODO: remove this class

std::vector<char> ImageConverter::RGBtoRGBA_Int8(const char* srcImage, std::size_t imageSize)
{
    return {};//ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<unsigned char> ImageConverter::RGBtoRGBA_UInt8(const unsigned char* srcImage, std::size_t imageSize)
{
    return {};//ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<short> ImageConverter::RGBtoRGBA_Int16(const short* srcImage, std::size_t imageSize)
{
    return {};//ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<unsigned short> ImageConverter::RGBtoRGBA_UInt16(const unsigned short* srcImage, std::size_t imageSize)
{
    return {};//ConvertImageRGBtoRGBA(srcImage, imageSize);
}

std::vector<float> ImageConverter::Float64toFloat32(const double* srcImage, std::size_t imageSize)
{
    return {};
}

#endif


} // /namespace LLGL



// ================================================================================
