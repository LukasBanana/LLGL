/*
 * ImageFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ImageFlags.h>
#include <LLGL/ColorRGBA.h>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <cstring>
#include "ImageUtils.h"
#include "../Core/Helper.h"
#include "../Core/Assertion.h"
#include "Float16Compressor.h"


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
    DataType                    srcDataType,
    const VariantConstBuffer&   srcBuffer,
    DataType                    dstDataType,
    VariantBuffer&              dstBuffer,
    std::size_t                 idxBegin,
    std::size_t                 idxEnd)
{
    for (auto i = idxBegin; i < idxEnd; ++i)
    {
        /* Read normalized variant from source buffer */
        double value = ReadNormalizedTypedVariant(srcDataType, srcBuffer, i);

        /* Write normalized variant to destination buffer */
        WriteNormalizedTypedVariant(dstDataType, dstBuffer, i, value);
    }
}

// Minimal number of entries each worker thread shall process
static const std::size_t g_threadMinWorkSize = 64;

static void ConvertImageBufferDataType(
    DataType    srcDataType,
    const void* srcBuffer,
    std::size_t srcBufferSize,
    DataType    dstDataType,
    void*       dstBuffer,
    std::size_t dstBufferSize,
    std::size_t threadCount)
{
    /* Validate destination buffer size */
    auto imageSize              = srcBufferSize / DataTypeSize(srcDataType);
    auto requiredDstBufferSize  = imageSize * DataTypeSize(dstDataType);

    if (dstBufferSize != requiredDstBufferSize)
        throw std::invalid_argument("cannot convert image data type with destination buffer size mismatch");

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src { srcBuffer };
    VariantBuffer dst { dstBuffer };

    threadCount = std::min(threadCount, imageSize / g_threadMinWorkSize);

    if (threadCount > 1)
    {
        /* Create worker threads */
        std::vector<std::thread> workers(threadCount);

        auto workSize       = imageSize / threadCount;
        auto workSizeRemain = imageSize % threadCount;

        std::size_t offset = 0;

        for (std::size_t i = 0; i < threadCount; ++i)
        {
            workers[i] = std::thread(
                ConvertImageBufferDataTypeWorker,
                srcDataType,
                std::ref(src),
                dstDataType,
                std::ref(dst),
                offset,
                offset + workSize
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
    ImageFormat                 srcFormat,
    DataType                    srcDataType,
    const VariantConstBuffer&   srcBuffer,
    ImageFormat                 dstFormat,
    VariantBuffer&              dstBuffer,
    std::size_t                 idxBegin,
    std::size_t                 idxEnd)
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

static void ConvertImageBufferFormat(
    const SrcImageDescriptor&   srcImageDesc,
    const DstImageDescriptor&   dstImageDesc,
    std::size_t                 threadCount)
{
    /* Get image parameters */
    auto dataTypeSize   = DataTypeSize(srcImageDesc.dataType);
    auto srcFormatSize  = ImageFormatSize(srcImageDesc.format);
    auto dstFormatSize  = ImageFormatSize(dstImageDesc.format);

    /* Validate destination buffer size */
    auto imageSize              = srcImageDesc.dataSize / srcFormatSize;
    auto requiredDstBufferSize  = imageSize * dstFormatSize;

    if (dstImageDesc.dataSize != requiredDstBufferSize)
        throw std::invalid_argument("cannot convert image format with destination buffer size mismatch");

    /* Allocate destination buffer */
    imageSize /= dataTypeSize;

    /* Get variant buffer for source and destination images */
    VariantConstBuffer src { srcImageDesc.data };
    VariantBuffer dst { dstImageDesc.data };

    threadCount = std::min(threadCount, imageSize / g_threadMinWorkSize);

    if (threadCount > 1)
    {
        /* Create worker threads */
        std::vector<std::thread> workers(threadCount);

        auto workSize       = imageSize / threadCount;
        auto workSizeRemain = imageSize % threadCount;

        std::size_t offset = 0;

        for (std::size_t i = 0; i < threadCount; ++i)
        {
            workers[i] = std::thread(
                ConvertImageBufferFormatWorker,
                srcImageDesc.format,
                srcImageDesc.dataType,
                std::ref(src),
                dstImageDesc.format,
                std::ref(dst),
                offset,
                offset + workSize
            );
            offset += workSize;
        }

        /* Execute conversion of remaining work on main thread */
        if (workSizeRemain > 0)
        {
            ConvertImageBufferFormatWorker(
                srcImageDesc.format,
                srcImageDesc.dataType,
                src,
                dstImageDesc.format,
                dst,
                offset,
                offset + workSizeRemain
            );
        }

        /* Join worker threads */
        for (auto& w : workers)
            w.join();
    }
    else
    {
        /* Execute conversion only on main thread */
        ConvertImageBufferFormatWorker(
            srcImageDesc.format,
            srcImageDesc.dataType,
            src,
            dstImageDesc.format,
            dst,
            0,
            imageSize
        );
    }
}

static void ValidateSourceImageDesc(const SrcImageDescriptor& imageDesc)
{
    LLGL_ASSERT_PTR(imageDesc.data);
    if (imageDesc.dataSize % GetMemoryFootprint(imageDesc.format, imageDesc.dataType, 1) != 0)
        throw std::invalid_argument("source image data size is not a multiple of the source data type size");
}

static void ValidateDestinationImageDesc(const DstImageDescriptor& imageDesc)
{
    LLGL_ASSERT_PTR(imageDesc.data);
    if (imageDesc.dataSize % GetMemoryFootprint(imageDesc.format, imageDesc.dataType, 1) != 0)
        throw std::invalid_argument("destination image data size is not a multiple of the destination data type size");
}

static void ValidateImageConversionParams(
    const SrcImageDescriptor&   srcImageDesc,
    ImageFormat                 dstFormat,
    DataType                    dstDataType)
{
    if (IsCompressedFormat(srcImageDesc.format) || IsCompressedFormat(dstFormat))
        throw std::invalid_argument("cannot convert compressed image formats");
    if (IsDepthStencilFormat(srcImageDesc.format) || IsDepthStencilFormat(dstFormat))
        throw std::invalid_argument("cannot convert depth-stencil image formats");
}


/* ----- Public functions ----- */

LLGL_EXPORT bool ConvertImageBuffer(
    const SrcImageDescriptor&   srcImageDesc,
    const DstImageDescriptor&   dstImageDesc,
    std::size_t                 threadCount)
{
    /* Validate input parameters */
    ValidateSourceImageDesc(srcImageDesc);
    ValidateDestinationImageDesc(dstImageDesc);
    ValidateImageConversionParams(srcImageDesc, dstImageDesc.format, dstImageDesc.dataType);

    if (threadCount >= Constants::maxThreadCount)
        threadCount = std::thread::hardware_concurrency();

    if (srcImageDesc.dataType != dstImageDesc.dataType && srcImageDesc.format != dstImageDesc.format)
    {
        /* Convert image data type with intermediate buffer */
        auto intermediateBufferSize = srcImageDesc.dataSize / DataTypeSize(srcImageDesc.dataType) * DataTypeSize(dstImageDesc.dataType);
        auto intermediateBuffer     = MakeUniqueArray<char>(intermediateBufferSize);

        ConvertImageBufferDataType(
            srcImageDesc.dataType,
            srcImageDesc.data,
            srcImageDesc.dataSize,
            dstImageDesc.dataType,
            intermediateBuffer.get(),
            intermediateBufferSize,
            threadCount
        );

        /* Set new source buffer and source data type */
        const SrcImageDescriptor intermediateImageDesc
        {
            srcImageDesc.format,
            dstImageDesc.dataType,
            intermediateBuffer.get(),
            intermediateBufferSize
        };

        /* Convert image format */
        ConvertImageBufferFormat(intermediateImageDesc, dstImageDesc, threadCount);

        return true;
    }
    else if (srcImageDesc.dataType != dstImageDesc.dataType)
    {
        /* Convert image data type */
        ConvertImageBufferDataType(
            srcImageDesc.dataType,
            srcImageDesc.data,
            srcImageDesc.dataSize,
            dstImageDesc.dataType,
            dstImageDesc.data,
            dstImageDesc.dataSize,
            threadCount
        );
        return true;
    }
    else if (srcImageDesc.format != dstImageDesc.format)
    {
        /* Convert image format */
        ConvertImageBufferFormat(srcImageDesc, dstImageDesc, threadCount);
        return true;
    }

    return false;
}

LLGL_EXPORT ByteBuffer ConvertImageBuffer(
    const SrcImageDescriptor&   srcImageDesc,
    ImageFormat                 dstFormat,
    DataType                    dstDataType,
    std::size_t                 threadCount)
{
    /* Validate input parameters */
    ValidateSourceImageDesc(srcImageDesc);
    ValidateImageConversionParams(srcImageDesc, dstFormat, dstDataType);

    if (threadCount >= Constants::maxThreadCount)
        threadCount = std::thread::hardware_concurrency();

    /* Initialize destination image descriptor */
    auto srcNumPixels = srcImageDesc.dataSize / (DataTypeSize(srcImageDesc.dataType) * ImageFormatSize(srcImageDesc.format));

    DstImageDescriptor dstImageDesc
    {
        dstFormat,
        dstDataType,
        nullptr,
        srcNumPixels * DataTypeSize(dstDataType) * ImageFormatSize(dstFormat)
    };

    if (srcImageDesc.dataType != dstDataType && srcImageDesc.format != dstFormat)
    {
        auto dstImage = MakeUniqueArray<char>(dstImageDesc.dataSize);
        {
            /* Convert image data type with intermediate buffer */
            auto intermediateBufferSize = srcImageDesc.dataSize / DataTypeSize(srcImageDesc.dataType) * DataTypeSize(dstDataType);
            auto intermediateBuffer     = MakeUniqueArray<char>(intermediateBufferSize);

            ConvertImageBufferDataType(
                srcImageDesc.dataType,
                srcImageDesc.data,
                srcImageDesc.dataSize,
                dstDataType,
                intermediateBuffer.get(),
                intermediateBufferSize,
                threadCount
            );

            /* Set new source buffer and source data type */
            const SrcImageDescriptor intermediateImageDesc
            {
                srcImageDesc.format,
                dstDataType,
                intermediateBuffer.get(),
                intermediateBufferSize
            };

            /* Convert image format */
            dstImageDesc.data = dstImage.get();
            ConvertImageBufferFormat(intermediateImageDesc, dstImageDesc, threadCount);
        }
        return dstImage;
    }
    else if (srcImageDesc.dataType != dstDataType)
    {
        /* Convert image data type */
        auto dstImage = MakeUniqueArray<char>(dstImageDesc.dataSize);
        {
            dstImageDesc.data = dstImage.get();
            ConvertImageBufferDataType(
                srcImageDesc.dataType,
                srcImageDesc.data,
                srcImageDesc.dataSize,
                dstDataType,
                dstImageDesc.data,
                dstImageDesc.dataSize,
                threadCount
            );
        }
        return dstImage;
    }
    else if (srcImageDesc.format != dstFormat)
    {
        /* Convert image format */
        auto dstImage = MakeUniqueArray<char>(dstImageDesc.dataSize);
        {
            dstImageDesc.data = dstImage.get();
            ConvertImageBufferFormat(srcImageDesc, dstImageDesc, threadCount);
        }
        return dstImage;
    }

    return nullptr;
}

// Returns the 1D flattened buffer position for a 3D image coordinate ('bpp' denotes the bytes per pixel)
static std::size_t GetFlattenedImageBufferPos(
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t z,
    std::uint32_t rows,
    std::uint32_t slices,
    std::uint32_t bpp)
{
    return static_cast<std::size_t>(z * slices + y * rows + x) * bpp;
}

static std::size_t GetFlattenedImageBufferPosEnd(
    const Offset3D& offset,
    const Extent3D& extent,
    std::uint32_t   rows,
    std::uint32_t   slices,
    std::uint32_t   bpp)
{
    /* Subtract 1 from extent dimensions and add <bpp> again to get the excluding iterator end */
    return GetFlattenedImageBufferPos(
        static_cast<std::uint32_t>(offset.x) + extent.width  - 1u,
        static_cast<std::uint32_t>(offset.y) + extent.height - 1u,
        static_cast<std::uint32_t>(offset.z) + extent.depth  - 1u,
        rows,
        slices,
        bpp
    ) + bpp;
}

LLGL_EXPORT void CopyImageBufferRegion(
    const DstImageDescriptor&   dstImageDesc,
    const Offset3D&             dstOffset,
    std::uint32_t               dstRowStride,
    std::uint32_t               dstSliceStride,
    const SrcImageDescriptor&   srcImageDesc,
    const Offset3D&             srcOffset,
    std::uint32_t               srcRowStride,
    std::uint32_t               srcSliceStride,
    const Extent3D&             extent)
{
    /* Validate input parameters */
    ValidateSourceImageDesc(srcImageDesc);
    ValidateDestinationImageDesc(dstImageDesc);

    if (srcImageDesc.format != dstImageDesc.format || srcImageDesc.dataType != dstImageDesc.dataType)
        throw std::invalid_argument("cannot copy image buffer region with source and destination images having different format or data type");

    const auto bpp = GetMemoryFootprint(dstImageDesc.format, dstImageDesc.dataType, 1);

    /* Validate destination image boundaries */
    const auto dstPos       = GetFlattenedImageBufferPos(dstOffset.x, dstOffset.y, dstOffset.z, dstRowStride, dstSliceStride, bpp);
    const auto dstPosEnd    = GetFlattenedImageBufferPosEnd(dstOffset, extent, dstRowStride, dstSliceStride, bpp);

    if (dstPosEnd > dstImageDesc.dataSize)
        throw std::out_of_range("destination image buffer region out of range");

    /* Validate source image boundaries */
    const auto srcPos       = GetFlattenedImageBufferPos(srcOffset.x, srcOffset.y, srcOffset.z, srcRowStride, srcSliceStride, bpp);
    const auto srcPosEnd    = GetFlattenedImageBufferPosEnd(srcOffset, extent, srcRowStride, srcSliceStride, bpp);

    if (srcPosEnd > srcImageDesc.dataSize)
        throw std::out_of_range("source image buffer region out of range");

    /* Copy image buffer region */
    BitBlit(
        extent,
        bpp,
        (reinterpret_cast<char*>(dstImageDesc.data) + dstPos),
        dstRowStride * bpp,
        dstSliceStride * bpp,
        (reinterpret_cast<const char*>(srcImageDesc.data) + srcPos),
        srcRowStride * bpp,
        srcSliceStride * bpp
    );
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
    auto imageBuffer = MakeUniqueArray<char>(bytesPerPixel * imageSize);

    /* Initialize image buffer with fill color */
    for (std::size_t i = 0; i < imageSize; ++i)
        ::memcpy(imageBuffer.get() + bytesPerPixel * i, fillBuffer1.raw, bytesPerPixel);

    return imageBuffer;
}

LLGL_EXPORT ByteBuffer GenerateEmptyByteBuffer(std::size_t bufferSize, bool initialize)
{
    auto buffer = MakeUniqueArray<char>(bufferSize);

    if (initialize)
        std::fill(buffer.get(), buffer.get() + bufferSize, 0);

    return buffer;
}


} // /namespace LLGL



// ================================================================================
