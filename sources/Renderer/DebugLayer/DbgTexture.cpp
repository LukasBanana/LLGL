/*
 * DbgTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgTexture.h"
#include "DbgCore.h"


namespace LLGL
{


DbgTexture::DbgTexture(Texture& instance, const TextureDescriptor& desc) :
    Texture   { desc.type          },
    instance  { instance           },
    desc      { desc               },
    mipLevels { NumMipLevels(desc) }
{
}

/*DbgTexture::DbgTexture(Texture& instance, DbgTexture& sharedTexture, const TextureViewDescriptor& desc) :
    Texture       { desc.type          },
    instance      { instance           },
    viewDesc      { desc               },
    mipLevels     { NumMipLevels(desc) },
    sharedTexture { &sharedTexture     }
{
}*/

void DbgTexture::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

Extent3D DbgTexture::QueryMipExtent(std::uint32_t mipLevel) const
{
    return instance.QueryMipExtent(mipLevel);
}

TextureDescriptor DbgTexture::QueryDesc() const
{
    return instance.QueryDesc();
}


} // /namespace LLGL



// ================================================================================
