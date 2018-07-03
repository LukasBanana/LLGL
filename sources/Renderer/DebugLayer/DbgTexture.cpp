/*
 * DbgTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgTexture.h"


namespace LLGL
{


DbgTexture::DbgTexture(Texture& instance, const TextureDescriptor& desc) :
    Texture   { desc.type },
    instance  { instance  },
    desc      { desc      }
{
    /* Store number of MIP-maps (if enabled) */
    if ((desc.flags & TextureFlags::GenerateMips) != 0)
        mipLevels = NumMipLevels(desc);
    else
        mipLevels = 1;
}

Extent3D DbgTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    return instance.QueryMipLevelSize(mipLevel);
}

TextureDescriptor DbgTexture::QueryDesc() const
{
    return instance.QueryDesc();
}


} // /namespace LLGL



// ================================================================================
