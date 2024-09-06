/*
 * ImageFlags.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/ImageFlags.h>
#include <LLGL/Utils/ColorRGBA.h>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <cstring>
#include "ImageUtils.h"
#include "../Core/CoreUtils.h"
#include "../Core/Assertion.h"
#include "../Core/Threading.h"
#include "Float16Compressor.h"
#include "BCDecompressor.h"
#include <LLGL/Utils/ForRange.h>


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
        // do nothing
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

using VariantColor = ColorRGBA<Variant>;

struct DepthStencilValue
{
    float           depth;
    std::uint32_t   stencil;
};


/* ----- Internal functions ----- */

// Reads the specified source variant and returns it to the normalized range [0, 1].
template <typename T>
double ReadNormalizedVariant(const T& src)
{
    constexpr double min = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double max = static_cast<double>(std::numeric_limits<T>::max());
    return (static_cast<double>(src) - min) / (max - min);
}

// Writes the specified value from the range [0, 1] to the destination variant.
template <typename T>
void WriteNormalizedVariant(T& dst, double value)
{
    constexpr double min = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double max = static_cast<double>(std::numeric_limits<T>::max());
    dst = static_cast<T>(value * (max - min) + min);
}

static double ReadNormalizedTypedVariant(DataType srcDataType, const VariantConstBuffer& srcBuffer, std::size_t idx)
{
    switch (srcDataType)
    {
        case DataType::Undefined:
            break;
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
        case DataType::Float16:
            return static_cast<double>(DecompressFloat16(srcBuffer.uint16[idx]));
        case DataType::Float32:
            return static_cast<double>(srcBuffer.real32[idx]);
        case DataType::Float64:
            return srcBuffer.real64[idx];
    }
    return 0.0;
}

static void WriteNormalizedTypedVariant(DataType dstDataType, VariantBuffer& dstBuffer, std::size_t idx, double value)
{
    switch (dstDataType)
    {
        case DataType::Undefined:
            break;
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
        case DataType::Float16:
            dstBuffer.uint16[idx] = CompressFloat16(static_cast<float>(value));
            break;
        case DataType::Float32:
            dstBuffer.real32[idx] = static_cast<float>(value);
            break;
        case DataType::Float64:
            dstBuffer.real64[idx] = value;
            break;
    }
}

// Worker thread procedure for the "ConvertImageBufferDataType" function
static void ConvertImageBufferDataTypeWorker(
    DataType            srcDataType,
    VariantConstBuffer  srcBuffer,
    DataType            dstDataType,
    VariantBuffer       dstBuffer,
    std::size_t         idxBegin,
    std::size_t         idxEnd)
{
    for_subrange(i, idxBegin, idxEnd)
    {
        /* Read normalized variant from source buffer */
        double value = ReadNormalizedTypedVariant(srcDataType, srcBuffer, i);

        /* Write normalized variant to destination buffer */
        WriteNormalizedTypedVariant(dstDataType, dstBuffer, i, value);
    }
}

static void ConvertImageBufferDataType(
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    DataType    dstDataType,
    void*       dstBuffer,
    std::size_t dstBufferSize,
    unsigned    threadCount)
{
    /* Validate destination buffer size */
    const std::size_t imageSize             = srcBufferSize / DataTypeSize(srcDataType);
    const std::size_t requiredDstBufferSize = imageSize * DataTypeSize(dstDataType);

    if (dstBufferSize != requiredDstBufferSize)
        LLGL_TRAP("cannot convert image data type with destination buffer size mismatch");

    /* Get variant buffer for source and destination images */
    DoConcurrentRange(
        std::bind(
            ConvertImageBufferDataTypeWorker,
            srcDataType,
            srcBuffer,
            dstDataType,
            dstBuffer,
            std::placeholders::_1,
            std::placeholders::_2
        ),
        imageSize,
        threadCount
    );
}

static void SetVariantMinMax(DataType dataType, Variant& var, bool setMin)
{
    switch (dataType)
    {
        case DataType::Undefined:
            break;
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
        case DataType::Float16:
            var.uint16 = CompressFloat16(setMin ? 0.0f : 1.0f);
            break;
        case DataType::Float32:
            var.real32 = (setMin ? 0.0f : 1.0f);
            break;
        case DataType::Float64:
            var.real64 = (setMin ? 0.0 : 1.0);
            break;
    }
}

static void CopyTypedVariant(DataType dataType, const VariantConstBuffer& srcBuffer, std::size_t idx, Variant& dst)
{
    switch (dataType)
    {
        case DataType::Undefined:
            break;
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
        case DataType::Float16:
            dst.uint16 = srcBuffer.uint16[idx];
            break;
        case DataType::Float32:
            dst.real32 = srcBuffer.real32[idx];
            break;
        case DataType::Float64:
            dst.real64 = srcBuffer.real64[idx];
            break;
    }
}

static void CopyTypedVariant(DataType dataType, VariantBuffer& dstBuffer, std::size_t idx, const Variant& src)
{
    switch (dataType)
    {
        case DataType::Undefined:
            break;
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
        case DataType::Float16:
            dstBuffer.uint16[idx] = src.uint16;
            break;
        case DataType::Float32:
            dstBuffer.real32[idx] = src.real32;
            break;
        case DataType::Float64:
            dstBuffer.real64[idx] = src.real64;
            break;
    }
}

template <typename TBuf, typename TVar>
void TransferRGBAFormattedVariantColor(ImageFormat format, DataType dataType, TBuf& buffer, std::size_t idx, TVar& value)
{
    switch (format)
    {
        case ImageFormat::Alpha:
            CopyTypedVariant(dataType, buffer, idx    , value.a);
            break;
        case ImageFormat::R:
            CopyTypedVariant(dataType, buffer, idx    , value.r);
            break;
        case ImageFormat::RG:
            CopyTypedVariant(dataType, buffer, idx*2    , value.r);
            CopyTypedVariant(dataType, buffer, idx*2 + 1, value.g);
            break;
        case ImageFormat::RGB:
            CopyTypedVariant(dataType, buffer, idx*3    , value.r);
            CopyTypedVariant(dataType, buffer, idx*3 + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx*3 + 2, value.b);
            break;
        case ImageFormat::BGR:
            CopyTypedVariant(dataType, buffer, idx*3    , value.b);
            CopyTypedVariant(dataType, buffer, idx*3 + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx*3 + 2, value.r);
            break;
        case ImageFormat::RGBA:
            CopyTypedVariant(dataType, buffer, idx*4    , value.r);
            CopyTypedVariant(dataType, buffer, idx*4 + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx*4 + 2, value.b);
            CopyTypedVariant(dataType, buffer, idx*4 + 3, value.a);
            break;
        case ImageFormat::BGRA:
            CopyTypedVariant(dataType, buffer, idx*4    , value.b);
            CopyTypedVariant(dataType, buffer, idx*4 + 1, value.g);
            CopyTypedVariant(dataType, buffer, idx*4 + 2, value.r);
            CopyTypedVariant(dataType, buffer, idx*4 + 3, value.a);
            break;
        case ImageFormat::ARGB:
            CopyTypedVariant(dataType, buffer, idx*4    , value.a);
            CopyTypedVariant(dataType, buffer, idx*4 + 1, value.r);
            CopyTypedVariant(dataType, buffer, idx*4 + 2, value.g);
            CopyTypedVariant(dataType, buffer, idx*4 + 3, value.b);
            break;
        case ImageFormat::ABGR:
            CopyTypedVariant(dataType, buffer, idx*4    , value.a);
            CopyTypedVariant(dataType, buffer, idx*4 + 1, value.b);
            CopyTypedVariant(dataType, buffer, idx*4 + 2, value.g);
            CopyTypedVariant(dataType, buffer, idx*4 + 3, value.r);
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

static void ReadDepthStencilValue(
    ImageFormat srcFormat, DataType dataType, const VariantConstBuffer& srcBuffer, std::size_t idx, DepthStencilValue& value)
{
    if (srcFormat == ImageFormat::Depth && dataType == DataType::UInt16)
    {
        /* Read D16UNorm format: Decompress 16-bit float */
        value.depth = DecompressFloat16(srcBuffer.uint16[idx]);
    }
    else if (srcFormat == ImageFormat::DepthStencil && dataType == DataType::UInt32)
    {
        /* Read D24UNormS8UInt format: Decompress 24-bit float and 8-bit unsigned integer */
        value.depth   = static_cast<float>(srcBuffer.uint32[idx] & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
        value.stencil = srcBuffer.uint32[idx] >> 24;
    }
    else if (srcFormat == ImageFormat::Depth && dataType == DataType::Float32)
    {
        /* Read D32Float format: Copy 32-bit float */
        value.depth = srcBuffer.real32[idx];
    }
    else if (srcFormat == ImageFormat::DepthStencil && dataType == DataType::Float32)
    {
        /* Read D32FloatS8X24UInt format: Copy 32-bit float and 8-bit unsigned integer */
        value.depth   = srcBuffer.real32[idx*2];
        value.stencil = srcBuffer.uint32[idx*2 + 1] >> 24;
    }
    else if (srcFormat == ImageFormat::Stencil && dataType == DataType::UInt8)
    {
        /* Read S8UInt format: Copy 8-bit unsigned integer */
        value.stencil = srcBuffer.uint8[idx];
    }
    else if (srcFormat == ImageFormat::Stencil && dataType == DataType::UInt32)
    {
        /* Read S8X24UInt format: Copy 8-bit unsigned integer */
        value.stencil = srcBuffer.uint32[idx] & 0xFF;
    }
}

static void WriteDepthStencilValue(
    ImageFormat dstFormat, DataType dataType, VariantBuffer& dstBuffer, std::size_t idx, const DepthStencilValue& value)
{
    if (dstFormat == ImageFormat::Depth && dataType == DataType::UInt16)
    {
        /* Write D16UNorm format: Compress 16-bit float */
        dstBuffer.uint16[idx] = CompressFloat16(value.depth);
    }
    else if (dstFormat == ImageFormat::DepthStencil && dataType == DataType::UInt32)
    {
        /* Write D24UNormS8UInt format: Decompress 24-bit float and 8-bit unsigned integer */
        const std::uint32_t depth24 = static_cast<std::uint32_t>(value.depth * static_cast<float>(0x00FFFFFFu));
        dstBuffer.uint32[idx] = ((value.stencil & 0x000000FFu)) << 24 | (depth24 & 0x00FFFFFFu);
    }
    else if (dstFormat == ImageFormat::Depth && dataType == DataType::Float32)
    {
        /* Write D32Float format: Copy 32-bit float */
        dstBuffer.real32[idx] = value.depth;
    }
    else if (dstFormat == ImageFormat::DepthStencil && dataType == DataType::Float32)
    {
        /* Read D32FloatS8X24UInt format: Copy 32-bit float and 8-bit unsigned integer */
        dstBuffer.real32[idx*2]     = value.depth;
        dstBuffer.uint32[idx*2 + 1] = (value.stencil & 0xFF) << 24;
    }
    else if (dstFormat == ImageFormat::Stencil && dataType == DataType::UInt8)
    {
        /* Write S8UInt format: Copy 8-bit unsigned integer */
        dstBuffer.uint8[idx] = value.stencil & 0xFF;
    }
    else if (dstFormat == ImageFormat::Stencil && dataType == DataType::UInt32)
    {
        /* Write S8X24UInt format: Copy 8-bit unsigned integer */
        dstBuffer.uint32[idx] = value.stencil;
    }
}

// Worker thread procedure for the "ConvertImageBufferFormat" function
static void ConvertImageBufferFormatWorker(
    ImageFormat         srcFormat,
    DataType            srcDataType,
    VariantConstBuffer  srcBuffer,
    ImageFormat         dstFormat,
    DataType            dstDataType,
    VariantBuffer       dstBuffer,
    std::size_t         begin,
    std::size_t         end)
{
    if (IsDepthOrStencilFormat(srcFormat))
    {
        /* Initialize default depth-stencil value (0, 0) */
        DepthStencilValue depthStencilValue{ 0.0f, 0u };

        for_subrange(i, begin, end)
        {
            /* Read depth-stencil value from source buffer */
            ReadDepthStencilValue(srcFormat, srcDataType, srcBuffer, i, depthStencilValue);

            /* Write depth-stencil value to destination buffer */
            WriteDepthStencilValue(dstFormat, dstDataType, dstBuffer, i, depthStencilValue);
        }
    }
    else
    {
        /* Initialize default variant color (0, 0, 0, 1) */
        VariantColor colorValue{ UninitializeTag{} };

        SetVariantMinMax(srcDataType, colorValue.r, true);
        SetVariantMinMax(srcDataType, colorValue.g, true);
        SetVariantMinMax(srcDataType, colorValue.b, true);
        SetVariantMinMax(srcDataType, colorValue.a, false);

        for_subrange(i, begin, end)
        {
            /* Read RGBA variant from source buffer */
            ReadRGBAFormattedVariant(srcFormat, srcDataType, srcBuffer, i, colorValue);

            /* Write RGBA variant to destination buffer */
            WriteRGBAFormattedVariant(dstFormat, dstDataType, dstBuffer, i, colorValue);
        }
    }
}

static void ConvertImageBufferFormat(
    const ImageView&        srcImageView,
    const MutableImageView& dstImageView,
    unsigned                threadCount)
{
    /* Validate destination buffer size */
    const std::size_t imageSize             = srcImageView.dataSize / GetMemoryFootprint(srcImageView.format, srcImageView.dataType, 1);
    const std::size_t requiredDstBufferSize = GetMemoryFootprint(dstImageView.format, dstImageView.dataType, imageSize);

    if (dstImageView.dataSize != requiredDstBufferSize)
        LLGL_TRAP("cannot convert image format with destination buffer size mismatch");

    /* Get variant buffer for source and destination images */
    DoConcurrentRange(
        std::bind(
            ConvertImageBufferFormatWorker,
            srcImageView.format,
            srcImageView.dataType,
            srcImageView.data,
            dstImageView.format,
            dstImageView.dataType,
            dstImageView.data,
            std::placeholders::_1,
            std::placeholders::_2
        ),
        imageSize,
        threadCount
    );
}

static void ValidateSourceImageView(const ImageView& imageView)
{
    LLGL_ASSERT_PTR(imageView.data);
    const std::size_t dataTypeSize = GetMemoryFootprint(imageView.format, imageView.dataType, 1);
    LLGL_ASSERT(dataTypeSize > 0, "source image data type size must be greater than zero");
    LLGL_ASSERT(imageView.dataSize % dataTypeSize == 0, "source image data size is not a multiple of the source data type size");
}

static void ValidateDestinationImageView(const MutableImageView& imageView)
{
    LLGL_ASSERT_PTR(imageView.data);
    const std::size_t dataTypeSize = GetMemoryFootprint(imageView.format, imageView.dataType, 1);
    LLGL_ASSERT(dataTypeSize > 0, "destination image data type size must be greater than zero");
    LLGL_ASSERT(imageView.dataSize % dataTypeSize == 0, "destination image data size is not a multiple of the source data type size");
}

static void ValidateImageConversionParams(
    const ImageView&    srcImageView,
    ImageFormat         dstFormat,
    DataType            dstDataType)
{
    if (IsCompressedFormat(srcImageView.format) || IsCompressedFormat(dstFormat))
        LLGL_TRAP("cannot convert compressed image formats");
    if (IsDepthOrStencilFormat(srcImageView.format) != IsDepthOrStencilFormat(dstFormat))
        LLGL_TRAP("cannot convert between depth-stencil and non-depth-stencil image formats");
    if (dstDataType < DataType::Int8 || dstDataType > DataType::Float64)
        LLGL_TRAP("invalid value for destination data type: 0x%08X", static_cast<unsigned>(dstDataType));
}


/* ----- Public functions ----- */

LLGL_EXPORT bool ConvertImageBuffer(
    const ImageView&        srcImageView,
    const MutableImageView& dstImageView,
    unsigned                threadCount)
{
    if (srcImageView.format == dstImageView.format && srcImageView.dataType == dstImageView.dataType)
        return false;

    /* Validate input parameters */
    ValidateSourceImageView(srcImageView);
    ValidateDestinationImageView(dstImageView);
    ValidateImageConversionParams(srcImageView, dstImageView.format, dstImageView.dataType);

    if (threadCount == LLGL_MAX_THREAD_COUNT)
        threadCount = std::thread::hardware_concurrency();

    if (IsDepthOrStencilFormat(srcImageView.format))
    {
        /* Convert depth-stencil image format */
        ConvertImageBufferFormat(srcImageView, dstImageView, threadCount);
    }
    else if (srcImageView.dataType != dstImageView.dataType && srcImageView.format != dstImageView.format)
    {
        /* Convert image data type with intermediate buffer */
        const std::size_t   intermediateBufferSize  = srcImageView.dataSize / DataTypeSize(srcImageView.dataType) * DataTypeSize(dstImageView.dataType);
        DynamicByteArray    intermediateBuffer      = DynamicByteArray{ intermediateBufferSize, UninitializeTag{} };

        ConvertImageBufferDataType(
            srcImageView.dataType,
            srcImageView.data,
            srcImageView.dataSize,
            dstImageView.dataType,
            intermediateBuffer.get(),
            intermediateBufferSize,
            threadCount
        );

        /* Set new source buffer and source data type */
        const ImageView intermediateImageView
        {
            srcImageView.format,
            dstImageView.dataType,
            intermediateBuffer.get(),
            intermediateBufferSize
        };

        /* Convert image format */
        ConvertImageBufferFormat(intermediateImageView, dstImageView, threadCount);

        return true;
    }
    else if (srcImageView.dataType != dstImageView.dataType)
    {
        /* Convert image data type */
        ConvertImageBufferDataType(
            srcImageView.dataType,
            srcImageView.data,
            srcImageView.dataSize,
            dstImageView.dataType,
            dstImageView.data,
            dstImageView.dataSize,
            threadCount
        );
        return true;
    }
    else if (srcImageView.format != dstImageView.format)
    {
        /* Convert image format */
        ConvertImageBufferFormat(srcImageView, dstImageView, threadCount);
        return true;
    }

    return false;
}

LLGL_EXPORT DynamicByteArray ConvertImageBuffer(
    const ImageView&    srcImageView,
    ImageFormat         dstFormat,
    DataType            dstDataType,
    unsigned            threadCount)
{
    if (srcImageView.format == dstFormat && srcImageView.dataType == dstDataType)
        return nullptr;

    /* Validate input parameters */
    ValidateSourceImageView(srcImageView);
    ValidateImageConversionParams(srcImageView, dstFormat, dstDataType);

    if (threadCount == LLGL_MAX_THREAD_COUNT)
        threadCount = std::thread::hardware_concurrency();

    /* Initialize destination image descriptor */
    const std::size_t srcNumPixels = srcImageView.dataSize / GetMemoryFootprint(srcImageView.format, srcImageView.dataType, 1);
    const std::size_t dstImageSize = GetMemoryFootprint(dstFormat, dstDataType, srcNumPixels);

    DynamicByteArray dstImage{ dstImageSize, UninitializeTag{} };

    const MutableImageView dstImageView{ dstFormat, dstDataType, dstImage.get(), dstImageSize };

    if (IsDepthOrStencilFormat(srcImageView.format))
    {
        /* Convert depth-stencil image format */
        ConvertImageBufferFormat(srcImageView, dstImageView, threadCount);
    }
    else if (srcImageView.dataType != dstDataType && srcImageView.format != dstFormat)
    {
        /* Convert image data type with intermediate buffer */
        const std::size_t   intermediateBufferSize  = srcImageView.dataSize / DataTypeSize(srcImageView.dataType) * DataTypeSize(dstDataType);
        DynamicByteArray    intermediateBuffer      = DynamicByteArray{ intermediateBufferSize, UninitializeTag{} };

        ConvertImageBufferDataType(
            srcImageView.dataType,
            srcImageView.data,
            srcImageView.dataSize,
            dstDataType,
            intermediateBuffer.get(),
            intermediateBufferSize,
            threadCount
        );

        /* Set new source buffer and source data type */
        const ImageView intermediateImageView{ srcImageView.format, dstDataType, intermediateBuffer.get(), intermediateBufferSize };

        /* Convert image format */
        ConvertImageBufferFormat(intermediateImageView, dstImageView, threadCount);
    }
    else if (srcImageView.dataType != dstDataType)
    {
        /* Convert image data type */
        ConvertImageBufferDataType(
            srcImageView.dataType,
            srcImageView.data,
            srcImageView.dataSize,
            dstDataType,
            dstImageView.data,
            dstImageView.dataSize,
            threadCount
        );
    }
    else if (srcImageView.format != dstFormat)
    {
        /* Convert image format */
        ConvertImageBufferFormat(srcImageView, dstImageView, threadCount);
    }

    return dstImage;
}

LLGL_DEPRECATED_IGNORE_PUSH()

LLGL_EXPORT DynamicByteArray DecompressImageBufferToRGBA8UNorm(
    const ImageView&    srcImageView,
    const Extent2D&     extent,
    unsigned            threadCount)
{
    switch (srcImageView.format)
    {
        case ImageFormat::BC1:
            return DecompressImageBufferToRGBA8UNorm(Format::BC1UNorm, srcImageView, extent, threadCount);
        default:
            return nullptr;
    }
}

LLGL_DEPRECATED_IGNORE_POP()

LLGL_EXPORT DynamicByteArray DecompressImageBufferToRGBA8UNorm(
    Format              compressedFormat,
    const ImageView&    srcImageView,
    const Extent2D&     extent,
    unsigned            threadCount)
{
    if (threadCount == LLGL_MAX_THREAD_COUNT)
        threadCount = std::thread::hardware_concurrency();

    /* Check for BC compression */
    switch (compressedFormat)
    {
        case Format::BC1UNorm:
        case Format::BC1UNorm_sRGB:
            return DecompressBC1ToRGBA8UNorm(extent, reinterpret_cast<const char*>(srcImageView.data), srcImageView.dataSize, threadCount);
        default:
            return nullptr;
    }
}

// Returns the 1D flattened buffer position for a 3D image coordinate ('bpp' denotes the bytes per pixel)
static std::size_t GetFlattenedImageBufferPos(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z,
    std::uint32_t rows,
    std::uint32_t layers,
    std::uint32_t bpp)
{
    return static_cast<std::size_t>(z * layers + y * rows + x) * bpp;
}

static std::size_t GetFlattenedImageBufferPosEnd(
    const Offset3D& offset,
    const Extent3D& extent,
    std::uint32_t   rows,
    std::uint32_t   layers,
    std::uint32_t   bpp)
{
    /* Subtract 1 from extent dimensions and add <bpp> again to get the excluding iterator end */
    return GetFlattenedImageBufferPos(
        static_cast<std::uint32_t>(offset.x) + extent.width  - 1u,
        static_cast<std::uint32_t>(offset.y) + extent.height - 1u,
        static_cast<std::uint32_t>(offset.z) + extent.depth  - 1u,
        rows,
        layers,
        bpp
    ) + bpp;
}

LLGL_EXPORT void CopyImageBufferRegion(
    const MutableImageView& dstImageView,
    const Offset3D&         dstOffset,
    std::uint32_t           dstRowStride,
    std::uint32_t           dstLayerStride,
    const ImageView&        srcImageView,
    const Offset3D&         srcOffset,
    std::uint32_t           srcRowStride,
    std::uint32_t           srcLayerStride,
    const Extent3D&         extent)
{
    /* Validate input parameters */
    ValidateSourceImageView(srcImageView);
    ValidateDestinationImageView(dstImageView);

    if (srcImageView.format != dstImageView.format || srcImageView.dataType != dstImageView.dataType)
        LLGL_TRAP("cannot copy image buffer region with source and destination images having different format or data type");

    const std::uint32_t bpp = static_cast<std::uint32_t>(GetMemoryFootprint(dstImageView.format, dstImageView.dataType, 1));

    /* Validate destination image boundaries */
    const std::size_t dstPos    = GetFlattenedImageBufferPos(dstOffset.x, dstOffset.y, dstOffset.z, dstRowStride, dstLayerStride, bpp);
    const std::size_t dstPosEnd = GetFlattenedImageBufferPosEnd(dstOffset, extent, dstRowStride, dstLayerStride, bpp);

    if (dstPosEnd > dstImageView.dataSize)
        LLGL_TRAP("destination image buffer region out of range");

    /* Validate source image boundaries */
    const std::size_t srcPos    = GetFlattenedImageBufferPos(srcOffset.x, srcOffset.y, srcOffset.z, srcRowStride, srcLayerStride, bpp);
    const std::size_t srcPosEnd = GetFlattenedImageBufferPosEnd(srcOffset, extent, srcRowStride, srcLayerStride, bpp);

    if (srcPosEnd > srcImageView.dataSize)
        LLGL_TRAP("source image buffer region out of range");

    /* Copy image buffer region */
    BitBlit(
        extent,
        bpp,
        (reinterpret_cast<char*>(dstImageView.data) + dstPos),
        dstRowStride * bpp,
        dstLayerStride * bpp,
        (reinterpret_cast<const char*>(srcImageView.data) + srcPos),
        srcRowStride * bpp,
        srcLayerStride * bpp
    );
}

LLGL_EXPORT DynamicByteArray GenerateImageBuffer(
    ImageFormat format,
    DataType    dataType,
    std::size_t imageSize,
    const float fillColor[4])
{
    LLGL_ASSERT(!IsCompressedFormat(format));

    /* Convert fill color data type */
    VariantColor fillColor0{ UninitializeTag{} };
    VariantBuffer fillBuffer0{ &fillColor0 };

    WriteNormalizedTypedVariant(dataType, fillBuffer0, 0, fillColor[0]);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 1, fillColor[1]);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 2, fillColor[2]);
    WriteNormalizedTypedVariant(dataType, fillBuffer0, 3, fillColor[3]);

    /* Convert fill color format */
    VariantColor fillColor1{ UninitializeTag{} };
    VariantBuffer fillBuffer1{ &fillColor1 };
    VariantConstBuffer fillBuffer2{ fillBuffer0.raw };

    ReadRGBAFormattedVariant(ImageFormat::RGBA, dataType, fillBuffer2, 0, fillColor1);
    WriteRGBAFormattedVariant(format, dataType, fillBuffer1, 0, fillColor1);

    /* Allocate image buffer */
    const std::size_t   bytesPerPixel   = GetMemoryFootprint(format, dataType, 1);
    DynamicByteArray    imageBuffer     = DynamicByteArray{ bytesPerPixel * imageSize, UninitializeTag{} };

    /* Initialize image buffer with fill color */
    DoConcurrentRange(
        [&imageBuffer, bytesPerPixel, &fillBuffer1](std::size_t begin, std::size_t end)
        {
            for_subrange(i, begin, end)
                ::memcpy(imageBuffer.get() + bytesPerPixel * i, fillBuffer1.raw, bytesPerPixel);
        },
        imageSize
    );

    return imageBuffer;
}


} // /namespace LLGL



// ================================================================================
