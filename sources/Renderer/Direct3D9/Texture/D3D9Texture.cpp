/*
 * D3D9Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Texture.h"
#include "../../TextureUtils.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static TextureDescriptor MakeD3D9TextureDesc(const TextureDescriptor& inDesc)
{
    auto outDesc = inDesc;
    outDesc.mipLevels = NumMipLevels(inDesc);
    return outDesc;
}

D3D9Texture::D3D9Texture(const TextureDescriptor& desc, const ImageView* initialImage) :
    Texture       { desc.type, desc.bindFlags },
    desc          { MakeD3D9TextureDesc(desc) },
    extent_       { LLGL::GetMipExtent(desc)  }
{
    AllocImages();

    if (initialImage != nullptr)
    {
        Write(TextureRegion{ Offset3D{}, extent_ }, *initialImage);
        if ((desc.miscFlags & MiscFlags::GenerateMips) != 0)
            GenerateMips();
    }

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9Texture::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool D3D9Texture::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

TextureDescriptor D3D9Texture::GetDesc() const
{
    return desc;
}

Format D3D9Texture::GetFormat() const
{
    return desc.format;
}

Extent3D D3D9Texture::GetMipExtent(std::uint32_t mipLevel) const
{
    return LLGL::GetMipExtent(GetType(), extent_, ClampMipLevel(mipLevel));
}

SubresourceFootprint D3D9Texture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    return CalcPackedSubresourceFootprint(GetType(), GetFormat(), desc.extent, mipLevel, desc.arrayLayers);
}

std::uint32_t D3D9Texture::ClampMipLevel(std::uint32_t mipLevel) const
{
    return std::min(mipLevel, desc.mipLevels - 1);
}

void D3D9Texture::Write(const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    if (textureRegion.subresource.baseMipLevel < images_.size() && textureRegion.subresource.numMipLevels == 1)
    {
        /* Write pixels to selected destination MIP-map image */
        Image& mipMap = images_[textureRegion.subresource.baseMipLevel];
        const Offset3D offset = CalcTextureOffset(GetType(), textureRegion.offset, textureRegion.subresource.baseArrayLayer);
        const Extent3D extent = CalcTextureExtent(GetType(), textureRegion.extent, textureRegion.subresource.numArrayLayers);
        mipMap.WritePixels(offset, extent, srcImageView);
    }
}

void D3D9Texture::Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    if (textureRegion.subresource.baseMipLevel < images_.size() && textureRegion.subresource.numMipLevels == 1)
    {
        /* Read pixels from selected source MIP-map image */
        Image& mipMap = images_[textureRegion.subresource.baseMipLevel];
        const Offset3D offset = CalcTextureOffset(GetType(), textureRegion.offset, textureRegion.subresource.baseArrayLayer);
        const Extent3D extent = CalcTextureExtent(GetType(), textureRegion.extent, textureRegion.subresource.numArrayLayers);
        mipMap.ReadPixels(offset, extent, dstImageView);
    }
}

void D3D9Texture::GenerateMips(const TextureSubresource* subresource)
{
    //todo
}

std::uint32_t D3D9Texture::PackSubresourceIndex(std::uint32_t mipLevel, std::uint32_t arrayLayer) const
{
    return (mipLevel * desc.mipLevels + arrayLayer);
}

void D3D9Texture::UnpackSubresourceIndex(std::uint32_t subresource, std::uint32_t& outMipLevel, std::uint32_t& outArrayLayer) const
{
    outMipLevel     = subresource / desc.mipLevels;
    outArrayLayer   = subresource % desc.mipLevels;
}


/*
 * ======= Private: =======
 */

void D3D9Texture::AllocImages()
{
    const auto& formatAttribs = GetFormatAttribs(desc.format);
    images_.reserve(desc.mipLevels);
    for_range(mipLevel, desc.mipLevels)
    {
        const Extent3D mipExtent = LLGL::GetMipExtent(GetType(), extent_, mipLevel);
        images_.emplace_back(mipExtent, formatAttribs.format, formatAttribs.dataType);
    }
}


} // /namespace LLGL



// ================================================================================
