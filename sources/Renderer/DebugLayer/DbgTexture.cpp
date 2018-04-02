/*
 * DbgTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgTexture.h"


namespace LLGL
{


DbgTexture::DbgTexture(Texture& instance, const TextureDescriptor& desc) :
    Texture  { desc.type },
    instance { instance  },
    desc     { desc      }
{
}

Gs::Vector3ui DbgTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    return instance.QueryMipLevelSize(mipLevel);
}

TextureDescriptor DbgTexture::QueryDesc() const
{
    return instance.QueryDesc();
}


} // /namespace LLGL



// ================================================================================
