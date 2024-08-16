/*
 * MTTexture.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTTexture.h"
#include "../MTTypes.h"
#include "../MTDevice.h"
#include "../Buffer/MTIntermediateBuffer.h"
#include "../../TextureUtils.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static NSUInteger GetTextureLayers(const TextureDescriptor& desc)
{
    if (IsCubeTexture(desc.type))
        return desc.arrayLayers / 6;
    else
        return desc.arrayLayers;
}

static MTLTextureUsage GetTextureUsage(const TextureDescriptor& desc)
{
    MTLTextureUsage usage = 0;

    if ((desc.bindFlags & BindFlags::Sampled) != 0)
        usage |= MTLTextureUsageShaderRead | MTLTextureUsagePixelFormatView;
    if ((desc.bindFlags & BindFlags::Storage) != 0)
        usage |= MTLTextureUsageShaderWrite;
    if ((desc.bindFlags & (BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment)) != 0)
        usage |= MTLTextureUsageRenderTarget;

    return usage;
}

static MTLResourceOptions GetResourceOptions(const TextureDescriptor& desc)
{
    MTLResourceOptions opt = 0;

    if (IsDepthOrStencilFormat(desc.format))
        opt |= MTLResourceStorageModePrivate;
    #ifndef LLGL_OS_IOS
    else
        opt |= MTLResourceStorageModeManaged;
    #endif // /LLGL_OS_IOS

    return opt;
}

static MTLTextureType ToMTLTextureTypeWithMipMaps(TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return MTLTextureType2D;
        case TextureType::Texture1DArray:   return MTLTextureType2DArray;
        default:                            return MTTypes::ToMTLTextureType(type);
    }
}

static void ConvertTextureDesc(id<MTLDevice> device, MTLTextureDescriptor* dst, const TextureDescriptor& src)
{
    /*
    Convert 1D textures to 2D textures of size (Wx1) if MIP-map count is greater than 1
    since Metal does not support MIP-mapped 1D textures.
    */
    const NSUInteger mipMapCount = NumMipLevels(src);

    dst.textureType         = (mipMapCount > 1 ? ToMTLTextureTypeWithMipMaps(src.type) : MTTypes::ToMTLTextureType(src.type));
    dst.pixelFormat         = MTTypes::ToMTLPixelFormat(src.format);
    dst.width               = src.extent.width;
    dst.height              = src.extent.height;
    dst.depth               = src.extent.depth;
    dst.mipmapLevelCount    = mipMapCount;
    dst.sampleCount         = (IsMultiSampleTexture(src.type) ? MTDevice::FindSuitableSampleCount(device, static_cast<NSUInteger>(src.samples)) : 1u);
    dst.arrayLength         = GetTextureLayers(src);
    dst.usage               = GetTextureUsage(src);
    dst.resourceOptions     = GetResourceOptions(src);
    if (IsMultiSampleTexture(src.type) || IsDepthOrStencilFormat(src.format))
        dst.storageMode = MTLStorageModePrivate;
}

MTTexture::MTTexture(id<MTLDevice> device, const TextureDescriptor& desc) :
    Texture { desc.type, desc.bindFlags }
{
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    ConvertTextureDesc(device, texDesc, desc);
    native_ = [device newTextureWithDescriptor:texDesc];
    [texDesc release];
}

MTTexture::~MTTexture()
{
    [native_ release];
}

Extent3D MTTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    auto w = static_cast<std::uint32_t>([native_ width]);
    auto h = static_cast<std::uint32_t>([native_ height]);
    auto d = static_cast<std::uint32_t>([native_ depth]);
    auto a = static_cast<std::uint32_t>([native_ arrayLength]);

    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            w = std::max(1u, w >> mipLevel);
            h = a;
            d = 1u;
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a;
            break;
        case TextureType::Texture3D:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = std::max(1u, d >> mipLevel);
            break;
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a * 6;
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a;
            break;
    }

    return Extent3D{ w, h, d };
}

TextureDescriptor MTTexture::GetDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type            = GetType();
    texDesc.bindFlags       = GetBindFlags();
    texDesc.miscFlags       = 0;
    texDesc.mipLevels       = static_cast<std::uint32_t>([native_ mipmapLevelCount]);
    texDesc.format          = GetFormat();
    texDesc.extent.width    = static_cast<std::uint32_t>([native_ width]);
    texDesc.extent.height   = static_cast<std::uint32_t>([native_ height]);
    texDesc.extent.depth    = static_cast<std::uint32_t>([native_ depth]);
    texDesc.arrayLayers     = static_cast<std::uint32_t>([native_ arrayLength]);
    texDesc.samples         = static_cast<std::uint32_t>([native_ sampleCount]);

    if (IsCubeTexture(GetType()))
        texDesc.arrayLayers *= 6;

    return texDesc;
}

Format MTTexture::GetFormat() const
{
    return MTTypes::ToFormat([native_ pixelFormat]);
}

SubresourceFootprint MTTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    const auto numArrayLayers = static_cast<std::uint32_t>([native_ arrayLength]);
    return CalcPackedSubresourceFootprint(GetType(), GetFormat(), GetMipExtent(0), mipLevel, numArrayLayers);
}

void MTTexture::WriteRegion(const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    /* Convert region to MTLRegion */
    MTLRegion region;
    MTTypes::Convert(region.origin, textureRegion.offset);
    MTTypes::Convert(region.size, textureRegion.extent);

    /* Get dimensions */
    const Format                        format          = MTTypes::ToFormat([native_ pixelFormat]);
    const FormatAttributes&             formatAttribs   = GetFormatAttribs(format);
    const SubresourceCPUMappingLayout   layout          = CalcSubresourceCPUMappingLayout(format, textureRegion, srcImageView.format, srcImageView.dataType);

    if (srcImageView.dataSize < layout.imageSize)
        return /*Out of bounds*/;

    const void* imageData = srcImageView.data;

    /* Check if image data must be converted */
    DynamicByteArray intermediateData;

    if (formatAttribs.bitSize > 0 && (formatAttribs.flags & FormatFlags::IsCompressed) == 0)
    {
        /* Convert image format (will be null if no conversion is necessary) */
        intermediateData = ConvertImageBuffer(srcImageView, formatAttribs.format, formatAttribs.dataType, /*cfg.threadCount*/0);
        if (intermediateData)
        {
            /* User converted temporary buffer as image source */
            imageData = intermediateData.get();
        }
    }

    /* Replace region of native texture with source image data */
    auto byteAlignedData = reinterpret_cast<const std::int8_t*>(imageData);

    for_range(arrayLayer, textureRegion.subresource.numArrayLayers)
    {
        [native_
            replaceRegion:  region
            mipmapLevel:    static_cast<NSUInteger>(textureRegion.subresource.baseMipLevel)
            slice:          (textureRegion.subresource.baseArrayLayer + arrayLayer)
            withBytes:      byteAlignedData
            bytesPerRow:    static_cast<NSUInteger>(layout.rowStride)
            bytesPerImage:  static_cast<NSUInteger>(layout.layerStride)
        ];
        byteAlignedData += layout.layerStride;
    }
}

void MTTexture::ReadRegion(
    const TextureRegion&    textureRegion,
    const MutableImageView& dstImageView,
    id<MTLCommandQueue>     cmdQueue,
    MTIntermediateBuffer*   intermediateBuffer)
{
    /* Convert region to MTLRegion */
    MTLRegion region;
    MTTypes::Convert(region.origin, textureRegion.offset);
    MTTypes::Convert(region.size, textureRegion.extent);

    /* Get dimensions */
    const Format                        format          = MTTypes::ToFormat([native_ pixelFormat]);
    const FormatAttributes&             formatAttribs   = GetFormatAttribs(format);
    const SubresourceCPUMappingLayout   layout          = CalcSubresourceCPUMappingLayout(format, textureRegion, dstImageView.format, dstImageView.dataType);

    if (dstImageView.dataSize < layout.imageSize)
        return /*Out of bounds*/;

    if ([native_ storageMode] == MTLStorageModePrivate)
    {
        if (cmdQueue == nil || intermediateBuffer == nullptr)
            return /*Invalid arguments*/;

        ReadRegionFromPrivateMemory(
            region,
            textureRegion.subresource,
            formatAttribs,
            layout,
            dstImageView,
            cmdQueue,
            *intermediateBuffer
        );
    }
    else
    {
        ReadRegionFromSharedMemory(
            region,
            textureRegion.subresource,
            formatAttribs,
            layout,
            dstImageView
        );
    }
}

id<MTLTexture> MTTexture::CreateSubresourceView(const TextureSubresource& subresource)
{
    NSUInteger firstLevel   = static_cast<NSUInteger>(subresource.baseMipLevel);
    NSUInteger numLevels    = static_cast<NSUInteger>(subresource.numMipLevels);
    NSUInteger firstSlice   = static_cast<NSUInteger>(subresource.baseArrayLayer);
    NSUInteger numSlices    = static_cast<NSUInteger>(subresource.numArrayLayers);

    return [native_
        newTextureViewWithPixelFormat:  [native_ pixelFormat]
        textureType:                    [native_ textureType]
        levels:                         NSMakeRange(firstLevel, numLevels)
        slices:                         NSMakeRange(firstSlice, numSlices)
    ];
}

NSUInteger MTTexture::GetBytesPerRow(std::uint32_t rowExtent) const
{
    const Format format = MTTypes::ToFormat([native_ pixelFormat]);
    return LLGL::GetMemoryFootprint(format, rowExtent);
}


/*
 * ======= Private: =======
 */

void MTTexture::ReadRegionFromSharedMemory(
    const MTLRegion&                    region,
    const TextureSubresource&           subresource,
    const FormatAttributes&             formatAttribs,
    const SubresourceCPUMappingLayout&  layout,
    const MutableImageView&             dstImageView)
{
    if (formatAttribs.format != dstImageView.format || formatAttribs.dataType != dstImageView.dataType)
    {
        /* Generate intermediate buffer for conversion */
        DynamicByteArray intermediateData = DynamicByteArray{ layout.subresourceSize, UninitializeTag{} };

        for_range(arrayLayer, subresource.numArrayLayers)
        {
            /* Copy bytes into intermediate data, then convert its format */
            [native_
                getBytes:       intermediateData.get()
                bytesPerRow:    layout.rowStride
                bytesPerImage:  layout.layerStride
                fromRegion:     region
                mipmapLevel:    subresource.baseMipLevel
                slice:          subresource.baseArrayLayer + arrayLayer
            ];

            /* Convert intermediate data into requested format */
            DynamicByteArray formattedData = ConvertImageBuffer(
                ImageView{ formatAttribs.format, formatAttribs.dataType, intermediateData.get(), layout.subresourceSize },
                dstImageView.format, dstImageView.dataType, /*GetConfiguration().threadCount*/0
            );

            /* Copy temporary data into output buffer */
            ::memcpy(dstImageView.data, formattedData.get(), dstImageView.dataSize);
        }
    }
    else
    {
        char* dstImageData = reinterpret_cast<char*>(dstImageView.data);

        for_range(arrayLayer, subresource.numArrayLayers)
        {
            /* Copy bytes into intermediate data, then convert its format */
            [native_
                getBytes:       dstImageData
                bytesPerRow:    layout.rowStride
                bytesPerImage:  layout.layerStride
                fromRegion:     region
                mipmapLevel:    subresource.baseMipLevel
                slice:          subresource.baseArrayLayer + arrayLayer
            ];
            dstImageData += layout.layerStride;
        }
    }
}

static MTLBlitOption GetMTLBlitOptionForImageFormat(ImageFormat format)
{
    if (format == ImageFormat::Depth)
        return MTLBlitOptionDepthFromDepthStencil;
    else if (format == ImageFormat::Stencil)
        return MTLBlitOptionStencilFromDepthStencil;
    else
        return MTLBlitOptionNone;
}

static void MTBlitBufferFromTextureAndSync(
    id<MTLCommandQueue>         cmdQueue,
    id<MTLBuffer>               dstBuffer,
    id<MTLTexture>              srcTexture,
    const MTLRegion&            region,
    const TextureSubresource&   subresource,
    NSUInteger                  rowStride,
    MTLBlitOption               options     = MTLBlitOptionNone)
{
    id<MTLCommandBuffer> cmdBuffer = [cmdQueue commandBuffer];
    id<MTLBlitCommandEncoder> blitEncoder = [cmdBuffer blitCommandEncoder];

    NSUInteger layerStride = rowStride * region.size.height;

    for_range(arrayLayer, subresource.numArrayLayers)
    {
        /* Copy bytes into intermediate data, then convert its format */
        [blitEncoder
            copyFromTexture:            srcTexture
            sourceSlice:                subresource.baseArrayLayer + arrayLayer
            sourceLevel:                subresource.baseMipLevel
            sourceOrigin:               region.origin
            sourceSize:                 region.size
            toBuffer:                   dstBuffer
            destinationOffset:          layerStride * arrayLayer
            destinationBytesPerRow:     rowStride
            destinationBytesPerImage:   layerStride
            options:                    options
        ];
    }

    [blitEncoder endEncoding];
    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];
}

static void CopyIntermediateBufferToImageBuffer(
    MTIntermediateBuffer&   srcBuffer,
    std::size_t             srcBufferSize,
    ImageFormat             srcImageFormat,
    DataType                srcDataType,
    const MutableImageView& dstImageView)
{
    if (srcImageFormat != dstImageView.format || srcDataType != dstImageView.dataType)
    {
        ConvertImageBuffer(
            ImageView{ srcImageFormat, srcDataType, srcBuffer.GetBytes(), srcBufferSize },
            dstImageView
        );
    }
    else
    {
        /* Copy bytes from intermediate shared buffer into output CPU buffer */
        ::memcpy(dstImageView.data, srcBuffer.GetBytes(), dstImageView.dataSize);
    }
}

void MTTexture::ReadRegionFromPrivateMemory(
    const MTLRegion&            	    region,
    const TextureSubresource&           subresource,
    const FormatAttributes&             formatAttribs,
    const SubresourceCPUMappingLayout&  layout,
    const MutableImageView&             dstImageView,
    id<MTLCommandQueue>                 cmdQueue,
    MTIntermediateBuffer&               intermediateBuffer)
{
    /* Copy texture data into intermediate buffer in shared CPU/GPU memory */
    intermediateBuffer.Grow(layout.subresourceSize);

    MTLBlitOption options = GetMTLBlitOptionForImageFormat(dstImageView.format);

    if (dstImageView.format == ImageFormat::Stencil)
    {
        MTBlitBufferFromTextureAndSync(
            /*cmdQueue:*/       cmdQueue,
            /*dstBuffer:*/      intermediateBuffer.GetNative(),
            /*srcTexture:*/     native_,
            /*region:*/         region,
            /*subresource:*/    subresource,
            /*rowStride:*/      region.size.width * sizeof(std::uint8_t),
            /*options:*/        options
        );
        CopyIntermediateBufferToImageBuffer(
            /*srcBuffer:*/      intermediateBuffer,
            /*srcBufferSize:*/  layout.subresourceSize,
            /*srcImageFormat:*/ ImageFormat::Stencil,
            /*srcDataType:*/    DataType::UInt8,
            /*dstImageView:*/   dstImageView
        );
    }
    else if (dstImageView.format == ImageFormat::DepthStencil)
    {
        //TODO: Blit depth and stencil separately; MTLBlitEncoder does not allow blitting combined depth-stencil buffers at once
    }
    else
    {
        MTBlitBufferFromTextureAndSync(
            /*cmdQueue:*/       cmdQueue,
            /*dstBuffer:*/      intermediateBuffer.GetNative(),
            /*srcTexture:*/     native_,
            /*region:*/         region,
            /*subresource:*/    subresource,
            /*rowStride:*/      layout.rowStride,
            /*options:*/        options
        );
        CopyIntermediateBufferToImageBuffer(
            /*srcBuffer:*/      intermediateBuffer,
            /*srcBufferSize:*/  layout.subresourceSize,
            /*srcImageFormat:*/ formatAttribs.format,
            /*srcDataType:*/    formatAttribs.dataType,
            /*dstImageView:*/   dstImageView
        );
    }
}


} // /namespace LLGL



// ================================================================================
