/*
 * WGTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGTexture.h"
#include "../WGCore.h"
#include "../WGTypes.h"
#include "../../TextureUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/TextureFlags.h>


namespace LLGL
{


static void MakeDefaultTextureViewDesc(TextureViewDescriptor& outDesc, const TextureDescriptor& inDesc)
{
    outDesc.type                        = inDesc.type;
    outDesc.format                      = inDesc.format;
    outDesc.subresource.baseArrayLayer  = 0;
    outDesc.subresource.numMipLevels    = NumMipLevels(inDesc);
    outDesc.subresource.baseArrayLayer  = 0;
    outDesc.subresource.numArrayLayers  = inDesc.arrayLayers;
}

WGTexture::WGTexture(WGPUDevice device, const TextureDescriptor& desc) :
    Texture { desc.type, desc.bindFlags }
{
    CreateWebGpuTexture(device, desc);

    /* Create default texture view that covers the entire resource */
    TextureViewDescriptor fullTexViewDesc;
    MakeDefaultTextureViewDesc(fullTexViewDesc, desc);
    CreateWebGpuTextureView(fullTexViewDesc);
}

WGTexture::~WGTexture()
{
    wgpuTextureViewRelease(textureView_);
    wgpuTextureRelease(texture_);
}

void WGTexture::Write(WGPUQueue queue, const TextureRegion& textureRegion, const ImageView& imageView)
{
    const Format format = GetFormat();
    const SubresourceLayout layout = CalcSubresourceLayout(format, textureRegion.extent, textureRegion.subresource.numArrayLayers);
    const Offset3D texOffset = CalcTextureOffset(GetType(), textureRegion.offset, textureRegion.subresource.baseArrayLayer);

    LLGL_ASSERT(layout.rowStride > 0);
    LLGL_ASSERT(layout.layerStride > 0);

    WGPUTexelCopyTextureInfo dstTexInfo;
    {
        dstTexInfo.texture  = texture_;
        dstTexInfo.mipLevel = textureRegion.subresource.baseMipLevel;
        dstTexInfo.origin   = WGTypes::ToWGOrigin3D(texOffset);
        dstTexInfo.aspect   = WGPUTextureAspect_All;
    }
    WGPUTexelCopyBufferLayout bufferLayout;
    {
        bufferLayout.offset         = 0;
        bufferLayout.bytesPerRow    = layout.rowStride;
        bufferLayout.rowsPerImage   = layout.layerStride / layout.rowStride;
    }
    const WGPUExtent3D writeSize = WGTypes::ToWGExtent3D(textureRegion.extent);
    wgpuQueueWriteTexture(queue, &dstTexInfo, imageView.data, imageView.dataSize, &bufferLayout, &writeSize);
}

bool WGTexture::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

static Extent3D GetWebGpuTextureExtent2D(WGPUTexture texture)
{
    return Extent3D
    {
        wgpuTextureGetWidth(texture),
        wgpuTextureGetHeight(texture),
        1u
    };
}

static Extent3D GetWebGpuTextureExtent3D(WGPUTexture texture)
{
    return Extent3D
    {
        wgpuTextureGetWidth(texture),
        wgpuTextureGetHeight(texture),
        wgpuTextureGetDepthOrArrayLayers(texture)
    };
}

static long GetWebGpuBindFlags(WGPUTextureUsage usage)
{
    long bindFlags = 0;

    if ((usage & WGPUTextureUsage_CopySrc) != 0)
        bindFlags |= BindFlags::CopySrc;
    if ((usage & WGPUTextureUsage_CopyDst) != 0)
        bindFlags |= BindFlags::CopyDst;
    if ((usage & WGPUTextureUsage_TextureBinding) != 0)
        bindFlags |= BindFlags::Sampled;
    if ((usage & WGPUTextureUsage_StorageBinding) != 0)
        bindFlags |= BindFlags::Storage;
    if ((usage & WGPUTextureUsage_RenderAttachment) != 0)
        bindFlags |= BindFlags::ColorAttachment;

    return bindFlags;
}

TextureDescriptor WGTexture::GetDesc() const
{
    TextureDescriptor outDesc;
    {
        outDesc.type            = GetType();
        outDesc.bindFlags       = GetWebGpuBindFlags(wgpuTextureGetUsage(texture_));
        outDesc.format          = GetFormat();
        outDesc.extent          = (GetType() == TextureType::Texture3D ? GetWebGpuTextureExtent3D(texture_) : GetWebGpuTextureExtent2D(texture_));
        outDesc.arrayLayers     = (IsArrayTexture(GetType()) ? wgpuTextureGetDepthOrArrayLayers(texture_) : 1u);
        outDesc.mipLevels       = wgpuTextureGetMipLevelCount(texture_);
        outDesc.samples         = wgpuTextureGetSampleCount(texture_);
    }
    return outDesc;
}

Format WGTexture::GetFormat() const
{
    return WGTypes::FromWGTextureFormat(wgpuTextureGetFormat(texture_));
}

Extent3D WGTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    const Extent3D texExtent
    {
        wgpuTextureGetWidth(texture_),
        wgpuTextureGetHeight(texture_),
        wgpuTextureGetDepthOrArrayLayers(texture_)
    };
    return LLGL::GetMipExtent(GetType(), texExtent, mipLevel);
}

SubresourceFootprint WGTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


/*
 * ======= Private: =======
 */

static WGPUTextureUsage GetWebGpuTextureUsage(long bindFlags)
{
    WGPUTextureUsage usage = WGPUTextureUsage_CopyDst;

    if ((bindFlags & BindFlags::CopySrc) != 0)
        usage |= WGPUTextureUsage_CopySrc;
    if ((bindFlags & BindFlags::CopyDst) != 0)
        usage |= WGPUTextureUsage_CopyDst;
    if ((bindFlags & BindFlags::Sampled) != 0)
        usage |= WGPUTextureUsage_TextureBinding;
    if ((bindFlags & BindFlags::Storage) != 0)
        usage |= WGPUTextureUsage_StorageBinding;
    if ((bindFlags & BindFlags::ColorAttachment) != 0)
        usage |= WGPUTextureUsage_RenderAttachment;
    //if ((bindFlags & BindFlags) != 0)
    //    usage |= WGPUTextureUsage_TransientAttachment;
    //if ((bindFlags & BindFlags::Storage) != 0)
    //    usage |= WGPUTextureUsage_StorageAttachment;

    return usage;
}

static WGPUTextureDimension GetWebGpuTextureDimension(TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:
            return WGPUTextureDimension_1D;

        case TextureType::Texture2D:
        case TextureType::Texture2DMS:
        case TextureType::Texture1DArray:
            return WGPUTextureDimension_2D;

        case TextureType::Texture3D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMSArray:
            return WGPUTextureDimension_3D;
    }
    return WGPUTextureDimension_Undefined;
}

void WGTexture::CreateWebGpuTexture(WGPUDevice device, const TextureDescriptor& desc)
{
    WGPUTextureDescriptor wgpuTextureDesc;
    {
        wgpuTextureDesc.nextInChain     = nullptr;
        wgpuTextureDesc.label           = WGPU_STRING_VIEW_INIT;
        wgpuTextureDesc.usage           = GetWebGpuTextureUsage(desc.bindFlags);
        wgpuTextureDesc.dimension       = GetWebGpuTextureDimension(desc.type);
        wgpuTextureDesc.size            = WGTypes::ToWGExtent3D(desc.extent);
        wgpuTextureDesc.format          = WGTypes::ToWGTextureFormat(desc.format);
        wgpuTextureDesc.mipLevelCount   = desc.mipLevels;
        wgpuTextureDesc.sampleCount     = desc.samples;
        wgpuTextureDesc.viewFormatCount = 0;
        wgpuTextureDesc.viewFormats     = nullptr;
    }
    texture_ = wgpuDeviceCreateTexture(device, &wgpuTextureDesc);
    WGThrowIfCreateFailed(texture_, "WGPUTexture");
}

static WGPUTextureViewDimension GetWebGpuTextureViewDimension(TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return WGPUTextureViewDimension_1D;
        case TextureType::Texture2D:        /*pass*/
        case TextureType::Texture2DMS:      return WGPUTextureViewDimension_2D;
        case TextureType::Texture1DArray:   break; // Not supported
        case TextureType::Texture3D:        return WGPUTextureViewDimension_3D;
        case TextureType::TextureCube:      return WGPUTextureViewDimension_Cube;
        case TextureType::Texture2DArray:   return WGPUTextureViewDimension_2DArray;
        case TextureType::TextureCubeArray: /*pass*/
        case TextureType::Texture2DMSArray: return WGPUTextureViewDimension_CubeArray;
    }
    return WGPUTextureViewDimension_Undefined;
}

void WGTexture::CreateWebGpuTextureView(const TextureViewDescriptor& desc)
{
    WGPUTextureViewDescriptor wgpuTexViewDesc;
    {
        wgpuTexViewDesc.nextInChain     = nullptr;
        wgpuTexViewDesc.label           = WGPU_STRING_VIEW_INIT;
        wgpuTexViewDesc.format          = WGTypes::ToWGTextureFormat(desc.format);
        wgpuTexViewDesc.dimension       = GetWebGpuTextureViewDimension(desc.type);
        wgpuTexViewDesc.baseMipLevel    = desc.subresource.baseMipLevel;
        wgpuTexViewDesc.mipLevelCount   = desc.subresource.numMipLevels;
        wgpuTexViewDesc.baseArrayLayer  = desc.subresource.baseArrayLayer;
        wgpuTexViewDesc.arrayLayerCount = desc.subresource.numArrayLayers;
        wgpuTexViewDesc.aspect          = WGPUTextureAspect_All;
        wgpuTexViewDesc.usage           = GetWebGpuTextureUsage(GetBindFlags());
    }
    textureView_ = wgpuTextureCreateView(texture_, &wgpuTexViewDesc);
    WGThrowIfCreateFailed(textureView_, "WGPUTextureView");
}


} // /namespace LLGL



// ================================================================================
