/*
 * DbgTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgTexture.h"
#include "DbgCore.h"


namespace LLGL
{


DbgTexture::DbgTexture(Texture& instance, const TextureDescriptor& desc) :
    Texture   { desc.type, desc.bindFlags },
    instance  { instance                  },
    desc      { desc                      },
    mipLevels { NumMipLevels(desc)        }
{
}

#if 0
DbgTexture::DbgTexture(Texture& instance, DbgTexture* sharedTexture, const TextureViewDescriptor& desc) :
    Texture        { desc.type, sharedTexture->desc.bindFlags },
    instance       { instance                                 },
    viewDesc       { desc                                     },
    mipLevels      { desc.subresource.numMipLevels            },
    isTextureView  { true                                     },
    sharedTexture_ { sharedTexture                            }
{
    sharedTexture->sharedTextureViews_.insert(this);
}
#endif

DbgTexture::~DbgTexture()
{
    #if 0
    /* Remove references between shared texture and texture views */
    if (sharedTexture_)
        sharedTexture_->sharedTextureViews_.erase(this);
    for (auto textureView : sharedTextureViews_)
        textureView->sharedTexture_ = nullptr;
    #endif
}

void DbgTexture::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

Extent3D DbgTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    return instance.GetMipExtent(mipLevel);
}

TextureDescriptor DbgTexture::GetDesc() const
{
    return instance.GetDesc();
}

Format DbgTexture::GetFormat() const
{
    return instance.GetFormat();
}


} // /namespace LLGL



// ================================================================================
