/*
 * NullTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullTexture.h"
#include "../../TextureUtils.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


static TextureDescriptor MakeNullTextureDesc(const TextureDescriptor& inDesc)
{
    auto outDesc = inDesc;
    outDesc.mipLevels = NumMipLevels(inDesc);
    return outDesc;
}

NullTexture::NullTexture(const TextureDescriptor& desc, const ImageView* initialImage) :
    Texture       { desc.type, desc.bindFlags },
    desc          { MakeNullTextureDesc(desc) },
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

void NullTexture::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool NullTexture::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}

TextureDescriptor NullTexture::GetDesc() const
{
    return desc;
}

Format NullTexture::GetFormat() const
{
    return desc.format;
}

Extent3D NullTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    return LLGL::GetMipExtent(GetType(), extent_, ClampMipLevel(mipLevel));
}

SubresourceFootprint NullTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    return CalcPackedSubresourceFootprint(GetType(), GetFormat(), desc.extent, mipLevel, desc.arrayLayers);
}

std::uint32_t NullTexture::ClampMipLevel(std::uint32_t mipLevel) const
{
    return std::min(mipLevel, desc.mipLevels - 1);
}

void NullTexture::Write(const TextureRegion& textureRegion, const ImageView& srcImageView)
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

void NullTexture::Read(const TextureRegion& textureRegion, const MutableImageView& dstImageView)
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

void NullTexture::GenerateMips(const TextureSubresource* subresource)
{
    //todo
}

std::uint32_t NullTexture::PackSubresourceIndex(std::uint32_t mipLevel, std::uint32_t arrayLayer) const
{
    return (mipLevel * desc.mipLevels + arrayLayer);
}

void NullTexture::UnpackSubresourceIndex(std::uint32_t subresource, std::uint32_t& outMipLevel, std::uint32_t& outArrayLayer) const
{
    outMipLevel     = subresource / desc.mipLevels;
    outArrayLayer   = subresource % desc.mipLevels;
}


/*
 * ======= Private: =======
 */

void NullTexture::AllocImages()
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
