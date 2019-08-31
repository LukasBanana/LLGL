/*
 * GLTexture.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_TEXTURE_H
#define LLGL_GL_TEXTURE_H


#include <LLGL/Texture.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLTexture final : public Texture
{

    public:

        void SetName(const char* name) override;

        Extent3D QueryMipExtent(std::uint32_t mipLevel) const override;

        TextureDescriptor QueryDesc() const override;

    public:

        GLTexture(const TextureDescriptor& desc);
        ~GLTexture();

        // Copies the specified source texture into this texture.
        void CopyImageSubData(
            GLint           dstLevel,
            const Offset3D& dstOffset,
            GLTexture&      srcTexture,
            GLint           srcLevel,
            const Offset3D& srcOffset,
            const Extent3D& extent
        );

        // Initializes this texture as a texture-view.
        void TextureView(GLTexture& sharedTexture, const TextureViewDescriptor& textureViewDesc);

        // Returns the GL_TEXTURE_INTERNAL_FORMAT parameter of this texture.
        GLenum GetInternalFormat() const;

        // Returns the hardware texture ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns the number of MIP-map levels.
        inline GLsizei GetNumMipLevels() const
        {
            return numMipLevels_;
        }

    private:

        void GetTexParams(GLint* internalFormat, GLint* extent) const;

    private:

        GLuint  id_             = 0;
        GLsizei numMipLevels_   = 1;

};


} // /namespace LLGL


#endif



// ================================================================================
