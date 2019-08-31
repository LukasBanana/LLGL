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
    Texture   { desc.type          },
    instance  { instance           },
    desc      { desc               },
    mipLevels { NumMipLevels(desc) }
{
}

DbgTexture::DbgTexture(Texture& instance, DbgTexture* sharedTexture, const TextureViewDescriptor& desc) :
    Texture        { desc.type                     },
    instance       { instance                      },
    viewDesc       { desc                          },
    mipLevels      { desc.subresource.numMipLevels },
    isTextureView  { true                          },
    sharedTexture_ { sharedTexture                 }
{
    sharedTexture->sharedTextureViews_.insert(this);
}

DbgTexture::~DbgTexture()
{
    /* Remove references between shared texture and texture views */
    if (sharedTexture_)
        sharedTexture_->sharedTextureViews_.erase(this);
    for (auto textureView : sharedTextureViews_)
        textureView->sharedTexture_ = nullptr;
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


} // /namespace LLGL



// ================================================================================
