/*
 * Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Texture.h>


namespace LLGL
{


Texture::Texture(const TextureType type, long bindFlags) :
    type_      { type      },
    bindFlags_ { bindFlags }
{
}

ResourceType Texture::GetResourceType() const
{
    return ResourceType::Texture;
}

std::uint32_t Texture::GetMemoryFootprint() const
{
    const auto desc = GetDesc();
    return LLGL::GetMemoryFootprint(GetType(), desc.format, desc.extent, TextureSubresource{ 0, desc.arrayLayers, 0, desc.mipLevels });
}

std::uint32_t Texture::GetMemoryFootprint(const Extent3D& extent, const TextureSubresource& subresource) const
{
    const auto format = GetFormat();
    return LLGL::GetMemoryFootprint(GetType(), format, extent, subresource);
}


} // /namespace LLGL



// ================================================================================
