/*
 * ImageFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ImageFlags.h>
#include <LLGL/ColorRGBA.h>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <cstring>
#include "../Renderer/Assertion.h"


namespace LLGL
{


/* ----- Internal structures ----- */

/*
The Variant structures are used to unify the data type conversion.
*/

union Variant
{
    Variant()
    {
    }

    std::int8_t      int8;
    std::uint8_t     uint8;
    std::int16_t     int16;
    std::uint16_t    uint16;
    std::int32_t     int32;
    std::uint32_t    uint32;
    float            real32;
    double           real64;
};

union VariantBuffer
{
    VariantBuffer(void* raw) :
        raw { raw }
    {
    }

    void*           raw;
    std::int8_t*    int8;
    std::uint8_t*   uint8;
    std::int16_t*   int16;
    std::uint16_t*  uint16;
    std::int32_t*   int32;
    std::uint32_t*  uint32;
    float*          real32;
    double*         real64;
};

union VariantConstBuffer
{
    VariantConstBuffer(const void* raw) :
        raw { raw }
    {
    }

    const void*             raw;
    const std::int8_t*      int8;
    const std::uint8_t*     uint8;
    const std::int16_t*     int16;
    const std::uint16_t*    uint16;
    const std::int32_t*     int32;
    const std::uint32_t*    uint32;
    const float*            real32;
    const double*           real64;
};

using VariantColor = ColorRGBAT<Variant>;


/* ----- Internal functions ----- */

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

static ByteBuffer AllocByteArray(std::size_t size)
{
    return ByteBuffer { new char[size] };
}

// Worker thread procedure for the "ConvertImageBufferDataType" function
static void ConvertImageBufferDataTypeWorker(
    DataType srcDataType, const VariantConstBuffer& srcBuffer,
    DataType dstDataType, VariantBuffer& dstBuffer,
    std::size_t idxBegin, std::size_t idxEnd)
{
    double value = 0.0;

    for (auto i = idxBegin; i < idxEnd; ++i)
    {
        /* Read normalized variant from source buffer */
        value = ReadNormalizedTypedVariant(srcDataType, srcBuffer, i);

        /* Write normalized variant to destination buffer */
        WriteNormalizedTypedVariant(dstDataType, dstBuffer, i, value);
    }
}

// Minimal number of entries each worker thread shall process
static const std::size_t g_threadMinWorkSize = 64;

static ByteBuffer ConvertImageBufferDataType(
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    DataType    dstDataType,
    std::size_t threadCount)
{
    /* Allocate destination buffer */
    auto imageSize      = srcBufferSize / DataTypeSize(srcDataType);
    auto dstBufferSize  = imageSize * DataTypeSize(dstDataType);
    auto dstBuffer      = AllocByteArray(dstBufferSize);

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src { srcBuffer };
    VariantBuffer dst { dstBuffer.get() };

    threadCount = std::min(threadCount, imageSize / g_threadMinWorkSize);

    if (threadCount > 1)
    {
        /* Create worker threads */
        std::vector<std::thread> workers(threadCount);

        auto workSize = imageSize / threadCount;
        auto workSizeRemain = imageSize % threadCount;

        std::size_t offset = 0;

        for (std::size_t i = 0; i < threadCount; ++i)
        {
            workers[i] = std::thread(
                ConvertImageBufferDataTypeWorker,
                srcDataType, std::ref(src),
                dstDataType, std::ref(dst),
                offset, offset + workSize
            );
            offset += workSize;
        }

        /* Execute conversion of remaining work on main thread */
        if (workSizeRemain > 0)
            ConvertImageBufferDataTypeWorker(srcDataType, src, dstDataType, dst, offset, offset + workSizeRemain);

        /* Join worker threads */
        for (auto& w : workers)
            w.join();
    }
    else
    {
        /* Execute conversion only on main thread */
        ConvertImageBufferDataTypeWorker(srcDataType, src, dstDataType, dst, 0, imageSize);
    }

    return dstBuffer;
}

static void SetVariantMinMax(DataType dataType, Variant& var, bool setMin)
{
    switch (dataType)
    {
        case DataType::Int8:
            var.int8 = (setMin ? std::numeric_limits<std::int8_t>::min() : std::numeric_limits<std::int8_t>::max());
            break;
        case DataType::UInt8:
            var.uint8 = (setMin ? std::numeric_limits<std::uint8_t>::min() : std::numeric_limits<std::uint8_t>::max());
            break;
        case DataType::Int16:
            var.int16 = (setMin ? std::numeric_limits<std::int16_t>::min() : std::numeric_limits<std::int16_t>::max());
            break;
        case DataType::UInt16:
            var.uint16 = (setMin ? std::numeric_limits<std::uint16_t>::min() : std::numeric_limits<std::uint16_t>::max());
            break;
        case DataType::Int32:
            var.int32 = (setMin ? std::numeric_limits<std::int32_t>::min() : std::numeric_limits<std::int32_t>::max());
            break;
        case DataType::UInt32:
            var.uint32 = (setMin ? std::numeric_limits<std::uint32_t>::min() : std::numeric_limits<std::uint32_t>::max());
            break;
        case DataType::Float:
            var.real32 = (setMin ? 0.0f : 1.0f);
            break;
        case DataType::Double:
            var.real64 = (setMin ? 0.0 : 1.0);
            break;
    }
}

static void CopyTypedVariant(DataType dataType, const VariantConstBuffer& srcBuffer, std::size_t idx, Variant& dst)
{
    switch (dataType)
    {
        case DataType::Int8:
            dst.int8 = srcBuffer.int8[idx];
            break;
        case DataType::UInt8:
            dst.uint8 = srcBuffer.uint8[idx];
            break;
        case DataType::Int16:
            dst.int16 = srcBuffer.int16[idx];
            break;
        case DataType::UInt16:
            dst.uint16 = srcBuffer.uint16[idx];
            break;
        case DataType::Int32:
            dst.int32 = srcBuffer.int32[idx];
            break;
        case DataType::UInt32:
            dst.uint32 = srcBuffer.uint32[idx];
            break;
        case DataType::Float:
            dst.real32 = srcBuffer.real32[idx];
            break;
        case DataType::Double:
            dst.real64 = srcBuffer.real64[idx];
            break;
    }
}

static void CopyTypedVariant(DataType dataType, VariantBuffer& dstBuffer, std::size_t idx, const Variant& src)
{
    switch (dataType)
    {
        case DataType::Int8:
            dstBuffer.int8[idx] = src.int8;
            break;
        case DataType::UInt8:
            dstBuffer.uint8[idx] = src.uint8;
            break;
        case DataType::Int16:
            dstBuffer.int16[idx] = src.int16;
            break;
        case DataType::UInt16:
            dstBuffer.uint16[idx] = src.uint16;
            break;
        case DataType::Int32:
            dstBuffer.int32[idx] = src.int32;
            break;
        case DataType::UInt32:
            dstBuffer.uint32[idx] = src.uint32;
            break;
        case DataType::Float:
            dstBuffer.real32[idx] = src.real32;
            break;
        case DataType::Double:
            dstBuffer.real64[idx] = src.real64;
            break;
    }
}

template <typename TBuf, typename TVar>
void TransferRGBAFormattedVariantColor(
    ImageFormat format, DataType dataType, TBuf& buffer, std::size_t idx, TVar& value)
{
    switch (format)
    {
        case ImageFormat::R:
            CopyTypedVariant(dataType, buffer, idx    , value.r);
            break;
        case ImageFormat::RG:
            CopyTypedVariant(dataType, buffer, idx    , value.r);
            CopyTypedVariant(dataType, buffer, idx + 1, value.g);
            break;
        case ImageFormat::RGB:
            CopyTypedVariant(dataType, buffer, idx    , value.r);
            CopyTypedVariant(dataType, buffer, idx + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx + 2, value.b);
            break;
        case ImageFormat::BGR:
            CopyTypedVariant(dataType, buffer, idx    , value.b);
            CopyTypedVariant(dataType, buffer, idx + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx + 2, value.r);
            break;
        case ImageFormat::RGBA:
            CopyTypedVariant(dataType, buffer, idx    , value.r);
            CopyTypedVariant(dataType, buffer, idx + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx + 2, value.b);
            CopyTypedVariant(dataType, buffer, idx + 3, value.a);
            break;
        case ImageFormat::BGRA:
            CopyTypedVariant(dataType, buffer, idx    , value.b);
            CopyTypedVariant(dataType, buffer, idx + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx + 2, value.r);
            CopyTypedVariant(dataType, buffer, idx + 3, value.a);
            break;
        case ImageFormat::ARGB:
            CopyTypedVariant(dataType, buffer, idx    , value.a);
            CopyTypedVariant(dataType, buffer, idx + 1, value.r);
            CopyTypedVariant(dataType, buffer, idx + 2, value.g);
            CopyTypedVariant(dataType, buffer, idx + 3, value.b);
            break;
        case ImageFormat::ABGR:
            CopyTypedVariant(dataType, buffer, idx    , value.a);
            CopyTypedVariant(dataType, buffer, idx + 1, value.b);
            CopyTypedVariant(dataType, buffer, idx + 2, value.g);
            CopyTypedVariant(dataType, buffer, idx + 3, value.r);
            break;
        default:
            break;
    }
}

static void ReadRGBAFormattedVariant(
    ImageFormat srcFormat, DataType dataType, const VariantConstBuffer& srcBuffer, std::size_t idx, VariantColor& value)
{
    TransferRGBAFormattedVariantColor(srcFormat, dataType, srcBuffer, idx, value);
}

static void WriteRGBAFormattedVariant(
    ImageFormat dstFormat, DataType dataType, VariantBuffer& dstBuffer, std::size_t idx, const VariantColor& value)
{
    TransferRGBAFormattedVariantColor(dstFormat, dataType, dstBuffer, idx, value);
}

// Worker thread procedure for the "ConvertImageBufferFormat" function
static void ConvertImageBufferFormatWorker(
    ImageFormat srcFormat, DataType srcDataType, const VariantConstBuffer& srcBuffer,
    ImageFormat dstFormat, VariantBuffer& dstBuffer,
    std::size_t idxBegin, std::size_t idxEnd)
{
    /* Get size for source and destination formats */
    auto srcFormatSize  = ImageFormatSize(srcFormat);
    auto dstFormatSize  = ImageFormatSize(dstFormat);

    /* Initialize default variant color (0, 0, 0, 1) */
    VariantColor value { UninitializeTag{} };

    SetVariantMinMax(srcDataType, value.r, true);
    SetVariantMinMax(srcDataType, value.g, true);
    SetVariantMinMax(srcDataType, value.b, true);
    SetVariantMinMax(srcDataType, value.a, false);

    for (auto i = idxBegin; i < idxEnd; ++i)
    {
        /* Read RGBA variant from source buffer */
        ReadRGBAFormattedVariant(srcFormat, srcDataType, srcBuffer, i*srcFormatSize, value);

        /* Write RGBA variant to destination buffer */
        WriteRGBAFormattedVariant(dstFormat, srcDataType, dstBuffer, i*dstFormatSize, value);
    }
}

static ByteBuffer ConvertImageBufferFormat(const SrcImageDescriptor& srcImageDesc, ImageFormat dstFormat, std::size_t threadCount)
{
    /* Allocate destination buffer */
    auto dataTypeSize   = DataTypeSize(srcImageDesc.dataType);
    auto srcFormatSize  = ImageFormatSize(srcImageDesc.format);
    auto dstFormatSize  = ImageFormatSize(dstFormat);

    auto imageSize      = srcImageDesc.dataSize / srcFormatSize;
    auto dstBufferSize  = imageSize * dstFormatSize;
    imageSize /= dataTypeSize;

    auto dstBuffer = AllocByteArray(dstBufferSize);

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src { srcImageDesc.data };
    VariantBuffer dst { dstBuffer.get() };

    threadCount = std::min(threadCount, imageSize / g_threadMinWorkSize);

    if (threadCount > 1)
    {
        /* Create worker threads */
        std::vector<std::thread> workers(threadCount);

        auto workSize = imageSize / threadCount;
        auto workSizeRemain = imageSize % threadCount;

        std::size_t offset = 0;

        for (std::size_t i = 0; i < threadCount; ++i)
        {
            workers[i] = std::thread(
                ConvertImageBufferFormatWorker,
                srcImageDesc.format, srcImageDesc.dataType, std::ref(src),
                dstFormat, std::ref(dst),
                offset, offset + workSize
            );
            offset += workSize;
        }

        /* Execute conversion of remaining work on main thread */
        if (workSizeRemain > 0)
            ConvertImageBufferFormatWorker(srcImageDesc.format, srcImageDesc.dataType, src, dstFormat, dst, offset, offset + workSizeRemain);

        /* Join worker threads */
        for (auto& w : workers)
            w.join();
    }
    else
    {
        /* Execute conversion only on main thread */
        ConvertImageBufferFormatWorker(srcImageDesc.format, srcImageDesc.dataType, src, dstFormat, dst, 0, imageSize);
    }

    return dstBuffer;
}


/* ----- Public functions ----- */

LLGL_EXPORT std::uint32_t ImageFormatSize(const ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::R:                return 1;
        case ImageFormat::RG:               return 2;
        case ImageFormat::RGB:              return 3;
        case ImageFormat::BGR:              return 3;
        case ImageFormat::RGBA:             return 4;
        case ImageFormat::BGRA:             return 4;
        case ImageFormat::ARGB:             return 4;
        case ImageFormat::ABGR:             return 4;
        case ImageFormat::Depth:            return 1;
        case ImageFormat::DepthStencil:     return 2;
        case ImageFormat::CompressedRGB:    return 0;
        case ImageFormat::CompressedRGBA:   return 0;
    }
    return 0;
}

LLGL_EXPORT std::uint32_t ImageDataSize(const ImageFormat format, const DataType dataType, std::uint32_t numPixels)
{
    return (ImageFormatSize(format) * DataTypeSize(dataType) * numPixels);
}

LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format)
{
    return (format >= ImageFormat::CompressedRGB);
}

LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat format)
{
    return (format == ImageFormat::Depth || format == ImageFormat::DepthStencil);
}

static std::tuple<ImageFormat, DataType> FindSuitableImageFormatPrimary(const TextureFormat textureFormat)
{
    using T = std::tuple<ImageFormat, DataType>;
    switch (textureFormat)
    {
        case TextureFormat::Unknown:        break;

        case TextureFormat::R8:             return T{ ImageFormat::R, DataType::UInt8 };
        case TextureFormat::R8Sgn:          return T{ ImageFormat::R, DataType::Int8 };

        case TextureFormat::R16:            return T{ ImageFormat::R, DataType::UInt16 };
        case TextureFormat::R16Sgn:         return T{ ImageFormat::R, DataType::Int16 };
        case TextureFormat::R16Float:       break;

        case TextureFormat::R32UInt:        return T{ ImageFormat::R, DataType::UInt32 };
        case TextureFormat::R32SInt:        return T{ ImageFormat::R, DataType::Int32 };
        case TextureFormat::R32Float:       return T{ ImageFormat::R, DataType::Float };

        case TextureFormat::RG8:            return T{ ImageFormat::RG, DataType::UInt8 };
        case TextureFormat::RG8Sgn:         return T{ ImageFormat::RG, DataType::Int8 };

        case TextureFormat::RG16:           return T{ ImageFormat::RG, DataType::UInt16 };
        case TextureFormat::RG16Sgn:        return T{ ImageFormat::RG, DataType::Int16 };
        case TextureFormat::RG16Float:      break;

        case TextureFormat::RG32UInt:       return T{ ImageFormat::RG, DataType::UInt32 };
        case TextureFormat::RG32SInt:       return T{ ImageFormat::RG, DataType::Int32 };
        case TextureFormat::RG32Float:      return T{ ImageFormat::RG, DataType::Float };

        case TextureFormat::RGB8:           return T{ ImageFormat::RGB, DataType::UInt8 };
        case TextureFormat::RGB8Sgn:        return T{ ImageFormat::RGB, DataType::Int8 };

        case TextureFormat::RGB16:          return T{ ImageFormat::RGB, DataType::UInt16 };
        case TextureFormat::RGB16Sgn:       return T{ ImageFormat::RGB, DataType::Int16 };
        case TextureFormat::RGB16Float:     break;

        case TextureFormat::RGB32UInt:      return T{ ImageFormat::RGB, DataType::UInt32 };
        case TextureFormat::RGB32SInt:      return T{ ImageFormat::RGB, DataType::Int32 };
        case TextureFormat::RGB32Float:     return T{ ImageFormat::RGB, DataType::Float };

        case TextureFormat::RGBA8:          return T{ ImageFormat::RGBA, DataType::UInt8 };
        case TextureFormat::RGBA8Sgn:       return T{ ImageFormat::RGBA, DataType::Int8 };

        case TextureFormat::RGBA16:         return T{ ImageFormat::RGBA, DataType::UInt16 };
        case TextureFormat::RGBA16Sgn:      return T{ ImageFormat::RGBA, DataType::Int16 };
        case TextureFormat::RGBA16Float:    break;

        case TextureFormat::RGBA32UInt:     return T{ ImageFormat::RGBA, DataType::UInt32 };
        case TextureFormat::RGBA32SInt:     return T{ ImageFormat::RGBA, DataType::Int32 };
        case TextureFormat::RGBA32Float:    return T{ ImageFormat::RGBA, DataType::Float };

        case TextureFormat::D32:            return T{ ImageFormat::Depth, DataType::Float };
        case TextureFormat::D24S8:          return T{ ImageFormat::DepthStencil, DataType::Float };

        case TextureFormat::RGB_DXT1:       return T{ ImageFormat::CompressedRGB, DataType::Int8 };
        case TextureFormat::RGBA_DXT1:      return T{ ImageFormat::CompressedRGBA, DataType::Int8 };
        case TextureFormat::RGBA_DXT3:      return T{ ImageFormat::CompressedRGBA, DataType::Int16 };
        case TextureFormat::RGBA_DXT5:      return T{ ImageFormat::CompressedRGBA, DataType::Int16 };
    }

    /* Return an invalid image format */
    return T{ ImageFormat::CompressedRGBA, DataType::Double };
}

LLGL_EXPORT bool FindSuitableImageFormat(const TextureFormat textureFormat, ImageFormat& imageFormat, DataType& dataType)
{
    /* Find suitable format and check for invalid output */
    ImageFormat imageFormatTmp;
    DataType dataTypeTmp;

    std::tie(imageFormatTmp, dataTypeTmp) = FindSuitableImageFormatPrimary(textureFormat);

    if (!(imageFormatTmp == ImageFormat::CompressedRGBA && dataTypeTmp == DataType::Double))
    {
        imageFormat = imageFormatTmp;
        dataType    = dataTypeTmp;
        return true;
    }

    return false;
}

LLGL_EXPORT ByteBuffer ConvertImageBuffer(
    SrcImageDescriptor  srcImageDesc,
    ImageFormat         dstFormat,
    DataType            dstDataType,
    std::size_t         threadCount)
{
    /* Validate input parameters */
    LLGL_ASSERT_PTR(srcImageDesc.data);

    if (IsCompressedFormat(srcImageDesc.format) || IsCompressedFormat(dstFormat))
        throw std::invalid_argument("cannot convert compressed image formats");
    if (IsDepthStencilFormat(srcImageDesc.format) || IsDepthStencilFormat(dstFormat))
        throw std::invalid_argument("cannot convert depth-stencil image formats");
    if (srcImageDesc.dataSize % (DataTypeSize(srcImageDesc.dataType) * ImageFormatSize(srcImageDesc.format)) != 0)
        throw std::invalid_argument("source image data size is not a multiple of the source data type size");

    ByteBuffer dstImage;

    if (threadCount == maxThreadCount)
        threadCount = std::thread::hardware_concurrency();

    if (srcImageDesc.dataType != dstDataType)
    {
        /* Convert image data type */
        dstImage = ConvertImageBufferDataType(srcImageDesc.dataType, srcImageDesc.data, srcImageDesc.dataSize, dstDataType, threadCount);

        /* Set new source buffer and source data type */
        srcImageDesc.dataSize   = srcImageDesc.dataSize / DataTypeSize(srcImageDesc.dataType) * DataTypeSize(dstDataType);
        srcImageDesc.dataType   = dstDataType;
        srcImageDesc.data       = dstImage.get();
    }

    if (srcImageDesc.format != dstFormat)
    {
        /* Convert image format */
        dstImage = ConvertImageBufferFormat(srcImageDesc, dstFormat, threadCount);
    }

    return dstImage;
}

LLGL_EXPORT ByteBuffer GenerateImageBuffer(
    ImageFormat         format,
    DataType            dataType,
    std::size_t         imageSize,
    const ColorRGBAd&   fillColor)
{
    /* Convert fill color data type */
    VariantColor fillColor0 { UninitializeTag{} };
    VariantBuffer fillBuffer0 { &fillColor0 };

    WriteNormalizedTypedVariant(dataType, fillBuffer0, 0, fillColor.r);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 1, fillColor.g);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 2, fillColor.b);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 3, fillColor.a);

    /* Convert fill color format */
    VariantColor fillColor1 { UninitializeTag{} };
    VariantBuffer fillBuffer1 { &fillColor1 };
    VariantConstBuffer fillBuffer2 { fillBuffer0.raw };

    ReadRGBAFormattedVariant(ImageFormat::RGBA, dataType, fillBuffer2, 0, fillColor1);
    WriteRGBAFormattedVariant(format, dataType, fillBuffer1, 0, fillColor1);

    /* Allocate image buffer */
    const auto bytesPerPixel = DataTypeSize(dataType) * ImageFormatSize(format);
    auto imageBuffer = AllocByteArray(bytesPerPixel * imageSize);

    /* Initialize image buffer with fill color */
    for (std::size_t i = 0; i < imageSize; ++i)
        ::memcpy(imageBuffer.get() + bytesPerPixel * i, fillBuffer1.raw, bytesPerPixel);

    return imageBuffer;
}

LLGL_EXPORT ByteBuffer GenerateEmptyByteBuffer(std::size_t bufferSize, bool initialize)
{
    auto buffer = AllocByteArray(bufferSize);

    if (initialize)
        std::fill(buffer.get(), buffer.get() + bufferSize, 0);

    return buffer;
}


} // /namespace LLGL



// ================================================================================
