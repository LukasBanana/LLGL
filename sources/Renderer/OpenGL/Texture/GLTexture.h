/*
 * GLTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEXTURE_H
#define LLGL_GL_TEXTURE_H


#include <LLGL/Texture.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLTexture : public Texture
{

    public:

        GLTexture(const TextureType type);
        ~GLTexture();

        Extent3D QueryMipLevelSize(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

        // Recreates the internal texture object. This will invalidate the previous texture ID.
        void Recreate();

        // Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        void AllocHwTexture();
        void FreeHwTexture();

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
