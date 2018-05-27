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

        // Queries the GL_TEXTURE_INTERNAL_FORMAT parameter of this texture.
        GLenum QueryGLInternalFormat() const;

        // Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        void QueryTexParams(GLint* internalFormat, GLint* extent) const;

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
