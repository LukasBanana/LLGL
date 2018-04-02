/*
 * GLTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

        Gs::Vector3ui QueryMipLevelSize(std::uint32_t mipLevel) const override;

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
