/*
 * WGTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGTexture.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


/* WGTexture::WGTexture(const TextureDescriptor& desc) { ... } */

bool WGTexture::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

TextureDescriptor WGTexture::GetDesc() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Format WGTexture::GetFormat() const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

Extent3D WGTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}

SubresourceFootprint WGTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    LLGL_TRAP_NOT_IMPLEMENTED();
}


} // /namespace LLGL



// ================================================================================
