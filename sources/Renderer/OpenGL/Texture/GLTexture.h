/*
 * GLTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_TEXTURE_H__
#define __LLGL_GL_TEXTURE_H__


#include <LLGL/Texture.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLTexture : public Texture
{

    public:

        GLTexture(const GLTexture&) = delete;
        GLTexture& operator = (const GLTexture&) = delete;

        GLTexture(const TextureType type);
        ~GLTexture();

        Gs::Vector3i QueryMipLevelSize(int mipLevel) const override;

        //! Recreates the internal texture object. This will invalidate the previous texture ID.
        void Recreate();

        //! Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
