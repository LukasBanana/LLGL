/*
 * DbgTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_TEXTURE_H
#define LLGL_DBG_TEXTURE_H


#include <LLGL/Texture.h>


namespace LLGL
{


class DbgTexture : public Texture
{

    public:

        DbgTexture(Texture& instance, const TextureDescriptor& desc);

        Gs::Vector3ui QueryMipLevelSize(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

        Texture&            instance;
        TextureDescriptor   desc;
        int                 mipLevels   = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
