/*
 * DbgTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_TEXTURE_H__
#define __LLGL_DBG_TEXTURE_H__


#include <LLGL/Texture.h>


namespace LLGL
{


class DbgTexture : public Texture
{

    public:

        DbgTexture(Texture& instance, const TextureDescriptor& desc) :
            instance( instance ),
            desc    ( desc     )
        {
        }

        Gs::Vector3i QueryMipLevelSize(int mipLevel) const override
        {
            return instance.QueryMipLevelSize(mipLevel);
        }

        void SetType(const TextureType type)
        {
            Texture::SetType(type);
        }

        Texture&            instance;
        TextureDescriptor   desc;
        int                 mipLevels   = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
