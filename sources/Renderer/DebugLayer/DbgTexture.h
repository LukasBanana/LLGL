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

        DbgTexture(Texture& instance) :
            instance( instance )
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

        Texture&        instance;
        Gs::Vector3i    size;
        int             mipLevels   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
