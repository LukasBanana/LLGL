/*
 * NullTexture.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullTexture.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/Misc/ForRange.h>
#include <algorithm>


namespace LLGL
{


static TextureDescriptor MakeNullTextureDesc(const TextureDescriptor& inDesc)
{
    auto outDesc = inDesc;
    outDesc.mipLevels = NumMipLevels(inDesc);
    return outDesc;
}

NullTexture::NullTexture(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc) :
    Texture       { desc.type, desc.bindFlags },
    desc          { MakeNullTextureDesc(desc) },
    extent_       { LLGL::GetMipExtent(desc)  }
{
    AllocImages();
    if (imageDesc != nullptr)
    {
        Write(TextureRegion{ Offset3D{}, extent_ }, *imageDesc);
        if ((desc.miscFlags & MiscFlags::GenerateMips) != 0)
            GenerateMips();
    }
}

void NullTexture::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

Extent3D NullTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    return LLGL::GetMipExtent(GetType(), extent_, ClampMipLevel(mipLevel));
}

TextureDescriptor NullTexture::GetDesc() const
{
    return desc;
}

Format NullTexture::GetFormat() const
{
    return desc.format;
}

std::uint32_t NullTexture::ClampMipLevel(std::uint32_t mipLevel) const
{
    return std::min(mipLevel, desc.mipLevels - 1);
}

void NullTexture::Write(const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    //todo
}

void NullTexture::Read(const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    //todo
}

void NullTexture::GenerateMips(const TextureSubresource* subresource)
{
    //todo
}

std::uint32_t NullTexture::PackSubresourceIndex(std::uint32_t mipLevel, std::uint32_t arrayLayer) const
{
    return mipLevel * desc.mipLevels + arrayLayer;
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
        const auto mipExtent = LLGL::GetMipExtent(GetType(), extent_, mipLevel);
        images_.emplace_back(mipExtent, formatAttribs.format, formatAttribs.dataType);
    }
}


} // /namespace LLGL



// ================================================================================
